# Nexus MIDI Debug Logging Architecture

## Overview

The Nexus MIDI Debug Logging system provides comprehensive debugging capabilities through MIDI System Exclusive (SysEx) messages. This allows real-time monitoring of the controller's operation without requiring a separate debug interface.

## Design Goals

1. **Minimal Memory Footprint**: The MSP430G2553 has only 512 bytes of RAM, so the logging system must be extremely memory-efficient
2. **Zero-Cost When Disabled**: When debug logging is disabled via compile-time flags, it should have zero impact on code size or performance
3. **Non-Blocking Operation**: Debug logging must not interfere with real-time MIDI performance
4. **Structured Data Format**: Use a consistent, parseable format for easy monitoring and analysis
5. **Comprehensive Coverage**: Support multiple log levels and categories for detailed debugging

## SysEx Message Format

### Message Structure
```
F0 7D 4E 45 58 <MSG_TYPE> <TIMESTAMP> <DATA...> F7

Where:
- F0: SysEx start
- 7D: Non-commercial/experimental manufacturer ID
- 4E 45 58: "NEX" in ASCII (Nexus identifier)
- MSG_TYPE: Message type/category (1 byte)
- TIMESTAMP: 2-byte timestamp (14-bit value, MSB first)
- DATA: Variable length payload (7-bit clean)
- F7: SysEx end
```

### Message Types
```
0x01: LOG_SYSTEM    - System events (startup, init, shutdown)
0x02: LOG_CONTROL   - Controller value changes
0x03: LOG_MEMORY    - Memory usage statistics
0x04: LOG_ERROR     - Error conditions
0x05: LOG_PERF      - Performance metrics
0x06: LOG_DEBUG     - General debug messages
0x07: LOG_CONFIG    - Configuration changes
0x08: LOG_STORAGE   - Flash storage operations
```

### Log Levels
```
0x00: TRACE   - Detailed trace information
0x01: DEBUG   - Debug-level messages
0x02: INFO    - Informational messages
0x03: WARNING - Warning conditions
0x04: ERROR   - Error conditions
0x05: FATAL   - Fatal errors
```

## Architecture Components

### 1. Debug Logger Class (`debug_logger.hpp`)
- Singleton pattern for global access
- Template-based for compile-time optimization
- Required ring buffer for non-blocking message queuing
- Ensures MIDI performance is never impacted by debug logging

### 2. Ring Buffer Implementation (Required)
- Fixed-size circular buffer (configurable size, default 64 bytes)
- Lock-free design for interrupt safety
- Overflow detection and reporting
- Efficient byte-level operations
- Guarantees non-blocking debug operations

```cpp
template<uint8_t SIZE>
class RingBuffer {
    uint8_t buffer[SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
    
public:
    bool put(uint8_t data) {
        uint8_t next = (head + 1) % SIZE;
        if (next == tail) return false; // Buffer full
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
};
```

### 3. Message Builder
- Efficient message construction with minimal allocations
- Support for various data types (uint8_t, uint16_t, strings)
- Automatic 7-bit encoding for SysEx compliance
- Message fragmentation for ring buffer

### 4. Integration Macros
- `NEXUS_LOG(category, level, ...)` - Main logging macro
- `NEXUS_LOG_VALUE(name, value)` - Log a named value
- `NEXUS_LOG_ERROR(code)` - Log error with code
- `NEXUS_LOG_MEMORY()` - Log current memory usage
- `NEXUS_LOG_FLUSH()` - Force flush ring buffer

### 5. Compile-Time Configuration
- `NEXUS_ENABLE_DEBUG_LOGGING` - Master enable/disable
- `NEXUS_DEBUG_LOG_LEVEL` - Minimum log level to compile
- `NEXUS_DEBUG_BUFFER_SIZE` - Ring buffer size (default 64, minimum 32)

#### Granular Category Control
Each logging category can be individually enabled/disabled at compile time:
- `NEXUS_LOG_ENABLE_SYSTEM` - System initialization, startup, shutdown
- `NEXUS_LOG_ENABLE_CONTROL` - Controller value changes
- `NEXUS_LOG_ENABLE_MEMORY` - Memory usage statistics
- `NEXUS_LOG_ENABLE_ERROR` - Error conditions
- `NEXUS_LOG_ENABLE_PERF` - Performance metrics
- `NEXUS_LOG_ENABLE_DEBUG` - General debug messages
- `NEXUS_LOG_ENABLE_CONFIG` - Configuration changes
- `NEXUS_LOG_ENABLE_STORAGE` - Flash storage operations

