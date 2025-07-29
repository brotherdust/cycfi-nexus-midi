/**
 * @file controller_factory.hpp
 * @brief Factory for creating and managing MIDI controller instances
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_CONTROLLER_FACTORY_HPP
#define NEXUS_CONTROLLER_FACTORY_HPP

#include "controllers/base_controller.hpp"
#include "controllers/pitch_bend_controller.hpp"
#include "controllers/sustain_controller.hpp"
#include "controllers/program_change_controller.hpp"
#include "controllers/bank_select_controller.hpp"
#include "midi.hpp"

namespace nexus {
namespace controllers {

/**
 * @brief Factory class for managing all MIDI controller instances
 *
 * Provides centralized access to all controller instances
 * and initialization methods.
 */
class ControllerFactory {
public:
    /**
     * @brief Constructor - initializes member variables
     */
    ControllerFactory();
    
    // Controller instances
    Controller<cycfi::midi::cc::channel_volume> volume;
    Controller<cycfi::midi::cc::effect_1> fx1;
    Controller<cycfi::midi::cc::effect_2> fx2;
    Controller<cycfi::midi::cc::modulation> modulation;
    PitchBendController pitch_bend;
    ProgramChangeController program_change;
    SustainController sustain;
    BankSelectController bank_select;
    
    /**
     * @brief Initialize all controllers
     *
     * Loads saved states from flash memory and transmits initial values
     */
    void initialize();
    
    /**
     * @brief Save controller states to flash memory
     *
     * Saves program change and bank select states if needed
     */
    void save_states();
    
    /**
     * @brief Process all controller inputs
     *
     * This method handles the main loop processing for all controllers
     * @param test_mode If true, runs in test mode with limited functionality
     */
    void process_inputs(bool test_mode = false);
    
private:
    uint32_t prev_time;
};

// Global controller factory instance
extern ControllerFactory controllers;

} // namespace controllers
} // namespace nexus

#endif // NEXUS_CONTROLLER_FACTORY_HPP