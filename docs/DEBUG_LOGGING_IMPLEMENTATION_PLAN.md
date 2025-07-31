# Debug Logging Implementation Plan

## File Structure

```
include/
├── debug/
│   ├── debug_logger.hpp      # Main debug logger class
│   ├── debug_categories.hpp  # Log categories and levels
│   └── debug_macros.hpp      # Convenience macros
└── config/
    └── feature_config.hpp    # Add debug logging flags

src/
├── debug/
│   └── debug_logger.cpp      # Logger implementation (if needed)
└── nexus_midi.ino           # Integration points
```

## Implementation Details

### 1. Debug Categories Header (`debug_categories.hpp`)

```cpp
namespace nexus {
namespace debug {

// Message types for SysEx
enum MessageType {
    LOG_SYSTEM    = 0x01,
    LOG_CONTROL   = 0x02,
    LOG_MEMORY    = 0x03,
    LOG_ERROR     = 0x04,
    LOG_PERF      = 0x05,
    LOG_DEBUG     = 0x06,
    LOG_CONFIG    = 0x07,
    LOG_STORAGE   = 0x08
};

// Log levels
enum LogLevel {
    TRACE   = 0x00,
    DEBUG   = 0x01,
    INFO    = 0x02,
    WARNING = 0x03,
    ERROR   = 0x04,
    FATAL   = 0x05
};

// Error codes
enum ErrorCode {
    ERR_NONE           = 0x00,
    ERR_MEMORY_LOW     = 0x01,
    ERR_FLASH_WRITE    = 0x02,
    ERR_FLASH_READ     = 0x03,
    ERR_INVALID_CONFIG = 0x04,
    ERR_MIDI_OVERFLOW  = 0x05
};

} // namespace debug
} // namespace nexus
```

### 2. Debug Logger Class (`debug_logger.hpp`)

```cpp
namespace nexus {
namespace debug {

class DebugLogger {
private:
    static uint16_t timestamp_counter;
    
    // Get 14-bit timestamp
    static uint16_t get_timestamp() {
        return (millis() >> 2) & 0x3FFF;  // Divide by 4, mask to 14 bits
    }
    
    // Send raw SysEx data
    static void send_sysex_start() {
        midi_out << 0xF0 << 0x7D << 0x4E << 0x45 << 0x58;
    }
    
    static void send_sysex_end() {
        midi_out << 0xF7;
    }

public:
    // Log system event
    static void log_system(uint8_t event, uint8_t param1 = 0, uint8_t param2 = 0);
    
    // Log controller change
    static void log_control(uint8_t controller_id, uint8_t value);
    
    // Log memory usage
    static void log_memory(uint8_t heap_percent, uint16_t stack_free);
    
    // Log error
    static void log_error(ErrorCode code, uint8_t context = 0);
    
    // Log generic debug message
    static void log_debug(const char* msg, uint8_t value = 0);
};

} // namespace debug
} // namespace nexus
```

### 3. Debug Macros (`debug_macros.hpp`)

```cpp
#ifdef NEXUS_ENABLE_DEBUG_LOGGING

#define NEXUS_LOG_SYSTEM(event, p1, p2) \
    nexus::debug::DebugLogger::log_system(event, p1, p2)

#define NEXUS_LOG_CONTROL(ctrl, val) \
    nexus::debug::DebugLogger::log_control(ctrl, val)

#define NEXUS_LOG_MEMORY() \
    nexus::debug::DebugLogger::log_memory_usage()

#define NEXUS_LOG_ERROR(code) \
    nexus::debug::DebugLogger::log_error(code)

#define NEXUS_LOG_DEBUG(msg, val) \
    nexus::debug::DebugLogger::log_debug(msg, val)

#else

// No-op macros when debugging is disabled
#define NEXUS_LOG_SYSTEM(event, p1, p2)
#define NEXUS_LOG_CONTROL(ctrl, val)
#define NEXUS_LOG_MEMORY()
#define NEXUS_LOG_ERROR(code)
#define NEXUS_LOG_DEBUG(msg, val)

#endif
```

### 4. Integration Points

#### In `nexus_midi.ino` setup():
```cpp
void setup() {
    // Initialize hardware pins
    nexus::config::initialize_pins();
    
    midi_out.start();
    
    // Log system startup
    NEXUS_LOG_SYSTEM(0x01, 0x02, 0x00);  // startup, version 2.0
    
    // Initialize all controllers
    nexus::controllers::controllers.initialize();
    
    // Log memory after init
    NEXUS_LOG_MEMORY();
    
    // Dump flash if requested
    nexus::test::test_runner.dump_flash();
}
```

#### In controller classes:
```cpp
void operator()(uint32_t val_) {
    uint32_t val = lp2(lp1(val_));
    if (gt(val)) {
        uint8_t const msb = val >> 3;
        uint8_t const lsb = (val << 4) & 0x7F;
        
        // Log the control change
        NEXUS_LOG_CONTROL(ctrl, msb);
        
        midi_out << cycfi::midi::control_change{0, ctrl_lsb, lsb};
        midi_out << cycfi::midi::control_change{0, ctrl, msb};
    }
}
```

#### In flash storage operations:
```cpp
bool write_to_flash(uint16_t address, uint8_t value) {
    bool success = Flash.write(address, &value, 1);
    if (!success) {
        NEXUS_LOG_ERROR(ERR_FLASH_WRITE);
    }
    return success;
}
```

### 5. Memory Usage Estimation

- DebugLogger class: ~50 bytes code (static methods only)
- Each log call: ~20-30 bytes code
- No heap allocation
- Stack usage: ~10 bytes per log call
- Total overhead when enabled: ~200-300 bytes code
- Zero overhead when disabled (macros compile to nothing)

### 6. Example SysEx Output

```
// System startup
F0 7D 4E 45 58 01 00 00 01 02 00 F7

// Volume controller change to 64
F0 7D 4E 45 58 02 12 34 07 40 F7

// Memory report: 50% heap, 200 bytes stack free
F0 7D 4E 45 58 03 23 45 32 00 C8 F7

// Flash write error
F0 7D 4E 45 58 04 34 56 02 00 F7

// Debug message with value
F0 7D 4E 45 58 06 45 67 48 69 21 0A F7
```

## Testing Plan

1. **Compile-time tests**:
   - Verify zero overhead when disabled
   - Check code size with/without debug logging

2. **Runtime tests**:
   - Verify SysEx format in MIDI-OX
   - Test all message types
   - Measure timing impact

3. **Memory tests**:
   - Monitor RAM usage during logging
   - Verify no memory leaks
   - Test under low memory conditions

## Implementation Priority

1. **Phase 1** (Essential):
   - Basic logger structure
   - System and error logging
   - Compile-time enable/disable

2. **Phase 2** (Important):
   - Controller value logging
   - Memory monitoring
   - Integration with existing code

3. **Phase 3** (Nice to have):
   - Performance metrics
   - Advanced debug features
   - Runtime configuration