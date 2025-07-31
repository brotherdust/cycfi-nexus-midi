/**
 * @file debug_categories.hpp
 * @brief Debug logging categories and definitions
 *
 * This file contains the definitions for debug message types, log levels,
 * and event codes used throughout the debug logging system.
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_DEBUG_CATEGORIES_HPP
#define NEXUS_DEBUG_CATEGORIES_HPP

#include <stdint.h>

namespace nexus { namespace debug {

/**
 * @brief Message types for SysEx debug messages
 *
 * These values are used as the MSG_TYPE byte in the SysEx format.
 * Each category can be individually enabled/disabled at compile time.
 */
enum MessageType {
    LOG_SYSTEM    = 0x01,  ///< System events (startup, init, shutdown)
    LOG_CONTROL   = 0x02,  ///< Controller value changes
    LOG_MEMORY    = 0x03,  ///< Memory usage statistics
    LOG_ERROR     = 0x04,  ///< Error conditions
    LOG_PERF      = 0x05,  ///< Performance metrics
    LOG_DEBUG     = 0x06,  ///< General debug messages
    LOG_CONFIG    = 0x07,  ///< Configuration changes
    LOG_STORAGE   = 0x08   ///< Flash storage operations
};

/**
 * @brief Log levels for filtering messages
 *
 * Used with NEXUS_DEBUG_LOG_LEVEL to compile out messages below
 * a certain severity level.
 */
enum LogLevel {
    LEVEL_TRACE   = 0,     ///< Detailed trace information
    LEVEL_DEBUG   = 1,     ///< Debug-level messages
    LEVEL_INFO    = 2,     ///< Informational messages
    LEVEL_WARNING = 3,     ///< Warning conditions
    LEVEL_ERROR   = 4,     ///< Error conditions
    LEVEL_FATAL   = 5      ///< Fatal errors
};

/**
 * @brief System event codes
 *
 * Event codes for LOG_SYSTEM messages
 */
enum SystemEvent {
    SYS_STARTUP   = 0x01,  ///< System startup
    SYS_INIT      = 0x02,  ///< Initialization phase
    SYS_SHUTDOWN  = 0x03,  ///< System shutdown
    SYS_RESET     = 0x04,  ///< System reset
    SYS_READY     = 0x05   ///< System ready
};

/**
 * @brief Controller event codes
 *
 * Event codes for LOG_CONTROL messages
 */
enum ControlEvent {
    CTRL_CHANGE   = 0x01,  ///< Controller value changed
    CTRL_INIT     = 0x02,  ///< Controller initialized
    CTRL_RESET    = 0x03   ///< Controller reset
};

/**
 * @brief Error codes
 *
 * Error codes for LOG_ERROR messages
 */
enum ErrorCode {
    ERR_NONE           = 0x00,  ///< No error
    ERR_MEMORY_LOW     = 0x01,  ///< Low memory condition
    ERR_FLASH_WRITE    = 0x02,  ///< Flash write error
    ERR_FLASH_READ     = 0x03,  ///< Flash read error
    ERR_INVALID_CONFIG = 0x04,  ///< Invalid configuration
    ERR_MIDI_OVERFLOW  = 0x05,  ///< MIDI buffer overflow
    ERR_DEBUG_OVERFLOW = 0x06   ///< Debug buffer overflow
};

/**
 * @brief Storage event codes
 *
 * Event codes for LOG_STORAGE messages
 */
enum StorageEvent {
    STOR_READ     = 0x01,  ///< Storage read operation
    STOR_WRITE    = 0x02,  ///< Storage write operation
    STOR_ERASE    = 0x03,  ///< Storage erase operation
    STOR_VERIFY   = 0x04   ///< Storage verify operation
};

/**
 * @brief Performance metric codes
 *
 * Metric codes for LOG_PERF messages
 */
enum PerfMetric {
    PERF_LOOP_TIME    = 0x01,  ///< Main loop execution time
    PERF_MIDI_LATENCY = 0x02,  ///< MIDI message latency
    PERF_CPU_USAGE    = 0x03   ///< CPU usage percentage
};

/**
 * @brief Controller IDs for debug logging
 *
 * These match the physical controller assignments
 */
enum ControllerId {
    CTRL_ID_VOLUME    = 0x07,  ///< Volume controller
    CTRL_ID_MOD       = 0x01,  ///< Modulation controller
    CTRL_ID_FX1       = 0x0C,  ///< Effects 1 controller
    CTRL_ID_FX2       = 0x0D,  ///< Effects 2 controller
    CTRL_ID_PITCH     = 0xE0,  ///< Pitch bend controller
    CTRL_ID_SUSTAIN   = 0x40,  ///< Sustain controller
    CTRL_ID_PROGRAM   = 0xC0,  ///< Program change controller
    CTRL_ID_BANK      = 0x00   ///< Bank select controller
};

}} // namespace nexus::debug

#endif // NEXUS_DEBUG_CATEGORIES_HPP