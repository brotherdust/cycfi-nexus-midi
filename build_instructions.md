# Build Instructions for Phase 1 Validation

## How to Build in VSCode with PlatformIO

### Method 1: Keyboard Shortcut
- **Windows/Linux**: Press `Ctrl + Alt + B`
- **Mac**: Press `Cmd + Option + B`

### Method 2: Status Bar
- Look at the bottom of VSCode
- Click the checkmark icon (✓) labeled "PlatformIO: Build"

### Method 3: Command Palette
1. Press `Ctrl + Shift + P` (or `Cmd + Shift + P` on Mac)
2. Type "PlatformIO: Build"
3. Press Enter

## What to Look For

### Success Indicators:
- "SUCCESS" message in the terminal
- No red error messages
- Memory usage statistics at the end

### Expected Output Format:
```
Building...
Compiling .pio/build/lpmsp430g2553/src/nexus_midi.ino.cpp.o
Compiling .pio/build/lpmsp430g2553/src/config/hardware_config.cpp.o
...
Linking .pio/build/lpmsp430g2553/firmware.elf
Building .pio/build/lpmsp430g2553/firmware.hex
Calculating size...
RAM:   [=         ]   X.X% (used XXX bytes from 512 bytes)
Flash: [==        ]   X.X% (used XXXX bytes from 16384 bytes)
========================= [SUCCESS] Took X.XX seconds =========================
```

## After Successful Build

Run this command in the terminal to get detailed memory usage:
```bash
/Users/jarad/.platformio/packages/toolchain-timsp430/bin/msp430-size -B .pio/build/lpmsp430g2553/firmware.elf
```

## Baseline Comparison

**Before refactoring:**
- Flash: 5,630 bytes (34.4%)
- RAM: 226 bytes (44.1%)

**Expected after Phase 1:**
- Flash: ~5,650-5,700 bytes (< 35%)
- RAM: ~226-230 bytes (< 45%)

## If Build Fails

Common issues and solutions:
1. **Missing include path**: Check that all includes use correct paths
2. **Undefined reference**: Ensure all functions are properly declared/defined
3. **Multiple definition**: Check for duplicate definitions

Share the error message and I'll help fix it!