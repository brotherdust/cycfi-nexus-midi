/**
 * @file debug_logger.hpp
 * @brief Main debug logger implementation
 *
 * This file contains the DebugLogger class which handles formatting and
 * sending debug messages via MIDI SysEx. It uses a ring buffer to ensure
 * non-blocking operation and maintains compatibility with real-time MIDI.
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_DEBUG_LOGGER_HPP
#define NEXUS_DEBUG_LOGGER_HPP

#include "config/feature_config.hpp"

#ifdef NEXUS_ENABLE_DEBUG_LOGGING

#include <Arduino.h>  // For millis()
#include <stdint.h>
#include "debug/ring_buffer.hpp"
#include "debug/debug_categories.hpp"
#include "midi.hpp"

// Forward declaration for midi_out
extern cycfi::midi::midi_stream midi_out;

namespace nexus { namespace debug {

/**
 * @brief Main debug logger class
 *
 * Provides static methods for logging various types of debug information
 * via MIDI SysEx messages. Messages are formatted using the MIDI library's
 * sysex template and buffered through a ring buffer for non-blocking operation.
 */
class DebugLogger {
private:
    // Ring buffer for non-blocking operation
    static RingBuffer<NEXUS_DEBUG_BUFFER_SIZE> buffer;
    
    // Fixed-size debug message payload
    // Format: [MSG_TYPE] [TIMESTAMP_MSB] [TIMESTAMP_LSB] [DATA1] [DATA2] [DATA3] [DATA4] [DATA5]
    static const int DEBUG_PAYLOAD_SIZE = 8;
    
    // Total SysEx message size = payload + 5 (F0, 00, ID_MSB, ID_LSB, F7)
    static const int SYSEX_MESSAGE_SIZE = DEBUG_PAYLOAD_SIZE + 5;
    
    /**
     * @brief Get 14-bit timestamp with overflow handling
     *
     * @return 14-bit timestamp value
     */
    static uint16_t get_timestamp() {
        uint32_t current = millis();
        // Create 14-bit timestamp (0-16383)
        return current & 0x3FFF;
    }
    
    /**
     * @brief Buffer a complete SysEx message
     *
     * @param msg The SysEx message to buffer
     */
    static void buffer_sysex(const cycfi::midi::sysex<DEBUG_PAYLOAD_SIZE>& msg) {
        // Buffer the complete SysEx message
        // sysex<N> inherits from message<N+5>
        for (int i = 0; i < SYSEX_MESSAGE_SIZE; ++i) {
            if (!buffer.put(msg.data[i])) {
                // Buffer full - we'll lose this message
                // In production, you might want to handle this differently
                break;
            }
        }
    }
    
    // Memory calculation helpers
    static uint16_t get_free_memory();
    static uint8_t get_memory_usage_percent();
    
public:
    /**
     * @brief Initialize the logger
     */
    static void init() {
        buffer.clear();
    }
    
    /**
     * @brief Flush buffered messages to MIDI output
     *
     * Should be called regularly from main loop.
     */
    static void flush() {
        uint8_t byte;
        // Limit bytes per flush to prevent blocking
        uint8_t max_bytes = 32;  // Adjust based on timing requirements
        while (max_bytes-- && buffer.get(byte)) {
            midi_out << byte;
        }
    }
    
    /**
     * @brief Check for buffer overflow
     *
     * @return true if overflow occurred since last check
     */
    static bool check_overflow() {
        return buffer.had_overflow();
    }
    
    // Logging methods for each category
    
    /**
     * @brief Log system event
     *
     * @param event Event code from SystemEvent enum
     * @param param1 Optional parameter 1
     * @param param2 Optional parameter 2
     */
    static void log_system(uint8_t event, uint8_t param1 = 0, uint8_t param2 = 0) {
        uint8_t payload[DEBUG_PAYLOAD_SIZE];
        uint16_t ts = get_timestamp();
        
        payload[0] = LOG_SYSTEM;              // Message type
        payload[1] = (ts >> 7) & 0x7F;        // Timestamp MSB
        payload[2] = ts & 0x7F;               // Timestamp LSB
        payload[3] = event & 0x7F;            // Event code
        payload[4] = param1 & 0x7F;           // Parameter 1
        payload[5] = param2 & 0x7F;           // Parameter 2
        payload[6] = 0;                       // Reserved
        payload[7] = 0;                       // Reserved
        
        // Create and buffer SysEx message
        cycfi::midi::sysex<DEBUG_PAYLOAD_SIZE> msg(0x7D4E, payload);
        buffer_sysex(msg);
    }
    
