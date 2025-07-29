/**
 * @file persistent_storage.cpp
 * @brief Persistent storage utilities implementation
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#include "storage/persistent_storage.hpp"
#include <Arduino.h>

namespace nexus {
namespace storage {

// Save delay configuration
const uint32_t save_delay = 3000;  // 3 seconds delay before saving to flash

// Save delay start time
int32_t save_delay_start_time = -1;

void reset_save_delay() {
    save_delay_start_time = millis();
}

bool should_save() {
    return (save_delay_start_time != -1) && 
           (millis() > (save_delay_start_time + save_delay));
}

void mark_saved() {
    save_delay_start_time = -1;
}

} // namespace storage
} // namespace nexus