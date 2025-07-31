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

### 2. Ring Buffer Implementation (`debug_ring_buffer.hpp`)

```cpp
namespace nexus {
namespace debug {

template<uint8_t SIZE>
class RingBuffer {
private:
    uint8_t buffer[SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile bool overflow;
    
public:
    RingBuffer() : head(0), tail(0), overflow(false) {}
    
    bool put(uint8_t data) {
        uint8_t next = (head + 1) % SIZE;
        if (next == tail) {
            overflow = true;
            return false; // Buffer full
        }
        buffer[head] = data;
        head = next;
        return true;
    }
    
    bool get(uint8_t& data) {
        if (head == tail) return false; // Buffer empty
        data = buffer[tail];
        tail = (tail + 1) % SIZE;
        return true;
    }
    
    uint8_t available() const {
        return (SIZE + head - tail) % SIZE;
    }
    
    bool is_empty() const {
        return head == tail;
    }
    
    bool had_overflow() {
        bool result = overflow;
        overflow = false;
        return result;
    }
};

} // namespace debug
} // namespace nexus
```

### 3. Debug Logger Class (`debug_logger.hpp`)

```cpp
namespace nexus {
namespace debug {

class DebugLogger {
private:
    // Ring buffer is required for non-blocking operation
    static RingBuffer<NEXUS_DEBUG_BUFFER_SIZE> buffer;
    static uint16_t last_timestamp;
    
    // Get 14-bit timestamp with overflow handling
    static uint16_t get_timestamp() {
        uint32_t current = millis();
        // Handle overflow by using delta from last timestamp
        uint16_t delta = (current - last_timestamp) & 0x3FFF;
        last_timestamp = current;
        return delta;
    }
    
    // Send byte through ring buffer (required for non-blocking)
    static void send_byte(uint8_t byte) {
        if (!buffer.put(byte)) {
            // Buffer overflow - set flag but don't block
            // Overflow will be detected by check_overflow()
        }
    }
    
    // Send complete SysEx message
    static void send_sysex(MessageType type, const uint8_t* data, uint8_t len) {
        uint16_t ts = get_timestamp();
        
        send_byte(0xF0);
        send_byte(0x7D);
        send_byte(0x4E);  // 'N'
        send_byte(0x45);  // 'E'
        send_byte(0x58);  // 'X'
        send_byte(type);
        send_byte((ts >> 7) & 0x7F);  // Timestamp MSB
        send_byte(ts & 0x7F);          // Timestamp LSB
        
        for (uint8_t i = 0; i < len; i++) {
            send_byte(data[i] & 0x7F);  // Ensure 7-bit clean
        }
        
        send_byte(0xF7);
    }

public:
    // Initialize the logger
    static void init() {
        last_timestamp = millis();
    }
    
    // Flush buffered messages to MIDI output
    static void flush() {
        uint8_t byte;
        // Limit bytes per flush to prevent blocking
        uint8_t max_bytes = 16;  // Adjust based on timing requirements
        while (max_bytes-- && buffer.get(byte)) {
            midi_out << byte;
        }
    }
    
    // Log system event
    static void log_system(uint8_t event, uint8_t param1 = 0, uint8_t param2 = 0) {
        uint8_t data[3] = {event, param1, param2};
        send_sysex(LOG_SYSTEM, data, 3);
    }
    
    // Log controller change
    static void log_control(uint8_t controller_id, uint8_t value) {
        uint8_t data[2] = {controller_id, value};
        send_sysex(LOG_CONTROL, data, 2);
    }
    
    // Log memory usage
    static void log_memory() {
        uint16_t free_mem = get_free_memory();
        uint8_t usage_percent = get_memory_usage_percent();
        uint8_t data[3] = {
            usage_percent,
            (free_mem >> 7) & 0x7F,
            free_mem & 0x7F
        };
        send_sysex(LOG_MEMORY, data, 3);
    }
    
    // Log error
    static void log_error(ErrorCode code, uint8_t context = 0) {
        uint8_t data[2] = {code, context};
        send_sysex(LOG_ERROR, data, 2);
    }
    
    // Log debug string (first 4 chars only due to memory constraints)
    static void log_debug(const char* msg, uint8_t value = 0) {
        uint8_t data[5] = {0};
        for (uint8_t i = 0; i < 4 && msg[i]; i++) {
            data[i] = msg[i] & 0x7F;
        }
        data[4] = value;
        send_sysex(LOG_DEBUG, data, 5);
    }
    
    // Check for buffer overflow
    static bool check_overflow() {
        return buffer.had_overflow();
    }
    
private:
    // Memory calculation helpers
    static uint16_t get_free_memory() {
        extern uint8_t __heap_start;
        extern uint8_t* __brkval;
        uint8_t* heap_end = __brkval ? __brkval : &__heap_start;
        uint8_t stack_top;
        return &stack_top - heap_end;
    }
    
    static uint8_t get_memory_usage_percent() {
        uint16_t free = get_free_memory();
        uint16_t total = 512; // MSP430G2553 RAM
        return ((total - free) * 100) / total;
    }
};

// Static member definitions
RingBuffer<NEXUS_DEBUG_BUFFER_SIZE> DebugLogger::buffer;
uint16_t DebugLogger::last_timestamp = 0;

} // namespace debug
} // namespace nexus
```

