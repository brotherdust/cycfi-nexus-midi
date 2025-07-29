# Memory Usage Analysis for Nexus MIDI Controller

## How to Check Current Memory Usage

### Method 1: Using PlatformIO (Recommended)

Run the following command in the terminal:
```bash
pio run -v
```

Look for output similar to:
```
Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
RAM:   [=         ]   X.X% (used XX bytes from 512 bytes)
Flash: [==        ]   X.X% (used XXXX bytes from 16384 bytes)
```

### Method 2: Detailed Memory Analysis

For more detailed analysis:
```bash
pio run --target size
```

This will show:
- Text (code) size
- Data (initialized variables) size  
- BSS (uninitialized variables) size
- Total RAM usage (data + bss)
- Total Flash usage (text + data)

### Method 3: Generate Memory Map

To see exactly what's using memory:
```bash
pio run -v | grep -A 20 "Checking size"
```

Or generate a map file:
```bash
# Add to platformio.ini:
build_flags = 
    -Wl,-Map,output.map
```

## Current Code Memory Estimation

Based on the code analysis:

### Global Variables (RAM Usage)
- `midi_out` (midi_stream): ~4-8 bytes
- Pin constants: 0 bytes (const in flash)
- Controller instances:
  - Each lowpass filter: ~8 bytes (2 × int32_t)
  - Each gate: ~4 bytes (int32_t)
  - Each controller: ~24-32 bytes
  - Total controllers: ~200-250 bytes
- Flash instances: ~8 bytes (2 pointers)
- Save delay variables: ~8 bytes
- Test mode variables: ~10-20 bytes

**Estimated Total RAM**: ~250-300 bytes (49-59% of 512B)

### Code Size (Flash Usage)
- MIDI library: ~2-3KB
- Util library: ~1-2KB
- Main code: ~3-4KB
- Arduino framework: ~2-3KB

**Estimated Total Flash**: ~8-12KB (49-73% of 16KB)

## Memory Impact of Refactoring

### Potential Increases:
1. **Virtual Tables**: Each virtual function adds ~2 bytes per function
2. **Additional Functions**: Call overhead for modular functions
3. **Namespace Overhead**: Minimal, mostly compile-time
4. **Static Initialization**: Singleton pattern may add ~50-100 bytes

### Potential Optimizations:
1. **Inline Functions**: Can reduce code size for small functions
2. **Constexpr**: Moves calculations to compile-time
3. **Template Optimization**: Compiler can optimize better with clear boundaries

## Recommended Memory Thresholds

For MSP430G2553:
- **RAM Usage**: Keep below 80% (409 bytes) for stack space
- **Flash Usage**: Keep below 90% (14.7KB) for future updates

## Analysis Commands Script

Create this script as `check_memory.sh`:
```bash
#!/bin/bash
echo "=== Nexus MIDI Memory Analysis ==="
echo ""
echo "Building project..."
pio run > build_output.txt 2>&1

echo "Current Memory Usage:"
grep -A 2 "RAM:" build_output.txt
echo ""

echo "Detailed Size Information:"
pio run --target size | grep -A 10 "text"
echo ""

echo "Top 10 Largest Symbols:"
arm-none-eabi-nm -S --size-sort .pio/build/lpmsp430g2553/firmware.elf | tail -10
echo ""

echo "=== Analysis Complete ==="
```

## Refactoring Decision Matrix

Based on memory usage results:

| Current RAM Usage | Current Flash Usage | Recommended Approach |
|-------------------|---------------------|---------------------|
| < 60% (307B) | < 60% (9.8KB) | Full modular refactoring safe |
| 60-75% (307-384B) | 60-75% (9.8-12.3KB) | Simplified refactoring recommended |
| 75-85% (384-435B) | 75-85% (12.3-13.9KB) | Minimal refactoring only |
| > 85% (435B+) | > 85% (13.9KB+) | Optimization before refactoring |

## Next Steps

1. Run `pio run -v` to get actual memory usage
2. Compare with estimates above
3. Based on results, choose approach:
   - **Green Zone** (< 60%): Proceed with full plan
   - **Yellow Zone** (60-75%): Use simplified approach
   - **Orange Zone** (75-85%): Extract only critical modules
   - **Red Zone** (> 85%): Focus on optimization first

## Memory Optimization Tips

If memory is tight:

### RAM Optimizations:
1. Use `uint8_t` instead of `int` where possible
2. Pack boolean flags into bit fields
3. Reuse buffers for temporary data
4. Move constants to PROGMEM (Flash)

### Flash Optimizations:
1. Use `-Os` optimization flag
2. Remove unused functions with `-ffunction-sections`
3. Combine similar code patterns
4. Use macros instead of inline functions for tiny operations

### Example Optimization:
```cpp
// Before: Multiple booleans (4 bytes each)
bool flag1, flag2, flag3, flag4;

// After: Bit field (1 byte total)
struct {
    uint8_t flag1:1;
    uint8_t flag2:1;
    uint8_t flag3:1;
    uint8_t flag4:1;
    uint8_t reserved:4;
} flags;