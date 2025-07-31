# Nexus MIDI Debug Logging Usage Guide

## Quick Start

### 1. Enable Debug Logging

Add to `platformio.ini`:
```ini
build_flags = 
    -std=gnu++0x
    -D__SINGLE_PRECISION__=1
    -Wno-overflow
    -Os
    -ffunction-sections
    -fdata-sections
    -Wl,--gc-sections
    -DNEXUS_ENABLE_DEBUG_LOGGING    ; Enable debug logging
```

Or define in `include/config/feature_config.hpp`:
```cpp
#define NEXUS_ENABLE_DEBUG_LOGGING
```

### 2. Set Up MIDI Monitoring

1. Connect your MIDI interface to your computer
2. Open MIDI-OX (Windows) or similar MIDI monitor
3. Configure to display SysEx messages in hex format
4. Set filter to show messages starting with `F0 7D 4E 45 58`

## Using Debug Macros

### System Events
```cpp
// Log system startup (event=0x01, version=2.0)
NEXUS_LOG_SYSTEM(0x01, 0x02, 0x00);

// Log initialization complete (event=0x02)
NEXUS_LOG_SYSTEM(0x02, 0x00, 0x00);

// Log entering sleep mode (event=0x03)
NEXUS_LOG_SYSTEM(0x03, 0x00, 0x00);
```

### Controller Changes
```cpp
// Log volume change
NEXUS_LOG_CONTROL(0x07, new_value);  // 0x07 = volume controller

// Log modulation change
NEXUS_LOG_CONTROL(0x01, new_value);  // 0x01 = modulation

// Log custom controller
NEXUS_LOG_CONTROL(controller_id, value);
```

### Error Logging
```cpp
// Log flash write error
NEXUS_LOG_ERROR(nexus::debug::ERR_FLASH_WRITE);

// Log memory low condition
NEXUS_LOG_ERROR(nexus::debug::ERR_MEMORY_LOW);

// Log with context
NEXUS_LOG_ERROR(nexus::debug::ERR_INVALID_CONFIG, config_id);
```

### Memory Monitoring
```cpp
// Log current memory usage
NEXUS_LOG_MEMORY();  // Automatically calculates heap/stack usage
```

### Generic Debug Messages
```cpp
// Log a debug message with value
NEXUS_LOG_DEBUG("Init", 0x01);

// Log a simple debug marker
NEXUS_LOG_DEBUG("Here", 0x00);
```

## Interpreting SysEx Messages

### Message Format
```
F0 7D 4E 45 58 TT TS TS [DATA...] F7

F0 7D        - SysEx start, experimental manufacturer ID
4E 45 58     - "NEX" identifier
TT           - Message type
TS TS        - Timestamp (14-bit, big-endian)
DATA         - Variable length payload
F7           - SysEx end
```

### Message Types
- `01` - System events
- `02` - Controller changes
- `03` - Memory reports
- `04` - Error conditions
- `05` - Performance metrics
- `06` - Debug messages
- `07` - Configuration changes
- `08` - Storage operations

### Example Decoding

#### System Startup
```
F0 7D 4E 45 58 01 00 10 01 02 00 F7
                |  |-----|  |  |
                |  timestamp |  version 0
                |           event: startup
                LOG_SYSTEM
```

#### Controller Change
```
F0 7D 4E 45 58 02 12 34 07 40 F7
                |  |-----|  |  |
                |  timestamp |  value: 64
                |           controller: volume (0x07)
                LOG_CONTROL
```

#### Memory Report
```
F0 7D 4E 45 58 03 23 45 32 00 C8 F7
                |  |-----|  |  |---|
                |  timestamp |  stack free: 200 bytes
                |           heap used: 50%
                LOG_MEMORY
```

## Advanced Usage

### Conditional Logging
```cpp
#ifdef NEXUS_DEBUG_VERBOSE
    NEXUS_LOG_DEBUG("Detailed", value);
#endif
```

### Performance Monitoring
```cpp
uint32_t start_time = millis();
// ... operation ...
uint32_t duration = millis() - start_time;
if (duration > 10) {
    NEXUS_LOG_DEBUG("Slow", duration);
}
```

### State Tracking
```cpp
static uint8_t last_value = 0;
if (abs(new_value - last_value) > 10) {
    NEXUS_LOG_CONTROL(controller_id, new_value);
    last_value = new_value;
}
```

## Troubleshooting

### No SysEx Messages Appearing
1. Check that `NEXUS_ENABLE_DEBUG_LOGGING` is defined
2. Verify MIDI connection and baud rate (31250)
3. Check MIDI monitor SysEx filter settings
4. Ensure MIDI interface supports SysEx

### Garbled Messages
1. Check for buffer overflows
2. Verify all data is 7-bit clean (& 0x7F)
3. Ensure proper F0/F7 framing

### Performance Impact
1. Disable debug logging for production builds
2. Use conditional compilation for verbose logging
3. Limit logging frequency in tight loops

## Best Practices

1. **Use Appropriate Log Levels**
   - System events for major state changes
   - Control logs only for significant changes
   - Error logs for actual errors, not warnings

2. **Minimize Logging in ISRs**
   - Avoid logging in interrupt handlers
   - Queue messages for main loop processing if needed

3. **Memory Awareness**
   - Check memory usage after adding logging
   - Remove verbose logging in production

4. **Message Throttling**
   - Don't log every analog read
   - Use change thresholds for continuous controllers
   - Implement rate limiting for repeated events

## Example Integration

```cpp
// In a controller class
void process() {
    uint16_t raw_value = analogRead(pin);
    uint8_t midi_value = raw_value >> 3;
    
    if (midi_value != last_value) {
        // Only log significant changes
        if (abs(midi_value - last_value) > 5) {
            NEXUS_LOG_CONTROL(controller_id, midi_value);
        }
        
        // Always send MIDI
        send_midi(midi_value);
        last_value = midi_value;
    }
}

// In error handling
bool save_config() {
    if (!flash.write(addr, data)) {
        NEXUS_LOG_ERROR(ERR_FLASH_WRITE);
        return false;
    }
    NEXUS_LOG_SYSTEM(0x07, 0x01, 0x00);  // Config saved
    return true;
}
```

## Memory Usage Guidelines

- Each log call: ~20-30 bytes of code
- No dynamic memory allocation
- Stack usage: ~10 bytes per call
- Total overhead: ~200-300 bytes when enabled
- Zero overhead when disabled

## Future Enhancements

1. **Bidirectional Communication**
   - Receive SysEx commands to control logging
   - Query specific debug information

2. **Log Filtering**
   - Runtime enable/disable of categories
   - Adjustable verbosity levels

3. **Data Compression**
   - Run-length encoding for repeated values
   - Delta encoding for sequential data