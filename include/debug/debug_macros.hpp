/**
 * @file debug_macros.hpp
 * @brief Convenience macros for debug logging
 *
 * This file provides macros that compile to no-ops when debug logging
 * is disabled, ensuring zero overhead in production builds. It also
 * handles granular category control based on compile-time flags.
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_DEBUG_MACROS_HPP
#define NEXUS_DEBUG_MACROS_HPP

#include "config/feature_config.hpp"

// Configuration defaults
#ifndef NEXUS_DEBUG_BUFFER_SIZE
#define NEXUS_DEBUG_BUFFER_SIZE 64  // Must be at least 32 bytes
#endif

// Validate buffer size at compile time
#if NEXUS_DEBUG_BUFFER_SIZE < 32
#error "NEXUS_DEBUG_BUFFER_SIZE must be at least 32 bytes"
#endif

// Ensure buffer size is power of 2
#if (NEXUS_DEBUG_BUFFER_SIZE & (NEXUS_DEBUG_BUFFER_SIZE - 1)) != 0
#error "NEXUS_DEBUG_BUFFER_SIZE must be a power of 2"
#endif

#ifndef NEXUS_DEBUG_LOG_LEVEL
#define NEXUS_DEBUG_LOG_LEVEL nexus::debug::LEVEL_INFO
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

// Include debug logger only when enabled
#ifdef NEXUS_ENABLE_DEBUG_LOGGING
#include "debug/debug_logger.hpp"
#endif

// Core macros (always available when logging is enabled)
#ifdef NEXUS_ENABLE_DEBUG_LOGGING

/**
 * @brief Initialize debug logging system
 */
#define NEXUS_LOG_INIT() \
    nexus::debug::DebugLogger::init()

/**
 * @brief Flush debug messages to MIDI output
 * Must be called regularly from main loop
 */
#define NEXUS_LOG_FLUSH() \
    nexus::debug::DebugLogger::flush()

/**
 * @brief Check if debug buffer has overflowed
 * @return true if overflow occurred
 */
#define NEXUS_LOG_CHECK_OVERFLOW() \
    nexus::debug::DebugLogger::check_overflow()

// Category-specific macros

#ifdef NEXUS_LOG_ENABLE_SYSTEM
    /**
     * @brief Log system event
     * @param event Event code from SystemEvent enum
     * @param p1 Optional parameter 1
     * @param p2 Optional parameter 2
     */
    #define NEXUS_LOG_SYSTEM(event, p1, p2) \
        nexus::debug::DebugLogger::log_system(event, p1, p2)
#else
    #define NEXUS_LOG_SYSTEM(event, p1, p2)
#endif

#ifdef NEXUS_LOG_ENABLE_CONTROL
    /**
     * @brief Log controller value change
     * @param ctrl Controller ID
     * @param val New value (0-127)
     */
    #define NEXUS_LOG_CONTROL(ctrl, val) \
        nexus::debug::DebugLogger::log_control(ctrl, val)
#else
    #define NEXUS_LOG_CONTROL(ctrl, val)
#endif

#ifdef NEXUS_LOG_ENABLE_MEMORY
    /**
     * @brief Log current memory usage
     */
    #define NEXUS_LOG_MEMORY() \
        nexus::debug::DebugLogger::log_memory()
#else
    #define NEXUS_LOG_MEMORY()
#endif

#ifdef NEXUS_LOG_ENABLE_ERROR
    /**
     * @brief Log error condition
     * @param code Error code from ErrorCode enum
     */
    #define NEXUS_LOG_ERROR(code) \
        nexus::debug::DebugLogger::log_error(code)
    
    /**
     * @brief Log error with context
     * @param code Error code from ErrorCode enum
     * @param ctx Context value
     */
    #define NEXUS_LOG_ERROR_CTX(code, ctx) \
        nexus::debug::DebugLogger::log_error(code, ctx)
#else
    #define NEXUS_LOG_ERROR(code)
    #define NEXUS_LOG_ERROR_CTX(code, ctx)
#endif

#ifdef NEXUS_LOG_ENABLE_DEBUG
    /**
     * @brief Log debug message
     * @param msg Message string (first 4 chars sent)
     * @param val Optional value
     */
    #define NEXUS_LOG_DEBUG(msg, val) \
        nexus::debug::DebugLogger::log_debug(msg, val)
#else
    #define NEXUS_LOG_DEBUG(msg, val)
#endif

#ifdef NEXUS_LOG_ENABLE_CONFIG
    /**
     * @brief Log configuration change
     * @param event Configuration event
     * @param p1 Parameter 1
     * @param p2 Parameter 2
     */
    #define NEXUS_LOG_CONFIG(event, p1, p2) \
        nexus::debug::DebugLogger::log_config(event, p1, p2)
#else
    #define NEXUS_LOG_CONFIG(event, p1, p2)
#endif

#ifdef NEXUS_LOG_ENABLE_STORAGE
    /**
     * @brief Log storage operation
     * @param event Storage event from StorageEvent enum
     * @param addr Memory address
     * @param val Value read/written
     */
    #define NEXUS_LOG_STORAGE(event, addr, val) \
        nexus::debug::DebugLogger::log_storage(event, addr, val)
#else
    #define NEXUS_LOG_STORAGE(event, addr, val)
#endif

#ifdef NEXUS_LOG_ENABLE_PERF
    /**
     * @brief Log performance metric
     * @param metric Metric type from PerfMetric enum
     * @param value Metric value
     */
    #define NEXUS_LOG_PERF(metric, value) \
        nexus::debug::DebugLogger::log_perf(metric, value)
#else
    #define NEXUS_LOG_PERF(metric, value)
#endif

#else // !NEXUS_ENABLE_DEBUG_LOGGING

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

#endif // NEXUS_ENABLE_DEBUG_LOGGING

#endif // NEXUS_DEBUG_MACROS_HPP