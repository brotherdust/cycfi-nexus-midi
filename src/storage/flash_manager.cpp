/**
 * @file flash_manager.cpp
 * @brief Flash memory management implementation
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#include "storage/flash_manager.hpp"
#include "MspFlash.h"

namespace nexus {
namespace storage {

// Define the global flash instances
// SEGMENT_B and SEGMENT_C are defined in MspFlash.h
FlashManager flash_b(SEGMENT_B);
FlashManager flash_c(SEGMENT_C);

FlashManager::FlashManager(byte* segment)
    : _segment(segment) {
}

void FlashManager::erase() {
    Flash.erase(_segment);
}

bool FlashManager::empty() const {
    return (*_segment == 0xff);
}

byte FlashManager::read() const {
    if (empty())
        return 0xff;
    
    if (byte* p = find_free())
        return *(p - 1);
    
    return _segment[63];
}

void FlashManager::write(byte val) {
    byte* p = find_free();
    if (p == nullptr) {
        erase();
        Flash.write(_segment, &val, 1);
    } else {
        Flash.write(p, &val, 1);
    }
}

byte* FlashManager::find_free() const {
    for (int i = 0; i != 64; ++i) {
        if (_segment[i] == 0xff)
            return &_segment[i];
    }
    return nullptr;
}

} // namespace storage
} // namespace nexus