    /**
     * @brief Log controller change
     *
     * @param controller_id Controller ID from ControllerId enum
     * @param value New controller value (0-127)
     */
    static void log_control(uint8_t controller_id, uint8_t value) {
        uint8_t payload[DEBUG_PAYLOAD_SIZE];
        uint16_t ts = get_timestamp();
        
        payload[0] = LOG_CONTROL;             // Message type
        payload[1] = (ts >> 7) & 0x7F;        // Timestamp MSB
        payload[2] = ts & 0x7F;               // Timestamp LSB
        payload[3] = CTRL_CHANGE;             // Event: controller change
        payload[4] = controller_id & 0x7F;    // Controller ID
        payload[5] = value & 0x7F;            // Controller value
        payload[6] = 0;                       // Reserved
        payload[7] = 0;                       // Reserved
        
        cycfi::midi::sysex<DEBUG_PAYLOAD_SIZE> msg(0x7D4E, payload);
        buffer_sysex(msg);
    }
    
    /**
     * @brief Log memory usage
     */
    static void log_memory() {
        uint16_t free_mem = get_free_memory();
        uint8_t usage_percent = get_memory_usage_percent();
        
        uint8_t payload[DEBUG_PAYLOAD_SIZE];
        uint16_t ts = get_timestamp();
        
        payload[0] = LOG_MEMORY;              // Message type
        payload[1] = (ts >> 7) & 0x7F;        // Timestamp MSB
        payload[2] = ts & 0x7F;               // Timestamp LSB
        payload[3] = usage_percent & 0x7F;    // Usage percentage
        payload[4] = (free_mem >> 7) & 0x7F;  // Free memory MSB
        payload[5] = free_mem & 0x7F;         // Free memory LSB
        payload[6] = 0;                       // Reserved
        payload[7] = 0;                       // Reserved
        
        cycfi::midi::sysex<DEBUG_PAYLOAD_SIZE> msg(0x7D4E, payload);
        buffer_sysex(msg);
    }
    
    /**
     * @brief Log error
     *
     * @param code Error code from ErrorCode enum
     * @param context Optional context value
     */
    static void log_error(ErrorCode code, uint8_t context = 0) {
        uint8_t payload[DEBUG_PAYLOAD_SIZE];
        uint16_t ts = get_timestamp();
        
        payload[0] = LOG_ERROR;               // Message type
        payload[1] = (ts >> 7) & 0x7F;        // Timestamp MSB
        payload[2] = ts & 0x7F;               // Timestamp LSB
        payload[3] = code & 0x7F;             // Error code
        payload[4] = context & 0x7F;          // Context
        payload[5] = 0;                       // Reserved
        payload[6] = 0;                       // Reserved
        payload[7] = 0;                       // Reserved
        
        cycfi::midi::sysex<DEBUG_PAYLOAD_SIZE> msg(0x7D4E, payload);
        buffer_sysex(msg);
    }
    
    /**
     * @brief Log debug string (first 4 chars only due to memory constraints)
     *
     * @param msg Debug message (only first 4 chars will be sent)
     * @param value Optional numeric value
     */
    static void log_debug(const char* str, uint8_t value = 0) {
        uint8_t payload[DEBUG_PAYLOAD_SIZE];
        uint16_t ts = get_timestamp();
        
        payload[0] = LOG_DEBUG;               // Message type
        payload[1] = (ts >> 7) & 0x7F;        // Timestamp MSB
        payload[2] = ts & 0x7F;               // Timestamp LSB
        
        // Pack 4 characters into payload
        for (uint8_t i = 0; i < 4; i++) {
            if (str && str[i]) {
                payload[3 + i] = str[i] & 0x7F;
            } else {
                payload[3 + i] = 0x20;  // Space padding
            }
        }
        
        payload[7] = value & 0x7F;            // Value
        
        cycfi::midi::sysex<DEBUG_PAYLOAD_SIZE> msg(0x7D4E, payload);
        buffer_sysex(msg);
    }
    
