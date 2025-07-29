# Nexus MIDI Controller - Refactoring Code Examples

## Critical Implementation Examples

### 1. Maintaining Global midi_out Access

Since `midi_out` is used throughout the codebase, we need to make it accessible to all modules without creating circular dependencies.

#### Option A: Dependency Injection (Recommended)
```cpp
// In controllers/base_controller.hpp
namespace nexus {
namespace controllers {

template <cycfi::midi::cc::controller ctrl>
class Controller {
private:
    cycfi::midi::midi_stream* _midi_out;
    
public:
    explicit Controller(cycfi::midi::midi_stream* midi_out) 
        : _midi_out(midi_out) {}
    
    void operator()(uint32_t val_) {
        uint32_t val = lp2(lp1(val_));
        if (gt(val)) {
            uint8_t const msb = val >> 3;
            uint8_t const lsb = (val << 4) & 0x7F;
            *_midi_out << cycfi::midi::control_change{0, ctrl_lsb, lsb};
            *_midi_out << cycfi::midi::control_change{0, ctrl, msb};
        }
    }
    // ... rest of implementation
};

} // namespace controllers
} // namespace nexus
```

#### Option B: Global Accessor Function
```cpp
// In nexus_midi.ino
cycfi::midi::midi_stream& get_midi_out() {
    return midi_out;
}

// In any controller implementation
get_midi_out() << cycfi::midi::control_change{0, ctrl, msb};
```

### 2. Template Controller Implementation File

Since templates must be visible at compile time, use a .tpp file:

```cpp
// include/controllers/base_controller.tpp
#ifndef NEXUS_BASE_CONTROLLER_TPP
#define NEXUS_BASE_CONTROLLER_TPP

namespace nexus {
namespace controllers {

template <cycfi::midi::cc::controller ctrl>
void Controller<ctrl>::operator()(uint32_t val_) {
    uint32_t val = lp2(lp1(val_));
    if (gt(val)) {
        uint8_t const msb = val >> 3;
        uint8_t const lsb = (val << 4) & 0x7F;
        
        // Assuming midi_out is passed via constructor or global accessor
        extern cycfi::midi::midi_stream midi_out;
        midi_out << cycfi::midi::control_change{0, ctrl_lsb, lsb};
        midi_out << cycfi::midi::control_change{0, ctrl, msb};
    }
}

} // namespace controllers
} // namespace nexus

#endif // NEXUS_BASE_CONTROLLER_TPP
```

### 3. Controller Factory Implementation

```cpp
// src/controllers/controller_factory.cpp
#include "controllers/controller_factory.hpp"
#include "controllers/pitch_bend_controller.hpp"
#include "controllers/program_change_controller.hpp"
#include "controllers/sustain_controller.hpp"
#include "controllers/bank_select_controller.hpp"

namespace nexus {
namespace controllers {

// Static instances
static Controller<cycfi::midi::cc::channel_volume>* s_volume_control = nullptr;
static Controller<cycfi::midi::cc::effect_1>* s_fx1_control = nullptr;
static Controller<cycfi::midi::cc::effect_2>* s_fx2_control = nullptr;
static Controller<cycfi::midi::cc::modulation>* s_modulation_control = nullptr;
static PitchBendController* s_pitch_bend = nullptr;
static ProgramChangeController* s_program_change = nullptr;
static SustainController* s_sustain_control = nullptr;
static BankSelectController* s_bank_select_control = nullptr;

ControllerFactory& ControllerFactory::instance() {
    static ControllerFactory instance;
    return instance;
}

ControllerFactory::ControllerFactory() {
    // Initialize all controllers
    s_volume_control = new Controller<cycfi::midi::cc::channel_volume>();
    s_fx1_control = new Controller<cycfi::midi::cc::effect_1>();
    s_fx2_control = new Controller<cycfi::midi::cc::effect_2>();
    s_modulation_control = new Controller<cycfi::midi::cc::modulation>();
    s_pitch_bend = new PitchBendController();
    s_program_change = new ProgramChangeController();
    s_sustain_control = new SustainController();
    s_bank_select_control = new BankSelectController();
}

Controller<cycfi::midi::cc::channel_volume>& ControllerFactory::volume_control() {
    return *s_volume_control;
}

// ... implement other accessors similarly

} // namespace controllers
} // namespace nexus
```

### 4. Handling Test Modes

