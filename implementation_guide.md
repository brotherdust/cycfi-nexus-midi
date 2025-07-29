# Nexus MIDI Controller - Implementation Guide

## Module Implementation Details

### 1. Hardware Configuration Module

#### include/config/hardware_config.hpp
```cpp
#ifndef NEXUS_HARDWARE_CONFIG_HPP
#define NEXUS_HARDWARE_CONFIG_HPP

#include <stdint.h>

namespace nexus {
namespace config {

// Pin definitions - maintain as extern to preserve original linkage
extern const int ch9;
extern const int ch10;
extern const int ch11;
extern const int ch12;
extern const int ch13;
extern const int ch14;
extern const int ch15;

extern const int aux1;
extern const int aux2;
extern const int aux3;
extern const int aux4;
extern const int aux5;
extern const int aux6;

// Configuration constants
extern const int noise_window;
extern const uint16_t min_x;
extern const uint16_t max_x;

// Hardware initialization
void initialize_pins();

// Analog read with normalization
uint16_t analog_read(uint16_t pin);

} // namespace config
} // namespace nexus

#endif // NEXUS_HARDWARE_CONFIG_HPP
```

#### src/config/hardware_config.cpp
```cpp
#include "config/hardware_config.hpp"
#include <Arduino.h>

namespace nexus {
namespace config {

// Pin definitions - maintain original values
const int ch9  = P2_0;
const int ch10 = P1_0;
const int ch11 = P1_3;
const int ch12 = P1_4;
const int ch13 = P1_5;
const int ch14 = P1_6;
const int ch15 = P1_7;

const int aux1 = P2_1;
const int aux2 = P2_2;
const int aux3 = P2_3;
const int aux4 = P2_4;
const int aux5 = P2_5;
const int aux6 = P2_6;

#ifdef NEXUS_TEST
const int noise_window = 4;
#else
const int noise_window = 2;
#endif

const uint16_t min_x = 1024 * 0.02;
const uint16_t max_x = 1024 * 0.98;

void initialize_pins() {
    pinMode(ch9 , INPUT_PULLUP);
    pinMode(ch10, INPUT);
    pinMode(ch11, INPUT);
    pinMode(ch12, INPUT);
    pinMode(ch13, INPUT);
    pinMode(ch14, INPUT);
    pinMode(ch15, INPUT);
    
    pinMode(aux1, INPUT_PULLUP);
    pinMode(aux2, INPUT_PULLUP);
    pinMode(aux3, INPUT_PULLUP);
    pinMode(aux4, INPUT_PULLUP);
    pinMode(aux5, INPUT_PULLUP);
    pinMode(aux6, INPUT_PULLUP);
}

uint16_t analog_read(uint16_t pin) {
    uint16_t x = analogRead(pin);
    if (x < min_x)
        x = min_x;
    else if (x > max_x)
        x = max_x;
    return map(x, min_x, max_x, 0, 1023);
}

} // namespace config
} // namespace nexus
```

### 2. Flash Storage Module

#### include/storage/flash_manager.hpp
```cpp
#ifndef NEXUS_FLASH_MANAGER_HPP
#define NEXUS_FLASH_MANAGER_HPP

#include <stdint.h>

// Forward declaration
class MspFlashClass;

namespace nexus {
namespace storage {

class FlashManager {
public:
    typedef unsigned char byte;
    
    FlashManager(byte* segment);
    
    void erase();
    bool empty() const;
    byte read() const;
    void write(byte val);
    
private:
    byte* find_free() const;
    byte* _segment;
};

// Global flash instances - maintain original linkage
extern FlashManager flash_b;
extern FlashManager flash_c;

} // namespace storage
} // namespace nexus

#endif // NEXUS_FLASH_MANAGER_HPP
```

