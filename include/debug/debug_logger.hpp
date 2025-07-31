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
 * via MIDI SysEx messages. All messages are queued through a ring buffer
 * to ensure non-blocking operation.
 */
class DebugLogger {
private:
    // Ring buffer is required for non-blocking operation
    static RingBuffer<NEXUS_DEBUG_BUFFER_SIZE> buffer;
    static uint32_t last_timestamp;
    
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
     * @brief Send byte through ring buffer (required for non-blocking)
     *
     * @param byte Byte to send (will be masked to 7-bit)
     */
    static void send_byte(uint8_t byte) {
        buffer.put(byte & 0x7F);  // Ensure 7-bit clean
    }
    
    /**
     * @brief Send complete SysEx message header
     *
     * @param type Message type
     */
    static void send_header(MessageType type) {
        uint16_t ts = get_timestamp();
        
        send_byte(0xF0);              // SysEx start
        send_byte(0x7D);              // Non-commercial ID
        send_byte(0x4E);              // 'N'
        send_byte(0x45);              // 'E'
        send_byte(0x58);              // 'X'
        send_byte(type);              // Message type
        send_byte((ts >> 7) & 0x7F);  // Timestamp MSB
        send_byte(ts & 0x7F);         // Timestamp LSB
    }
    
    /**
     * @brief Send SysEx end byte
     */
    static void send_footer() {
        send_byte(0xF7);
    }
    
    // Memory calculation helpers
    static uint16_t get_free_memory();
    static uint8_t get_memory_usage_percent();
    
public:
    /**
     * @brief Initialize the logger
     */
    static void init() {
        last_timestamp = millis();
        buffer.clear();
    }
    
    /**
     * @brief Flush buffered messages to MIDI output
     *
     * Should be called regularly from main loop. Limits bytes per
     * flush to prevent blocking.
     */
    static void flush() {
        uint8_t byte;
        // Limit bytes per flush to prevent blocking
        uint8_t max_bytes = 16;  // Adjust based on timing requirements
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
        send_header(LOG_SYSTEM);
        send_byte(event);
        send_byte(param1);
        send_byte(param2);
        send_footer();
    }
    
    /**
     * @brief Log controller change
     *
     * @param controller_id Controller ID from ControllerId enum
     * @param value New controller value (0-127)
     */
    static void log_control(uint8_t controller_id, uint8_t value) {
        send_header(LOG_CONTROL);
        send_byte(CTRL_CHANGE);
        send_byte(controller_id);
        send_byte(value & 0x7F);
        send_footer();
    }
    
    /**
     * @brief Log memory usage
     */
    static void log_memory() {
        uint16_t free_mem = get_free_memory();
        uint8_t usage_percent = get_memory_usage_percent();
        
        send_header(LOG_MEMORY);
        send_byte(usage_percent);
        send_byte((free_mem >> 7) & 0x7F);
        send_byte(free_mem & 0x7F);
        send_footer();
    }
    
    /**
     * @brief Log error
     *
     * @param code Error code from ErrorCode enum
     * @param context Optional context value
     */
    static void log_error(ErrorCode code, uint8_t context = 0) {
        send_header(LOG_ERROR);
        send_byte(code);
        send_byte(context);
        send_footer();
    }
    
    /**
     * @brief Log debug string (first 4 chars only due to memory constraints)
     *
     * @param msg Debug message (only first 4 chars will be sent)
     * @param value Optional numeric value
     */
    static void log_debug(const char* msg, uint8_t value = 0) {
        send_header(LOG_DEBUG);
        
        // Send up to 4 characters, ensure 7-bit ASCII
        for (uint8_t i = 0; i < 4; i++) {
            if (msg[i]) {
                send_byte(msg[i] & 0x7F);
            } else {
                send_byte(0x20);  // Space padding
            }
        }
        
        send_byte(value);
        send_footer();
    }
    
    /**
     * @brief Log configuration change
     *
     * @param event Configuration event
     * @param param1 Parameter 1
     * @param param2 Parameter 2
     */
    static void log_config(uint8_t event, uint8_t param1 = 0, uint8_t param2 = 0) {
        send_header(LOG_CONFIG);
        send_byte(event);
        send_byte(param1);
        send_byte(param2);
        send_footer();
    }
    
    /**
     * @brief Log storage operation
     *
     * @param event Storage event from StorageEvent enum
     * @param address Memory address (14-bit)
     * @param value Value read/written
     */
    static void log_storage(uint8_t event, uint16_t address, uint8_t value) {
        send_header(LOG_STORAGE);
        send_byte(event);
        send_byte((address >> 7) & 0x7F);
        send_byte(address & 0x7F);
        send_byte(value);
        send_footer();
    }
    
    /**
     * @brief Log performance metric
     *
     * @param metric Metric type from PerfMetric enum
     * @param value Metric value
     */
    static void log_perf(uint8_t metric, uint8_t value) {
        send_header(LOG_PERF);
        send_byte(metric);
        send_byte(value);
        send_footer();
    }
};

// Static member definitions
RingBuffer<NEXUS_DEBUG_BUFFER_SIZE> DebugLogger::buffer;
uint32_t DebugLogger::last_timestamp = 0;

// Memory calculation implementation
inline uint16_t DebugLogger::get_free_memory() {
    extern uint8_t __heap_start;
    extern uint8_t* __brkval;
    uint8_t* heap_end = __brkval ? __brkval : &__heap_start;
    uint8_t stack_top;
    return &stack_top - heap_end;
}

inline uint8_t DebugLogger::get_memory_usage_percent() {
    uint16_t free = get_free_memory();
    uint16_t total = 512; // MSP430G2553 RAM
    return ((total - free) * 100) / total;
}

}} // namespace nexus::debug

#endif // NEXUS_ENABLE_DEBUG_LOGGING

#endif // NEXUS_DEBUG_LOGGER_HPP