```cpp
// src/test/test_runner.cpp
#include "test/test_runner.hpp"
#include "config/test_config.hpp"
#include "config/hardware_config.hpp"
#include "controllers/controller_factory.hpp"

namespace nexus {
namespace test {

#ifdef NEXUS_TEST

#ifdef NEXUS_TEST_NOTE
struct Note {
    void operator()(bool sw) {
        int state = edge(sw);
        if (state == 1) {
            extern cycfi::midi::midi_stream midi_out;
            midi_out << cycfi::midi::note_on{0, 80, 127};
        } else if (state == -1) {
            extern cycfi::midi::midi_stream midi_out;
            midi_out << cycfi::midi::note_off{0, 80, 127};
        }
    }
    cycfi::edge_detector<> edge;
};
static Note s_note;
#endif

void run_tests(cycfi::midi::midi_stream& midi_out) {
    auto& factory = controllers::ControllerFactory::instance();
    
#ifdef NEXUS_TEST_NOTE
    s_note(digitalRead(config::aux1));
#endif

#ifdef NEXUS_TEST_VOLUME
    factory.volume_control()(config::analog_read(config::ch10));
#endif

    // ... other test cases
}

#endif // NEXUS_TEST

} // namespace test
} // namespace nexus
```

### 5. Flash Storage with Namespace

```cpp
// src/storage/flash_manager.cpp
#include "storage/flash_manager.hpp"
#include "MspFlash.h"

namespace nexus {
namespace storage {

// Define the global instances
FlashManager flash_b(SEGMENT_B);
FlashManager flash_c(SEGMENT_C);

FlashManager::FlashManager(byte* segment)
    : _segment(segment) {
}

void FlashManager::erase() {
    Flash.erase(_segment);
}

bool FlashManager::empty() const {
    return (*_segment == 0xff);
}

// ... rest of implementation

} // namespace storage
} // namespace nexus
```

### 6. Program Change Controller with Flash Access

```cpp
// src/controllers/program_change_controller.cpp
#include "controllers/program_change_controller.hpp"
#include "storage/flash_manager.hpp"
#include "storage/persistent_storage.hpp"

namespace nexus {
namespace controllers {

void ProgramChangeController::load() {
    if (!storage::flash_b.empty()) {
        base = storage::flash_b.read();
    }
}

void ProgramChangeController::save() {
    uint8_t base_ = max(min(base, 127), 0);
    if (base_ != storage::flash_b.read()) {
        storage::flash_b.write(base_);
    }
}

// ... rest of implementation

} // namespace controllers
} // namespace nexus
```

### 7. Updated Main Loop

```cpp
// In nexus_midi.ino
void loop() {
#ifdef NEXUS_TEST
    nexus::test::run_tests(midi_out);
#else
    using namespace nexus;
    
    if (prev_time != millis()) {
        auto& factory = controllers::ControllerFactory::instance();
        
        // Process all inputs
        factory.sustain_control()(digitalRead(config::ch9));
        factory.volume_control()(config::analog_read(config::ch10));
        factory.fx1_control()(config::analog_read(config::ch11));
        factory.fx2_control()(config::analog_read(config::ch12));
        factory.pitch_bend()(config::analog_read(config::ch13));
        factory.program_change()(config::analog_read(config::ch14));
        factory.modulation_control()(config::analog_read(config::ch15));
        
        // Process buttons
        factory.program_change().up(!digitalRead(config::aux1));
        factory.program_change().down(!digitalRead(config::aux2));
        factory.program_change().group_up(!digitalRead(config::aux3));
        factory.program_change().group_down(!digitalRead(config::aux4));
        factory.bank_select_control().up(!digitalRead(config::aux5));
        factory.bank_select_control().down(!digitalRead(config::aux6));
        
        prev_time = millis();
    }
    
    // Save states if needed
    if (storage::should_save()) {
        factory.program_change().save();
        factory.bank_select_control().save();
        storage::mark_saved();
    }
#endif
}
```

## Important Considerations

1. **Include Order**: Always include Arduino.h first in .cpp files that use Arduino functions
2. **Namespace Usage**: Use `using namespace` sparingly, prefer explicit namespace qualification
3. **Memory Management**: MSP430 has limited memory, avoid dynamic allocation where possible
4. **Template Instantiation**: Ensure all template instantiations are visible to the linker
5. **Global Access**: Use extern declarations carefully to maintain single definition rule

## Compiler Flags

Add to platformio.ini if needed:
```ini
build_flags = 
    -std=gnu++0x
    -D__SINGLE_PRECISION__=1
    -Wno-overflow
    -I include