### 4. Debug Macros (`debug_macros.hpp`)

```cpp
// Configuration defaults
#ifndef NEXUS_DEBUG_BUFFER_SIZE
#define NEXUS_DEBUG_BUFFER_SIZE 64  // Must be at least 32 bytes
#endif

// Validate buffer size at compile time
#if NEXUS_DEBUG_BUFFER_SIZE < 32
#error "NEXUS_DEBUG_BUFFER_SIZE must be at least 32 bytes"
#endif

#ifndef NEXUS_DEBUG_LOG_LEVEL
#define NEXUS_DEBUG_LOG_LEVEL nexus::debug::INFO
#endif

// Category enable defaults - if none specified, enable all
#ifdef NEXUS_ENABLE_DEBUG_LOGGING
    #if !defined(NEXUS_LOG_ENABLE_SYSTEM) && \
        !defined(NEXUS_LOG_ENABLE_CONTROL) && \
        !defined(NEXUS_LOG_ENABLE_MEMORY) && \
        !defined(NEXUS_LOG_ENABLE_ERROR) && \
        !defined(NEXUS_LOG_ENABLE_PERF) && \
        !defined(NEXUS_LOG_ENABLE_DEBUG) && \
        !defined(NEXUS_LOG_ENABLE_CONFIG) && \
        !defined(NEXUS_LOG_ENABLE_STORAGE) && \
        !defined(NEXUS_LOG_ENABLE_ALL)
        // No categories specified, enable all by default
        #define NEXUS_LOG_ENABLE_ALL
    #endif
    
    // If ENABLE_ALL is defined, enable all categories
    #ifdef NEXUS_LOG_ENABLE_ALL
        #define NEXUS_LOG_ENABLE_SYSTEM
        #define NEXUS_LOG_ENABLE_CONTROL
        #define NEXUS_LOG_ENABLE_MEMORY
        #define NEXUS_LOG_ENABLE_ERROR
        #define NEXUS_LOG_ENABLE_PERF
        #define NEXUS_LOG_ENABLE_DEBUG
        #define NEXUS_LOG_ENABLE_CONFIG
        #define NEXUS_LOG_ENABLE_STORAGE
    #endif
#endif

// Core macros (always available when logging is enabled)
#ifdef NEXUS_ENABLE_DEBUG_LOGGING

#define NEXUS_LOG_INIT() \
    nexus::debug::DebugLogger::init()

#define NEXUS_LOG_FLUSH() \
    nexus::debug::DebugLogger::flush()

#define NEXUS_LOG_CHECK_OVERFLOW() \
    nexus::debug::DebugLogger::check_overflow()

// Category-specific macros
#ifdef NEXUS_LOG_ENABLE_SYSTEM
    #define NEXUS_LOG_SYSTEM(event, p1, p2) \
        nexus::debug::DebugLogger::log_system(event, p1, p2)
#else
    #define NEXUS_LOG_SYSTEM(event, p1, p2)
#endif

#ifdef NEXUS_LOG_ENABLE_CONTROL
    #define NEXUS_LOG_CONTROL(ctrl, val) \
        nexus::debug::DebugLogger::log_control(ctrl, val)
#else
    #define NEXUS_LOG_CONTROL(ctrl, val)
#endif

#ifdef NEXUS_LOG_ENABLE_MEMORY
    #define NEXUS_LOG_MEMORY() \
        nexus::debug::DebugLogger::log_memory()
#else
    #define NEXUS_LOG_MEMORY()
#endif

#ifdef NEXUS_LOG_ENABLE_ERROR
    #define NEXUS_LOG_ERROR(code) \
        nexus::debug::DebugLogger::log_error(code)
    #define NEXUS_LOG_ERROR_CTX(code, ctx) \
        nexus::debug::DebugLogger::log_error(code, ctx)
#else
    #define NEXUS_LOG_ERROR(code)
    #define NEXUS_LOG_ERROR_CTX(code, ctx)
#endif

#ifdef NEXUS_LOG_ENABLE_DEBUG
    #define NEXUS_LOG_DEBUG(msg, val) \
        nexus::debug::DebugLogger::log_debug(msg, val)
#else
    #define NEXUS_LOG_DEBUG(msg, val)
#endif

#ifdef NEXUS_LOG_ENABLE_CONFIG
    #define NEXUS_LOG_CONFIG(event, p1, p2) \
        nexus::debug::DebugLogger::log_config(event, p1, p2)
#else
    #define NEXUS_LOG_CONFIG(event, p1, p2)
#endif

#ifdef NEXUS_LOG_ENABLE_STORAGE
    #define NEXUS_LOG_STORAGE(event, addr, val) \
        nexus::debug::DebugLogger::log_storage(event, addr, val)
#else
    #define NEXUS_LOG_STORAGE(event, addr, val)
#endif

#ifdef NEXUS_LOG_ENABLE_PERF
    #define NEXUS_LOG_PERF(metric, value) \
        nexus::debug::DebugLogger::log_perf(metric, value)
#else
    #define NEXUS_LOG_PERF(metric, value)
#endif

#else

// No-op macros when debugging is disabled
#define NEXUS_LOG_INIT()
#define NEXUS_LOG_FLUSH()
#define NEXUS_LOG_SYSTEM(event, p1, p2)
#define NEXUS_LOG_CONTROL(ctrl, val)
#define NEXUS_LOG_MEMORY()
#define NEXUS_LOG_ERROR(code)
#define NEXUS_LOG_ERROR_CTX(code, ctx)
#define NEXUS_LOG_DEBUG(msg, val)
#define NEXUS_LOG_CONFIG(event, p1, p2)
#define NEXUS_LOG_STORAGE(event, addr, val)
#define NEXUS_LOG_PERF(metric, value)
#define NEXUS_LOG_CHECK_OVERFLOW() false

#endif
```

