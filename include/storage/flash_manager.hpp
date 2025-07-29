/**
 * @file flash_manager.hpp
 * @brief Flash memory management for persistent storage
 *
 * This file provides flash memory abstraction for storing MIDI 7-bit data
 * in a ring buffer fashion to minimize erase cycles and maximize flash lifespan.
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_FLASH_MANAGER_HPP
#define NEXUS_FLASH_MANAGER_HPP

#include <stdint.h>

namespace nexus {
namespace storage {

/**
 * @brief Flash memory management class
 *
 * Provides functionality to read, write, and erase flash memory segments
 * in a way that maximizes the lifespan of the flash memory.
 * 
 * The MSP430 has SEGMENT_B to SEGMENT_D available for applications to use,
 * where each segment has 64 bytes. When erased, data in the flash is
 * read as 0xff. You can only write once to a data slot, per segment,
 * per erase cycle.
 * 
 * MSP430 is spec'd to allow a minimum guaranteed 10,000 erase cycles
 * (100,000 erase cycles typical). We store 7-bits data into flash memory
 * in a ring buffer fashion to minimize erase cycles.
 */
class FlashManager {
public:
    typedef unsigned char byte;
    
    /**
     * @brief Constructor
     * @param segment Pointer to the flash memory segment
     */
    explicit FlashManager(byte* segment);
    
    /**
     * @brief Erase the flash memory segment
     */
    void erase();
    
    /**
     * @brief Check if the flash memory segment is empty
     * @return true if empty, false otherwise
     */
    bool empty() const;
    
    /**
     * @brief Read a byte from the flash memory
     * @return The byte value or 0xff if empty
     */
    byte read() const;
    
    /**
     * @brief Write a byte to the flash memory
     * @param val The byte value to write
     */
    void write(byte val);
    
private:
    /**
     * @brief Find a free byte in the flash memory segment
     * @return Pointer to the free byte or nullptr if none found
     */
    byte* find_free() const;
    
    byte* _segment; ///< Pointer to the flash memory segment
};

/**
 * @section GLOBAL_FLASH_INSTANCES
 *
 * Global flash instances for program change and bank select data
 * We use SEGMENT_B and SEGMENT_C to store this data
 */
extern FlashManager flash_b;
extern FlashManager flash_c;

} // namespace storage
} // namespace nexus

#endif // NEXUS_FLASH_MANAGER_HPP