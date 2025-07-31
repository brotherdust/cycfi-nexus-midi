/**
 * @file base_controller.hpp
 * @brief Base controller template for MIDI continuous controllers
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_BASE_CONTROLLER_HPP
#define NEXUS_BASE_CONTROLLER_HPP

#include "midi.hpp"
#include "util.hpp"
#include "config/hardware_config.hpp"
#include "debug/debug_macros.hpp"
#include <stdint.h>

// Global MIDI output stream declaration
extern cycfi::midi::midi_stream midi_out;

namespace nexus {
namespace controllers {

/**
 * @brief Generic MIDI controller template
 *
 * Handles continuous controller messages with MSB/LSB support
 * Applies lowpass filtering and noise gating
 *
 * @tparam ctrl The MIDI controller type from cycfi::midi::cc::controller
 */
template <cycfi::midi::cc::controller ctrl>
class Controller {
public:
    static cycfi::midi::cc::controller const ctrl_lsb =
        cycfi::midi::cc::controller(ctrl | 0x20);

    /**
     * @brief Process analog input value and send MIDI control change messages
     *
     * @param val_ Raw analog input value
     */
    void operator()(uint32_t val_) {
        uint32_t val = lp2(lp1(val_));
        if (gt(val)) {
            uint8_t const msb = val >> 3;
            uint8_t const lsb = (val << 4) & 0x7F;
            
            // Log the control change
            NEXUS_LOG_CONTROL(ctrl, msb);
            
            // Use global midi_out
            midi_out << cycfi::midi::control_change{0, ctrl_lsb, lsb};
            midi_out << cycfi::midi::control_change{0, ctrl, msb};
        }
    }

private:
    cycfi::lowpass<8, int32_t> lp1;
    cycfi::lowpass<16, int32_t> lp2;
    cycfi::gate<nexus::config::noise_window, int32_t> gt;
};

/**
 * @brief Base interface for non-templated controllers
 * Note: No virtual destructor to avoid vtable overhead
 */
class IController {
public:
    // No virtual methods to minimize memory overhead
};

} // namespace controllers
} // namespace nexus

#endif // NEXUS_BASE_CONTROLLER_HPP