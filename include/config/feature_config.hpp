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

/**
 * @def NEXUS_ENABLE_DEBUG_LOGGING
 * @brief Enable MIDI SysEx debug logging
 *
 * When enabled, debug messages are sent via MIDI SysEx for real-time monitoring.
 * Uses a ring buffer to ensure non-blocking operation and maintains compatibility
 * with real-time MIDI performance.
 *
 * Features:
 * - Non-blocking operation via ring buffer
 * - Granular category control
 * - 7-bit ASCII compliant for MIDI SysEx
 * - Zero overhead when disabled
 *
 * To disable: Comment out or remove the #define below
 * To enable: Uncomment the #define below
 *
 * Memory impact: ~100 bytes RAM (64-byte buffer + overhead)
 * Dependencies: None
 */
#define NEXUS_ENABLE_DEBUG_LOGGING

#ifdef NEXUS_ENABLE_DEBUG_LOGGING
    /**
     * @def NEXUS_DEBUG_BUFFER_SIZE
     * @brief Size of debug message ring buffer in bytes
     *
     * Must be a power of 2 and at least 32 bytes.
     * Increase if experiencing buffer overflow.
     *
     * Default: 64 bytes
     */
    #ifndef NEXUS_DEBUG_BUFFER_SIZE
        #define NEXUS_DEBUG_BUFFER_SIZE 64
    #endif
    
    /**
     * @def NEXUS_DEBUG_LOG_LEVEL
     * @brief Minimum log level to compile
     *
     * Messages below this level are excluded at compile time.
     * Values: TRACE(0), DEBUG(1), INFO(2), WARNING(3), ERROR(4), FATAL(5)
     *
     * Default: INFO (2)
     */
    #ifndef NEXUS_DEBUG_LOG_LEVEL
        #define NEXUS_DEBUG_LOG_LEVEL 2
    #endif
    
    /**
     * @section DEBUG_CATEGORY_CONTROL
     * @brief Granular control over debug categories
     *
     * Enable specific debug categories by defining the corresponding macros.
     * If no categories are specified, all are enabled by default.
     *
     * Available categories:
     * - NEXUS_LOG_ENABLE_SYSTEM   : System initialization, startup, shutdown
     * - NEXUS_LOG_ENABLE_CONTROL  : Controller value changes
     * - NEXUS_LOG_ENABLE_MEMORY   : Memory usage statistics
     * - NEXUS_LOG_ENABLE_ERROR    : Error conditions
     * - NEXUS_LOG_ENABLE_PERF     : Performance metrics
     * - NEXUS_LOG_ENABLE_DEBUG    : General debug messages
     * - NEXUS_LOG_ENABLE_CONFIG   : Configuration changes
     * - NEXUS_LOG_ENABLE_STORAGE  : Flash storage operations
     * - NEXUS_LOG_ENABLE_ALL      : Enable all categories
     *
     * Example configurations:
     * @code
     * // Debug only system initialization and errors
     * #define NEXUS_LOG_ENABLE_SYSTEM
     * #define NEXUS_LOG_ENABLE_ERROR
     *
     * // Debug controller behavior
     * #define NEXUS_LOG_ENABLE_CONTROL
     * #define NEXUS_LOG_ENABLE_MEMORY
     *
     * // Enable all categories
     * #define NEXUS_LOG_ENABLE_ALL
     * @endcode
     */
    
    // Uncomment to enable specific categories
    #define NEXUS_LOG_ENABLE_SYSTEM
    #define NEXUS_LOG_ENABLE_CONTROL
    #define NEXUS_LOG_ENABLE_MEMORY
    #define NEXUS_LOG_ENABLE_ERROR
    // #define NEXUS_LOG_ENABLE_PERF
    // #define NEXUS_LOG_ENABLE_DEBUG
    // #define NEXUS_LOG_ENABLE_CONFIG
    // #define NEXUS_LOG_ENABLE_STORAGE
    // #define NEXUS_LOG_ENABLE_ALL
#endif

/**
 * @section BUILD_FLAG_OVERRIDES_DEBUG
 *
 * Build flags can override the debug logging configuration.
 *
 * Example usage in platformio.ini:
 *   build_flags = -DNEXUS_ENABLE_DEBUG_LOGGING -DNEXUS_LOG_ENABLE_ERROR
 *
 * Or to force disable:
 *   build_flags = -DNEXUS_DISABLE_DEBUG_LOGGING
 */

// Handle build flag overrides for debug logging
#ifdef NEXUS_DISABLE_DEBUG_LOGGING
    #undef NEXUS_ENABLE_DEBUG_LOGGING
#endif

#ifdef NEXUS_FORCE_DEBUG_LOGGING
    #ifndef NEXUS_ENABLE_DEBUG_LOGGING
        #define NEXUS_ENABLE_DEBUG_LOGGING
    #endif
#endif

#endif // NEXUS_FEATURE_CONFIG_HPP