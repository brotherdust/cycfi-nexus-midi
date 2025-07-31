/**
 * @file program_change_controller.cpp
 * @brief MIDI program change controller implementation
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#include <Arduino.h>  // Must be first for Arduino functions
#include "controllers/program_change_controller.hpp"
#include "debug/debug_macros.hpp"

using namespace cycfi;

namespace nexus {
namespace controllers {

ProgramChangeController::ProgramChangeController()
    : curr{0}
    , base{0}
{}

void ProgramChangeController::load() {
    if (!storage::flash_b.empty())
        base = storage::flash_b.read();
}

void ProgramChangeController::save() {
    uint8_t base_ = max(min(base, 127), 0);
    if (base_ != storage::flash_b.read())
        storage::flash_b.write(base_);
}

uint8_t ProgramChangeController::get() {
    return uint8_t{max(min(curr + base, 127), 0)};
}

void ProgramChangeController::transmit() {
    uint8_t program = get();
    
    // Log the program change
    NEXUS_LOG_CONTROL(nexus::debug::CTRL_ID_PROGRAM, program);
    
    midi_out << midi::program_change{0, program};
#ifdef NEXUS_ENABLE_PC_CC_MAPPING
    send_cc_mapping();  // Send CC mapping after program change
#endif
}

#ifdef NEXUS_ENABLE_PC_CC_MAPPING
void ProgramChangeController::send_cc_mapping() {
    // Only map if curr is in valid range 0-4
    if (curr >= 0 && curr <= 4) {
        // Send all 5 CC messages to ensure proper one-hot encoding
        for (uint8_t i = 0; i < 5; ++i) {
            uint8_t cc_num = NEXUS_PC_CC_MAPPING_START + i;  // CC start to start+4
            uint8_t cc_val = (i == curr) ? 127 : 0;  // 127 for active, 0 for others
            
            // Log the CC mapping
            NEXUS_LOG_CONTROL(cc_num, cc_val);
            
            midi_out << midi::control_change{0, static_cast<midi::cc::controller>(cc_num), cc_val};
        }
    }
}
#endif // NEXUS_ENABLE_PC_CC_MAPPING

void ProgramChangeController::operator()(uint32_t val_) {
    uint32_t curr_ = curr * 205;
    int diff = curr_ - val_;
    if (diff < 0)
        diff = -diff;
    if (diff < 8)
        return;

    uint8_t val = (val_ * 5) / 1024;
    if (val != curr) {
        curr = val;
        transmit();
    }
}

void ProgramChangeController::up(bool sw) {
    if (btn_up(sw) && (base < 127)) {
        ++base;
        storage::reset_save_delay();
        transmit();
    }
}

void ProgramChangeController::down(bool sw) {
    if (btn_down(sw) && (base > 0)) {
        --base;
        storage::reset_save_delay();
        transmit();
    }
}

void ProgramChangeController::group_up(bool sw) {
    if (grp_btn_up(sw) && (base < 127)) {
        base += 5;
        storage::reset_save_delay();
        transmit();
    }
}

void ProgramChangeController::group_down(bool sw) {
    if (grp_btn_down(sw) && (base > 0)) {
        base -= 5;
        storage::reset_save_delay();
        transmit();
    }
}

} // namespace controllers
} // namespace nexus