    /**
     * @brief Log configuration change
     *
     * @param event Configuration event
     * @param param1 Parameter 1
     * @param param2 Parameter 2
     */
    static void log_config(uint8_t event, uint8_t param1 = 0, uint8_t param2 = 0) {
        uint8_t payload[DEBUG_PAYLOAD_SIZE];
        uint16_t ts = get_timestamp();
        
        payload[0] = LOG_CONFIG;              // Message type
        payload[1] = (ts >> 7) & 0x7F;        // Timestamp MSB
        payload[2] = ts & 0x7F;               // Timestamp LSB
        payload[3] = event & 0x7F;            // Event code
        payload[4] = param1 & 0x7F;           // Parameter 1
        payload[5] = param2 & 0x7F;           // Parameter 2
        payload[6] = 0;                       // Reserved
        payload[7] = 0;                       // Reserved
        
        cycfi::midi::sysex<DEBUG_PAYLOAD_SIZE> msg(0x7D4E, payload);
        buffer_sysex(msg);
    }
    
    /**
     * @brief Log storage operation
     *
     * @param event Storage event from StorageEvent enum
     * @param address Memory address (14-bit)
     * @param value Value read/written
     */
    static void log_storage(uint8_t event, uint16_t address, uint8_t value) {
        uint8_t payload[DEBUG_PAYLOAD_SIZE];
        uint16_t ts = get_timestamp();
        
        payload[0] = LOG_STORAGE;             // Message type
        payload[1] = (ts >> 7) & 0x7F;        // Timestamp MSB
        payload[2] = ts & 0x7F;               // Timestamp LSB
        payload[3] = event & 0x7F;            // Event code
        payload[4] = (address >> 7) & 0x7F;   // Address MSB
        payload[5] = address & 0x7F;          // Address LSB
        payload[6] = value & 0x7F;            // Value
        payload[7] = 0;                       // Reserved
        
        cycfi::midi::sysex<DEBUG_PAYLOAD_SIZE> msg(0x7D4E, payload);
        buffer_sysex(msg);
    }
    
    /**
     * @brief Log performance metric
     *
     * @param metric Metric type from PerfMetric enum
     * @param value Metric value
     */
    static void log_perf(uint8_t metric, uint8_t value) {
        uint8_t payload[DEBUG_PAYLOAD_SIZE];
        uint16_t ts = get_timestamp();
        
        payload[0] = LOG_PERF;                // Message type
        payload[1] = (ts >> 7) & 0x7F;        // Timestamp MSB
        payload[2] = ts & 0x7F;               // Timestamp LSB
        payload[3] = metric & 0x7F;           // Metric type
        payload[4] = value & 0x7F;            // Metric value
        payload[5] = 0;                       // Reserved
        payload[6] = 0;                       // Reserved
        payload[7] = 0;                       // Reserved
        
        cycfi::midi::sysex<DEBUG_PAYLOAD_SIZE> msg(0x7D4E, payload);
        buffer_sysex(msg);
    }
};

// Memory calculation implementation
inline uint16_t DebugLogger::get_free_memory() {
    // For MSP430, we can estimate free memory by checking stack pointer
    // This is a simplified approach - actual implementation may vary
    uint8_t stack_var;
    // Estimate based on stack location (grows down from top of RAM)
    // MSP430G2553 has RAM from 0x0200 to 0x03FF (512 bytes)
    return ((uint16_t)&stack_var - 0x0200);
}

inline uint8_t DebugLogger::get_memory_usage_percent() {
    uint16_t free = get_free_memory();
    uint16_t total = 512; // MSP430G2553 RAM
    return ((total - free) * 100) / total;
}

}} // namespace nexus::debug

#endif // NEXUS_ENABLE_DEBUG_LOGGING

#endif // NEXUS_DEBUG_LOGGER_HPP