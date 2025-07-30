/**
 * @file feature_config.hpp
 * @brief Feature configuration flags for Nexus MIDI controller
 *
 * This file contains compile-time configuration options that allow
 * selective enabling/disabling of features. Features can be toggled
 * by defining or undefining the corresponding preprocessor macros.
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_FEATURE_CONFIG_HPP
#define NEXUS_FEATURE_CONFIG_HPP

/**
 * @section FEATURE_FLAGS
 *
 * Define or undefine these macros to enable/disable features at compile time.
 * When a feature is disabled, its code is completely excluded from the binary.
 */

/**
 * @def NEXUS_ENABLE_PC_CC_MAPPING
 * @brief Enable MIDI CC mapping for Program Change messages
 * 
 * When enabled, the Program Change controller will simultaneously send
 * MIDI CC messages 102-106 in a one-hot encoding pattern based on the
 * 5-way switch position (0-4).
 * 
 * Mapping:
 * - Program Change 0 → CC 102 = 127, CC 103-106 = 0
 * - Program Change 1 → CC 103 = 127, CC 102,104-106 = 0
 * - Program Change 2 → CC 104 = 127, CC 102-103,105-106 = 0
 * - Program Change 3 → CC 105 = 127, CC 102-104,106 = 0
 * - Program Change 4 → CC 106 = 127, CC 102-105 = 0
 * 
 * This feature is useful for DAWs that cannot remap program change values.
 * 
 * To disable: Comment out or remove the #define below
 * To enable: Uncomment the #define below
 * 
 * Memory impact: ~100 bytes of program memory when enabled
 * Dependencies: None
 */
#define NEXUS_ENABLE_PC_CC_MAPPING

/**
 * @def NEXUS_PC_CC_MAPPING_START
 * @brief Starting CC number for Program Change CC mapping
 *
 * Defines the first CC number for the one-hot encoding sequence.
 * The mapping will use 5 consecutive CC numbers starting from this value.
 *
 * Default: 0x66 (102 decimal, uses CC 0x66-0x6A)
 * Valid range: 0x00-0x7A (to ensure CC numbers don't exceed 0x7F)
 *
 * Examples:
 * - Start 0x66: Uses CC 0x66, 0x67, 0x68, 0x69, 0x6A (102-106)
 * - Start 0x14: Uses CC 0x14, 0x15, 0x16, 0x17, 0x18 (20-24)
 * - Start 0x40: Uses CC 0x40, 0x41, 0x42, 0x43, 0x44 (64-68)
 *
 * To change: Define a new value before including this file or via build flags
 * Example: -DNEXUS_PC_CC_MAPPING_START=0x40
 */
#ifndef NEXUS_PC_CC_MAPPING_START
    #define NEXUS_PC_CC_MAPPING_START 0x66
#endif

// Validate the starting CC number at compile time
#if NEXUS_PC_CC_MAPPING_START < 0x00 || NEXUS_PC_CC_MAPPING_START > 0x7A
    #error "NEXUS_PC_CC_MAPPING_START must be between 0x00 and 0x7A to ensure valid CC range"
#endif

/**
 * @section BUILD_FLAG_OVERRIDES
 *
 * Build flags can override the default configuration above.
 * This allows feature toggling via compiler flags without modifying this file.
 * 
 * Example usage in platformio.ini:
 *   build_flags = -DNEXUS_DISABLE_PC_CC_MAPPING
 * 
 * Or to force enable:
 *   build_flags = -DNEXUS_FORCE_PC_CC_MAPPING
 */

// Handle build flag overrides for PC CC mapping
#ifdef NEXUS_DISABLE_PC_CC_MAPPING
    #undef NEXUS_ENABLE_PC_CC_MAPPING
#endif

#ifdef NEXUS_FORCE_PC_CC_MAPPING
    #ifndef NEXUS_ENABLE_PC_CC_MAPPING
        #define NEXUS_ENABLE_PC_CC_MAPPING
    #endif
#endif

/**
 * @def NEXUS_ENABLE_IMMEDIATE_DEBOUNCE
 * @brief Enable immediate press detection with release-only debouncing
 *
 * When enabled, button handling classes will use immediate press detection
 * while still debouncing the release event. This provides more responsive
 * button presses while preventing false release triggers from switch bounce.
 *
 * Benefits:
 * - Instant button press response for better user experience
 * - Reliable release detection prevents false triggers
 * - No additional memory overhead
 * - Backward compatible - existing code works unchanged
 *
 * To disable: Comment out or remove the #define below
 * To enable: Uncomment the #define below
 *
 * Memory impact: None (uses same memory as standard debouncing)
 * Dependencies: None
 */
#define NEXUS_ENABLE_IMMEDIATE_DEBOUNCE

/**
 * @section BUILD_FLAG_OVERRIDES_DEBOUNCE
 *
 * Build flags can override the immediate debounce configuration.
 *
 * Example usage in platformio.ini:
 *   build_flags = -DNEXUS_ENABLE_IMMEDIATE_DEBOUNCE
 *
 * Or to force disable:
 *   build_flags = -DNEXUS_DISABLE_IMMEDIATE_DEBOUNCE
 */

// Handle build flag overrides for immediate debounce
#ifdef NEXUS_DISABLE_IMMEDIATE_DEBOUNCE
    #undef NEXUS_ENABLE_IMMEDIATE_DEBOUNCE
#endif

#ifdef NEXUS_FORCE_IMMEDIATE_DEBOUNCE
    #ifndef NEXUS_ENABLE_IMMEDIATE_DEBOUNCE
        #define NEXUS_ENABLE_IMMEDIATE_DEBOUNCE
    #endif
#endif

#endif // NEXUS_FEATURE_CONFIG_HPP