### 5. Integration Points

#### In `nexus_midi.ino` setup():
```cpp
void setup() {
    // Initialize hardware pins
    nexus::config::initialize_pins();
    
    midi_out.start();
    
    // Initialize debug logging
    NEXUS_LOG_INIT();
    
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

#### In main loop():
```cpp
void loop() {
    // Process controllers
    nexus::controllers::controllers.process_inputs();
    
    // Flush debug messages during idle time (required)
    static uint32_t last_flush = 0;
    uint32_t now = millis();
    if (now - last_flush > 5) {  // Flush every 5ms for responsiveness
        NEXUS_LOG_FLUSH();
        last_flush = now;
        
        // Check for buffer overflow periodically
        static uint32_t last_overflow_check = 0;
        if (now - last_overflow_check > 100) {  // Check every 100ms
            if (NEXUS_LOG_CHECK_OVERFLOW()) {
                NEXUS_LOG_ERROR(nexus::debug::ERR_DEBUG_OVERFLOW);
            }
            last_overflow_check = now;
        }
    }
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

### 6. Memory Usage Estimation

- DebugLogger class: ~100 bytes code (with buffer support)
- Ring buffer: 64 bytes RAM (configurable)
- Each log call: ~20-30 bytes code
- No heap allocation
- Stack usage: ~10 bytes per log call
- Total overhead when enabled: ~300-400 bytes code + buffer RAM
- Zero overhead when disabled (macros compile to nothing)

### 7. Feature Configuration in `feature_config.hpp`

```cpp
// Debug logging configuration
#ifdef NEXUS_ENABLE_DEBUG_LOGGING
    // Buffer size in bytes (minimum 32, default 64)
    // Increase if experiencing overflow
    #ifndef NEXUS_DEBUG_BUFFER_SIZE
        #define NEXUS_DEBUG_BUFFER_SIZE 64
    #endif
    
    // Minimum log level to compile
    #ifndef NEXUS_DEBUG_LOG_LEVEL
        #define NEXUS_DEBUG_LOG_LEVEL nexus::debug::INFO
    #endif
    
    // Granular category control
    // Uncomment to enable specific categories only
    // #define NEXUS_LOG_ENABLE_SYSTEM   // System init, startup, shutdown
    // #define NEXUS_LOG_ENABLE_CONTROL  // Controller value changes
    // #define NEXUS_LOG_ENABLE_MEMORY   // Memory usage stats
    // #define NEXUS_LOG_ENABLE_ERROR    // Error conditions
    // #define NEXUS_LOG_ENABLE_PERF     // Performance metrics
    // #define NEXUS_LOG_ENABLE_DEBUG    // General debug messages
    // #define NEXUS_LOG_ENABLE_CONFIG   // Configuration changes
    // #define NEXUS_LOG_ENABLE_STORAGE  // Flash storage operations
    
    // Or enable all categories
    // #define NEXUS_LOG_ENABLE_ALL
    
    // Add debug overflow error code
    namespace nexus { namespace debug {
        enum { ERR_DEBUG_OVERFLOW = 0x06 };
    }}
#endif
```

### 8. Example Configurations

```cpp
// Example 1: Debug only system initialization and errors
#define NEXUS_ENABLE_DEBUG_LOGGING
#define NEXUS_LOG_ENABLE_SYSTEM
#define NEXUS_LOG_ENABLE_ERROR

// Example 2: Debug controller behavior
#define NEXUS_ENABLE_DEBUG_LOGGING
#define NEXUS_LOG_ENABLE_CONTROL
#define NEXUS_LOG_ENABLE_MEMORY

// Example 3: Debug storage operations
#define NEXUS_ENABLE_DEBUG_LOGGING
#define NEXUS_LOG_ENABLE_STORAGE
#define NEXUS_LOG_ENABLE_ERROR

// Example 4: Verbose debugging (all categories)
#define NEXUS_ENABLE_DEBUG_LOGGING
#define NEXUS_LOG_ENABLE_ALL

// Example 5: Production build with minimal debugging
#define NEXUS_ENABLE_DEBUG_LOGGING
#define NEXUS_LOG_ENABLE_ERROR
#define NEXUS_DEBUG_LOG_LEVEL nexus::debug::ERROR
```

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