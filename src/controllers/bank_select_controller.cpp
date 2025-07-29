/**
 * @file bank_select_controller.cpp
 * @brief MIDI bank select controller implementation
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#include <Arduino.h>  // Must be first for Arduino functions
#include "controllers/bank_select_controller.hpp"

using namespace cycfi;

namespace nexus {
namespace controllers {

BankSelectController::BankSelectController()
    : curr{0}
{}

void BankSelectController::load() {
    if (!storage::flash_c.empty())
        curr = storage::flash_c.read();
}

void BankSelectController::save() {
    uint8_t curr_ = max(min(curr, 127), 0);
    if (curr_ != storage::flash_c.read())
        storage::flash_c.write(curr_);
}

void BankSelectController::transmit() {
    midi_out << midi::control_change{0, midi::cc::bank_select, curr};
}

void BankSelectController::up(bool sw) {
    if (btn_up(sw) && (curr < 127)) {
        ++curr;
        storage::reset_save_delay();
        transmit();
    }
}

void BankSelectController::down(bool sw) {
    if (btn_down(sw) && (curr > 0)) {
        --curr;
        storage::reset_save_delay();
        transmit();
    }
}

} // namespace controllers
} // namespace nexus