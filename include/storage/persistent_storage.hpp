/**
 * @file persistent_storage.hpp
 * @brief Persistent storage utilities with save delay mechanism
 *
 * This file provides functionality for delayed saving to flash memory
 * to minimize write cycles and extend flash lifespan.
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_PERSISTENT_STORAGE_HPP
#define NEXUS_PERSISTENT_STORAGE_HPP

#include <stdint.h>

namespace nexus {
namespace storage {

/**
 * @section SAVE_DELAY_MECHANISM
 *
 * We lazily save data to flash to minimize writes to flash
 * and thus conserve erase cycles. To do this, we avoid eagerly saving
 * data when the user is actively changing states (e.g. buttons are pushed).
 * We delay the actual save N milliseconds after the last state change.
 */

// Save delay configuration - 3 seconds delay before saving to flash
extern const uint32_t save_delay;

// Save delay start time (-1 indicates no pending save)
extern int32_t save_delay_start_time;

/**
 * @brief Reset the save delay timer
 *
 * Called when a state change occurs to start the countdown
 * before saving to flash memory.
 */
void reset_save_delay();

/**
 * @brief Check if data should be saved
 *
 * @return true if the save delay has elapsed and data should be saved
 */
bool should_save();

/**
 * @brief Mark that data has been saved
 *
 * Resets the save delay timer to indicate no pending save
 */
void mark_saved();

} // namespace storage
} // namespace nexus

#endif // NEXUS_PERSISTENT_STORAGE_HPP