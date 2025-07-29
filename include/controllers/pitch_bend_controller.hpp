/**
 * @file pitch_bend_controller.hpp
 * @brief MIDI pitch bend controller
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_PITCH_BEND_CONTROLLER_HPP
#define NEXUS_PITCH_BEND_CONTROLLER_HPP

#include "controllers/base_controller.hpp"
#include "midi.hpp"
#include "util.hpp"
#include "config/hardware_config.hpp"

// Global MIDI output stream declaration
extern cycfi::midi::midi_stream midi_out;

namespace nexus {
namespace controllers {

/**
 * @brief MIDI pitch bend controller
 *
 * Processes analog input to generate MIDI pitch bend messages
 */
class PitchBendController : public IController {
public:
    /**
     * @brief Process analog input value and send MIDI pitch bend messages
     *
     * @param val_ Raw analog input value
     */
    void operator()(uint32_t val_) {
        uint32_t val = lp2(lp1(val_));
        if (gt(val)) {
            midi_out << cycfi::midi::pitch_bend{0, uint16_t{(val << 4) + (val % 16)}};
        }
    }

private:
    cycfi::lowpass<8, int32_t> lp1;
    cycfi::lowpass<16, int32_t> lp2;
    cycfi::gate<nexus::config::noise_window, int32_t> gt;
};

} // namespace controllers
} // namespace nexus

#endif // NEXUS_PITCH_BEND_CONTROLLER_HPP