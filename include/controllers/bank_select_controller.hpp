/**
 * @file bank_select_controller.hpp
 * @brief MIDI bank select controller
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_BANK_SELECT_CONTROLLER_HPP
#define NEXUS_BANK_SELECT_CONTROLLER_HPP

#include "controllers/base_controller.hpp"
#include "midi.hpp"
#include "util.hpp"
#include "storage/flash_manager.hpp"
#include "storage/persistent_storage.hpp"
#include <stdint.h>

// Global MIDI output stream declaration
extern cycfi::midi::midi_stream midi_out;

namespace nexus {
namespace controllers {

/**
 * @brief MIDI bank select controller
 *
 * Handles bank selection via buttons.
 * Persists bank select values to flash memory.
 */
class BankSelectController : public IController {
public:
    BankSelectController();
    
    /**
     * @brief Load bank select value from flash memory
     */
    void load();
    
    /**
     * @brief Save bank select value to flash memory
     *
     * Clamps the value to the valid MIDI range (0-127) before saving.
     */
    void save();
    
    /**
     * @brief Transmit the current bank select value as a MIDI message
     */
    void transmit();
    
    /**
     * @brief Increment bank select value by 1
     *
     * @param sw Switch state (true = pressed, false = released)
     */
    void up(bool sw);
    
    /**
     * @brief Decrement bank select value by 1
     *
     * @param sw Switch state (true = pressed, false = released)
     */
    void down(bool sw);

private:
    uint8_t curr;
    cycfi::repeat_button<> btn_up;
    cycfi::repeat_button<> btn_down;
};

} // namespace controllers
} // namespace nexus

#endif // NEXUS_BANK_SELECT_CONTROLLER_HPP