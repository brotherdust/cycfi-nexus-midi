# Program Change CC Mapping Configuration

## Overview

The Nexus MIDI controller includes an optional feature that simultaneously sends MIDI CC (Control Change) messages when Program Change messages are sent from the 5-way switch. This feature provides compatibility with DAWs that cannot remap program change values to other MIDI messages.

## Feature Description

When enabled, the Program Change controller will:
1. Send the normal Program Change message (as before)
2. Additionally send CC messages in a one-hot encoding pattern (default: CC 0x66-0x6A / 102-106)

### Default Mapping Table (Start = 0x66)

| Switch Position | Program Change | CC 0x66 (102) | CC 0x67 (103) | CC 0x68 (104) | CC 0x69 (105) | CC 0x6A (106) |
|----------------|----------------|---------------|---------------|---------------|---------------|---------------|
| 0              | 0 + base       | 127           | 0             | 0             | 0             | 0             |
| 1              | 1 + base       | 0             | 127           | 0             | 0             | 0             |
| 2              | 2 + base       | 0             | 0             | 127           | 0             | 0             |
| 3              | 3 + base       | 0             | 0             | 0             | 127           | 0             |
| 4              | 4 + base       | 0             | 0             | 0             | 0             | 127           |

**Note:** The CC mapping is based only on the raw switch position (0-4), not the final program change value that includes the base offset.

### Configurable CC Range

The starting CC number can be configured at compile time. The system will use 5 consecutive CC numbers starting from the configured value.

Examples:
- Start = 0x66 (default): Uses CC 0x66-0x6A (102-106)
- Start = 0x14: Uses CC 0x14-0x18 (20-24)
- Start = 0x40: Uses CC 0x40-0x44 (64-68)

## Configuration Methods

### Method 1: Edit Configuration File (Recommended)

Edit `include/config/feature_config.hpp`:

```cpp
// To ENABLE the feature (default):
#define NEXUS_ENABLE_PC_CC_MAPPING

// To DISABLE the feature:
// #define NEXUS_ENABLE_PC_CC_MAPPING

// To change the starting CC number (default is 0x66):
#define NEXUS_PC_CC_MAPPING_START 0x40  // Uses CC 0x40-0x44
```

### Method 2: Build Flags in platformio.ini

Add build flags to your `platformio.ini` file:

```ini
[env:lpmsp430g2553]
; To disable the feature:
build_flags = -DNEXUS_DISABLE_PC_CC_MAPPING

; To force enable the feature:
build_flags = -DNEXUS_FORCE_PC_CC_MAPPING

; To change the starting CC number:
build_flags = -DNEXUS_PC_CC_MAPPING_START=0x40

; Multiple flags can be combined:
build_flags = 
    -DNEXUS_FORCE_PC_CC_MAPPING
    -DNEXUS_PC_CC_MAPPING_START=0x14
```

### Method 3: Compiler Command Line

When building manually, add flags to the compiler command:

```bash
# To disable:
msp430-gcc -DNEXUS_DISABLE_PC_CC_MAPPING ...

# To force enable:
msp430-gcc -DNEXUS_FORCE_PC_CC_MAPPING ...

# To change starting CC:
msp430-gcc -DNEXUS_PC_CC_MAPPING_START=0x40 ...
```

## Build Flag Priority

The configuration system follows this priority order:
1. `NEXUS_FORCE_PC_CC_MAPPING` - Forces the feature ON (highest priority)
2. `NEXUS_DISABLE_PC_CC_MAPPING` - Forces the feature OFF
3. `NEXUS_ENABLE_PC_CC_MAPPING` in feature_config.hpp - Default setting (lowest priority)

The starting CC number (`NEXUS_PC_CC_MAPPING_START`) can be set via build flags or in the configuration file. Build flags take precedence.

## Memory Impact

- **When Enabled:** Approximately 100 bytes of additional program memory
- **When Disabled:** The feature code is completely excluded from the binary

## Dependencies

- No additional dependencies required
- Uses existing MIDI library functionality
- Compatible with all MSP430 configurations

## Verification

To verify the feature state after building:

1. Check the build output for the presence of `send_cc_mapping` symbol
2. Monitor MIDI output to confirm CC messages are sent (or not sent)
3. Use a MIDI monitor tool to observe the message stream

## Use Cases

This feature is particularly useful for:
- DAWs that only respond to CC messages for automation
- MIDI learn functions that don't recognize program changes
- Creating custom control mappings without DAW-specific configuration
- Providing redundant control signals for reliability

## Troubleshooting

If the feature isn't working as expected:

1. Verify the configuration in `feature_config.hpp`
2. Check for conflicting build flags in `platformio.ini`
3. Ensure a clean build after configuration changes
4. Confirm the 5-way switch is generating values 0-4
5. Use a MIDI monitor to verify message output

## Example Custom Configurations

### Using General Purpose Controllers
```cpp
// Use CC 0x10-0x14 (General Purpose Controllers 1-4 + undefined)
#define NEXUS_PC_CC_MAPPING_START 0x10
```

### Using Sound Controllers
```cpp
// Use CC 0x46-0x4A (Sound Controllers 1-5)
#define NEXUS_PC_CC_MAPPING_START 0x46
```

### Using Undefined CC Range
```cpp
// Use CC 0x14-0x18 (Undefined controllers 20-24)
#define NEXUS_PC_CC_MAPPING_START 0x14