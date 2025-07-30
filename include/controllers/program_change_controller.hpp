/**
 * @file program_change_controller.hpp
 * @brief MIDI program change controller
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_PROGRAM_CHANGE_CONTROLLER_HPP
#define NEXUS_PROGRAM_CHANGE_CONTROLLER_HPP

#include "controllers/base_controller.hpp"
#include "midi.hpp"
#include "util.hpp"
#include "storage/flash_manager.hpp"
#include "storage/persistent_storage.hpp"
#include "config/feature_config.hpp"
#include <stdint.h>

// Global MIDI output stream declaration
extern cycfi::midi::midi_stream midi_out;

namespace nexus {
namespace controllers {

/**
 * @brief MIDI program change controller
 *
 * Handles program change selection via analog input or buttons.
 * Supports individual and group increment/decrement.
 * Persists program change values to flash memory.
 */
class ProgramChangeController : public IController {
public:
    ProgramChangeController();
    
    /**
     * @brief Load program change base value from flash memory
     */
    void load();
    
    /**
     * @brief Save program change base value to flash memory
     *
     * Clamps the value to the valid MIDI range (0-127) before saving.
     */
    void save();
    
    /**
     * @brief Get the current program change value
     *
     * @return Combined and clamped program change value
     */
    uint8_t get();
    
    /**
     * @brief Transmit the current program change value as a MIDI message
     */
    void transmit();
    
    /**
     * @brief Process analog input for program change selection
     *
     * Implements hysteresis to prevent jitter and maps the analog
     * value to the appropriate program change range.
     *
     * @param val_ Raw analog input value
     */
    void operator()(uint32_t val_);
    
    /**
     * @brief Increment program change base value by 1
     *
     * @param sw Switch state (true = pressed, false = released)
     */
    void up(bool sw);
    
    /**
     * @brief Decrement program change base value by 1
     *
     * @param sw Switch state (true = pressed, false = released)
     */
    void down(bool sw);
    
    /**
     * @brief Increment program change base value by 5
     *
     * @param sw Switch state (true = pressed, false = released)
     */
    void group_up(bool sw);
    
    /**
     * @brief Decrement program change base value by 5
     *
     * @param sw Switch state (true = pressed, false = released)
     */
    void group_down(bool sw);

private:
    int16_t curr;
    int16_t base;
    cycfi::repeat_button<> btn_up;
    cycfi::repeat_button<> btn_down;
    cycfi::repeat_button<> grp_btn_up;
    cycfi::repeat_button<> grp_btn_down;
    
#ifdef NEXUS_ENABLE_PC_CC_MAPPING
    /**
     * @brief Send CC messages 102-106 in one-hot encoding based on curr value
     *
     * Maps the 5-way switch position (0-4) to CC messages:
     * - Position 0: CC 102 = 127, CC 103-106 = 0
     * - Position 1: CC 103 = 127, CC 102,104-106 = 0
     * - Position 2: CC 104 = 127, CC 102-103,105-106 = 0
     * - Position 3: CC 105 = 127, CC 102-104,106 = 0
     * - Position 4: CC 106 = 127, CC 102-105 = 0
     *
     * This method is only compiled when NEXUS_ENABLE_PC_CC_MAPPING is defined.
     */
    void send_cc_mapping();
#endif // NEXUS_ENABLE_PC_CC_MAPPING
};

} // namespace controllers
} // namespace nexus

#endif // NEXUS_PROGRAM_CHANGE_CONTROLLER_HPP