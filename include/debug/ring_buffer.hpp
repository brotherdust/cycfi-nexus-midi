/**
 * @file ring_buffer.hpp
 * @brief Lock-free ring buffer implementation for debug logging
 *
 * This file contains a template-based ring buffer implementation designed
 * for non-blocking debug message queuing. The buffer is interrupt-safe and
 * ensures that debug logging never interferes with real-time MIDI performance.
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_RING_BUFFER_HPP
#define NEXUS_RING_BUFFER_HPP

#include <stdint.h>

namespace nexus { namespace debug {

/**
 * @brief Lock-free ring buffer for debug message queuing
 *
 * This ring buffer implementation is designed for single-producer,
 * single-consumer scenarios typical in embedded systems. It uses
 * power-of-2 sizing for efficient modulo operations.
 *
 * @tparam SIZE Buffer size in bytes (must be power of 2, minimum 32)
 */
template<uint8_t SIZE>
class RingBuffer {
private:
    // Compile-time validation
    // Note: C++98 doesn't have static_assert, so we use template trick
    template<bool> struct CompileTimeError;
    template<> struct CompileTimeError<true> {};
    
    // Validate SIZE is power of 2
    enum { 
        IsPowerOfTwo = (SIZE & (SIZE - 1)) == 0,
        IsMinimumSize = SIZE >= 32
    };
    
    uint8_t buffer[SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile bool overflow;
    
public:
    /**
     * @brief Constructor
     */
    RingBuffer() : head(0), tail(0), overflow(false) {
        // Force compile error if SIZE is invalid
        CompileTimeError<IsPowerOfTwo>();
        CompileTimeError<IsMinimumSize>();
    }
    
    /**
     * @brief Add a byte to the buffer
     *
     * @param data Byte to add
     * @return true if successful, false if buffer full
     */
    bool put(uint8_t data) {
        uint8_t next = (head + 1) & (SIZE - 1);
        if (next == tail) {
            overflow = true;
            return false; // Buffer full
        }
        buffer[head] = data;
        head = next;
        return true;
    }
    
    /**
     * @brief Get a byte from the buffer
     *
     * @param data Reference to store the retrieved byte
     * @return true if successful, false if buffer empty
     */
    bool get(uint8_t& data) {
        if (head == tail) return false; // Buffer empty
        data = buffer[tail];
        tail = (tail + 1) & (SIZE - 1);
        return true;
    }
    
    /**
     * @brief Get number of bytes available in buffer
     *
     * @return Number of bytes available to read
     */
    uint8_t available() const {
        return (SIZE + head - tail) & (SIZE - 1);
    }
    
    /**
     * @brief Check if buffer is empty
     *
     * @return true if buffer is empty
     */
    bool empty() const {
        return head == tail;
    }
    
    /**
     * @brief Check and clear overflow flag
     *
     * @return true if overflow occurred since last check
     */
    bool had_overflow() {
        bool result = overflow;
        overflow = false;
        return result;
    }
    
    /**
     * @brief Clear all data in buffer
     */
    void clear() {
        tail = head;
        overflow = false;
    }
};

}} // namespace nexus::debug

#endif // NEXUS_RING_BUFFER_HPP