#### include/storage/persistent_storage.hpp
```cpp
#ifndef NEXUS_PERSISTENT_STORAGE_HPP
#define NEXUS_PERSISTENT_STORAGE_HPP

#include <stdint.h>

namespace nexus {
namespace storage {

// Save delay configuration
extern const uint32_t save_delay;
extern int32_t save_delay_start_time;

void reset_save_delay();
bool should_save();
void mark_saved();

} // namespace storage
} // namespace nexus

#endif // NEXUS_PERSISTENT_STORAGE_HPP
```

### 3. Base Controller Architecture

#### include/controllers/base_controller.hpp
```cpp
#ifndef NEXUS_BASE_CONTROLLER_HPP
#define NEXUS_BASE_CONTROLLER_HPP

#include "midi.hpp"
#include "util.hpp"
#include "config/hardware_config.hpp"

namespace nexus {
namespace controllers {

// Forward declaration
namespace cycfi { namespace midi { struct midi_stream; } }

// Generic controller template - maintains original implementation
template <cycfi::midi::cc::controller ctrl>
class Controller {
public:
    static cycfi::midi::cc::controller const ctrl_lsb = 
        cycfi::midi::cc::controller(ctrl | 0x20);
    
    void operator()(uint32_t val_);
    
private:
    cycfi::lowpass<8, int32_t> lp1;
    cycfi::lowpass<16, int32_t> lp2;
    cycfi::gate<nexus::config::noise_window, int32_t> gt;
};

// Controller interface for non-templated controllers
class IController {
public:
    virtual ~IController() = default;
};

} // namespace controllers
} // namespace nexus

// Include template implementation
#include "controllers/base_controller.tpp"

#endif // NEXUS_BASE_CONTROLLER_HPP
```

### 4. Individual Controllers

#### include/controllers/pitch_bend_controller.hpp
```cpp
#ifndef NEXUS_PITCH_BEND_CONTROLLER_HPP
#define NEXUS_PITCH_BEND_CONTROLLER_HPP

#include "controllers/base_controller.hpp"

namespace nexus {
namespace controllers {

class PitchBendController : public IController {
public:
    void operator()(uint32_t val_);
    
private:
    cycfi::lowpass<8, int32_t> lp1;
    cycfi::lowpass<16, int32_t> lp2;
    cycfi::gate<nexus::config::noise_window, int32_t> gt;
};

} // namespace controllers
} // namespace nexus

#endif // NEXUS_PITCH_BEND_CONTROLLER_HPP
```

#### include/controllers/program_change_controller.hpp
```cpp
#ifndef NEXUS_PROGRAM_CHANGE_CONTROLLER_HPP
#define NEXUS_PROGRAM_CHANGE_CONTROLLER_HPP

#include "controllers/base_controller.hpp"

namespace nexus {
namespace controllers {

class ProgramChangeController : public IController {
public:
    ProgramChangeController();
    
    void load();
    void save();
    uint8_t get();
    void transmit();
    
    void operator()(uint32_t val_);
    void up(bool sw);
    void down(bool sw);
    void group_up(bool sw);
    void group_down(bool sw);
    
private:
    int16_t curr;
    int16_t base;
    cycfi::repeat_button<> btn_up;
    cycfi::repeat_button<> btn_down;
    cycfi::repeat_button<> grp_btn_up;
    cycfi::repeat_button<> grp_btn_down;
};

} // namespace controllers
} // namespace nexus

#endif // NEXUS_PROGRAM_CHANGE_CONTROLLER_HPP
```

### 5. Controller Factory