Example configurations:
```cpp
// Debug only system initialization
#define NEXUS_ENABLE_DEBUG_LOGGING
#define NEXUS_LOG_ENABLE_SYSTEM
#define NEXUS_LOG_ENABLE_ERROR

// Debug controllers and memory
#define NEXUS_ENABLE_DEBUG_LOGGING
#define NEXUS_LOG_ENABLE_CONTROL
#define NEXUS_LOG_ENABLE_MEMORY

// Enable all categories (default if none specified)
#define NEXUS_ENABLE_DEBUG_LOGGING
#define NEXUS_LOG_ENABLE_ALL
```

## Memory Optimization Strategies

1. **Static String Storage**: Use PROGMEM/Flash for string literals
2. **Ring Buffer Usage**: Configurable size, can be disabled for direct transmission
3. **Compile-Time Filtering**: Exclude debug code based on log level
4. **Bit-Packing**: Pack multiple values into single bytes where possible
5. **Conditional Compilation**: Zero overhead when disabled

### MSP430 Memory Calculation

For the MSP430G2553 with 512 bytes of RAM:
- **Stack grows down** from top of RAM (0x03FF)
- **Heap grows up** from end of .bss section
- **Free memory** = Stack pointer - Top of heap

```cpp
// Get free memory between heap and stack
uint16_t get_free_memory() {
    extern uint8_t __heap_start;
    extern uint8_t* __brkval;
    uint8_t* heap_end = __brkval ? __brkval : &__heap_start;
    uint8_t stack_top;
    return &stack_top - heap_end;
}

// Calculate memory usage percentage
uint8_t get_memory_usage_percent() {
    uint16_t free = get_free_memory();
    uint16_t total = 512; // MSP430G2553 RAM size
    return ((total - free) * 100) / total;
}
```

### Ring Buffer Integration (Required Component)

The ring buffer ensures non-blocking debug logging:
1. **All logging**: Goes through ring buffer (fast, non-blocking)
2. **Main loop**: Flushes buffer to MIDI output during idle time
3. **Overflow handling**: Oldest messages discarded, overflow flag set
4. **Performance guarantee**: Debug logging never blocks MIDI operations

The ring buffer is mandatory to prevent debug logging from interfering with real-time MIDI performance. Without it, SysEx transmission could cause timing issues.

## Example Debug Messages

### System Startup
```
F0 7D 4E 45 58 01 00 00 01 02 00 F7
                |  |-----|  |  |  |
                |  timestamp |  |  version
                |           |  RAM size (x10 bytes)
                LOG_SYSTEM  startup event
```

### Controller Value Change
```
F0 7D 4E 45 58 02 12 34 01 07 00 7F F7
                |  |-----|  |  |  |---|
                |  timestamp |  |  value (0-127)
                |           |  controller ID (volume)
                LOG_CONTROL value change event
```

### Memory Report
```
F0 7D 4E 45 58 03 23 45 01 64 02 00 F7
                |  |-----|  |  |  |---|
                |  timestamp |  |  stack free
                |           |  heap used %
                LOG_MEMORY  memory report
```

## Implementation Phases

1. **Phase 1**: Basic logger with system events
2. **Phase 2**: Controller value logging
3. **Phase 3**: Memory monitoring
4. **Phase 4**: Error handling and performance metrics
5. **Phase 5**: Advanced features (filtering, buffering)

## Testing Strategy

1. **Unit Tests**: Test message formatting and encoding
2. **Integration Tests**: Verify logging doesn't affect MIDI timing
3. **Memory Tests**: Ensure minimal RAM usage
4. **Performance Tests**: Measure CPU overhead
5. **MIDI Monitor Tests**: Verify messages are correctly received

## Future Enhancements

1. **Compression**: Simple compression for repeated messages
2. **Filtering**: Runtime log level adjustment via SysEx commands
3. **Querying**: Request specific debug information on demand
4. **Checksums**: Add optional checksum for data integrity