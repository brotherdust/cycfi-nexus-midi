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
- Circular buffer for message queuing (optional)
- Direct-send mode for immediate transmission

### 2. Message Builder
- Efficient message construction with minimal allocations
- Support for various data types (uint8_t, uint16_t, strings)
- Automatic 7-bit encoding for SysEx compliance

### 3. Integration Macros
- `NEXUS_LOG(category, level, ...)` - Main logging macro
- `NEXUS_LOG_VALUE(name, value)` - Log a named value
- `NEXUS_LOG_ERROR(code)` - Log error with code
- `NEXUS_LOG_MEMORY()` - Log current memory usage

### 4. Compile-Time Configuration
- `NEXUS_ENABLE_DEBUG_LOGGING` - Master enable/disable
- `NEXUS_DEBUG_LOG_LEVEL` - Minimum log level to compile
- `NEXUS_DEBUG_BUFFER_SIZE` - Size of message buffer (if used)

## Memory Optimization Strategies

1. **Static String Storage**: Use PROGMEM/Flash for string literals
2. **Minimal Buffer Usage**: Direct transmission when possible
3. **Compile-Time Filtering**: Exclude debug code based on log level
4. **Bit-Packing**: Pack multiple values into single bytes where possible
5. **Conditional Compilation**: Zero overhead when disabled

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