/**
 * @file controller_factory.cpp
 * @brief Controller factory implementation
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#include <Arduino.h>  // Must be first for Arduino functions
#include "controllers/controller_factory.hpp"
#include "config/hardware_config.hpp"
#include "storage/persistent_storage.hpp"

namespace nexus {
namespace controllers {

// Global controller factory instance
ControllerFactory controllers;

ControllerFactory::ControllerFactory()
    : prev_time(0)
{
}

void ControllerFactory::initialize() {
    // Load saved states from flash
    program_change.load();
    bank_select.load();
    
    // Transmit initial states
    program_change.transmit();
    bank_select.transmit();
}

void ControllerFactory::save_states() {
    if (storage::should_save()) {
        program_change.save();
        bank_select.save();
        storage::mark_saved();
    }
}

void ControllerFactory::process_inputs(bool test_mode) {
    if (!test_mode) {
        // Limit processing rate to 1kHz maximum
        if (prev_time != millis()) {
            // Process analog inputs
            sustain(digitalRead(config::ch9));
            volume(config::analog_read(config::ch10));
            fx1(config::analog_read(config::ch11));
            fx2(config::analog_read(config::ch12));
            pitch_bend(config::analog_read(config::ch13));
            program_change(config::analog_read(config::ch14));
            modulation(config::analog_read(config::ch15));
            
            // Process digital inputs (active low)
            program_change.up(!digitalRead(config::aux1));
            program_change.down(!digitalRead(config::aux2));
            program_change.group_up(!digitalRead(config::aux3));
            program_change.group_down(!digitalRead(config::aux4));
            bank_select.up(!digitalRead(config::aux5));
            bank_select.down(!digitalRead(config::aux6));
            
            prev_time = millis();
        }
    }
    
    // Always check for save regardless of test mode
    save_states();
}

} // namespace controllers
} // namespace nexus