#### include/controllers/controller_factory.hpp
```cpp
#ifndef NEXUS_CONTROLLER_FACTORY_HPP
#define NEXUS_CONTROLLER_FACTORY_HPP

#include "midi.hpp"

namespace nexus {
namespace controllers {

// Forward declarations
template <cycfi::midi::cc::controller ctrl> class Controller;
class PitchBendController;
class ProgramChangeController;
class SustainController;
class BankSelectController;

class ControllerFactory {
public:
    // Singleton pattern to maintain global instances
    static ControllerFactory& instance();
    
    // Controller accessors
    Controller<cycfi::midi::cc::channel_volume>& volume_control();
    Controller<cycfi::midi::cc::effect_1>& fx1_control();
    Controller<cycfi::midi::cc::effect_2>& fx2_control();
    Controller<cycfi::midi::cc::modulation>& modulation_control();
    PitchBendController& pitch_bend();
    ProgramChangeController& program_change();
    SustainController& sustain_control();
    BankSelectController& bank_select_control();
    
private:
    ControllerFactory();
    ~ControllerFactory() = default;
    ControllerFactory(const ControllerFactory&) = delete;
    ControllerFactory& operator=(const ControllerFactory&) = delete;
};

} // namespace controllers
} // namespace nexus

#endif // NEXUS_CONTROLLER_FACTORY_HPP
```

### 6. Test Configuration

#### include/config/test_config.hpp
```cpp
#ifndef NEXUS_TEST_CONFIG_HPP
#define NEXUS_TEST_CONFIG_HPP

// Test mode definitions
//#define NEXUS_TEST
//#define NEXUS_TEST_NOTE
//#define NEXUS_TEST_VOLUME
//#define NEXUS_TEST_PITCH_BEND
//#define NEXUS_TEST_PROGRAM_CHANGE
//#define NEXUS_TEST_PROGRAM_CHANGE_UP_DOWN
//#define NEXUS_TEST_PROGRAM_CHANGE_GROUP_UP_DOWN
//#define NEXUS_TEST_EFFECTS_1
//#define NEXUS_TEST_EFFECTS_2
//#define NEXUS_TEST_MODULATION
//#define NEXUS_TEST_SUSTAIN
//#define NEXUS_TEST_BANK_SELECT
//#define NEXUS_DUMP_FLASH

#endif // NEXUS_TEST_CONFIG_HPP
```

### 7. Main Application Structure

#### src/nexus_midi.ino
```cpp
/**
 * @file nexus_midi.ino
 * @brief Main implementation file for the Nexus MIDI controller
 */

#include "config/test_config.hpp"
#include "config/hardware_config.hpp"
#include "storage/flash_manager.hpp"
#include "storage/persistent_storage.hpp"
#include "controllers/controller_factory.hpp"
#include "test/test_runner.hpp"
#include "midi.hpp"
#include "util.hpp"

using namespace cycfi;
using namespace nexus;

// Global MIDI output stream - maintains original linkage
midi::midi_stream midi_out;

// Previous time for loop timing
uint32_t prev_time = 0;

void setup() {
    // Initialize hardware
    config::initialize_pins();
    
    // Start MIDI output
    midi_out.start();
    
    // Get controller factory instance
    auto& factory = controllers::ControllerFactory::instance();
    
    // Load saved states
    factory.program_change().load();
    factory.bank_select_control().load();
    
    // Transmit current states
    factory.program_change().transmit();
    factory.bank_select_control().transmit();
    
#ifdef NEXUS_DUMP_FLASH
    test::dump_flash(midi_out);
#endif
}

void loop() {
#ifdef NEXUS_TEST
    test::run_tests(midi_out);
#else
    // Production loop - limit to 1kHz
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

## Key Implementation Notes

1. **Namespace Structure**: Using nested namespaces (nexus::config, nexus::storage, etc.) for organization without changing functionality

2. **Global Variable Preservation**: 
   - midi_out remains global in main file
   - Controller instances accessed through singleton factory
   - Flash instances as extern in storage namespace
   - Pin definitions as extern const in config namespace

3. **Header Guards**: All headers use traditional include guards for MSP430 compatibility

4. **Forward Declarations**: Used extensively to minimize header dependencies

5. **Template Implementation**: Template code separated into .tpp files included at end of headers

6. **Singleton Pattern**: ControllerFactory uses singleton to maintain single instances of controllers

7. **Build Order**: Each module can be compiled independently, linked together in final build