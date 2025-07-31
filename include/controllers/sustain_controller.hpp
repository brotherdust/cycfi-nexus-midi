/**
 * @file sustain_controller.hpp
 * @brief MIDI sustain pedal controller
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_SUSTAIN_CONTROLLER_HPP
#define NEXUS_SUSTAIN_CONTROLLER_HPP

#include "controllers/base_controller.hpp"
#include "midi.hpp"
#include "util.hpp"
#include "debug/debug_macros.hpp"

// Global MIDI output stream declaration
extern cycfi::midi::midi_stream midi_out;

namespace nexus {
namespace controllers {

/**
 * @brief MIDI sustain pedal controller
 *
 * Processes switch input to generate MIDI sustain pedal messages
 */
class SustainController : public IController {
public:
    /**
     * @brief Process switch state to generate MIDI sustain pedal messages
     *
     * @param sw Switch state (true = pressed, false = released)
     */
    void operator()(bool sw) {
        int state = edge(sw);
        if (state == 1) {
            // Log sustain off
            NEXUS_LOG_CONTROL(nexus::debug::CTRL_ID_SUSTAIN, 0);
            midi_out << cycfi::midi::control_change{0, cycfi::midi::cc::sustain, 0};
        } else if (state == -1) {
            // Log sustain on
            NEXUS_LOG_CONTROL(nexus::debug::CTRL_ID_SUSTAIN, 127);
            midi_out << cycfi::midi::control_change{0, cycfi::midi::cc::sustain, 127};
        }
    }

private:
    cycfi::edge_detector<> edge;
};

} // namespace controllers
} // namespace nexus

#endif // NEXUS_SUSTAIN_CONTROLLER_HPP