/**
 * @file nexus_midi.ino
 * @brief Main implementation file for the Nexus MIDI controller
 *
 * This file contains the implementation of a MIDI controller for the MSP430
 * microcontroller. It handles various MIDI controls including program changes,
 * volume control, pitch bending, modulation, effects, and sustain.
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#include "midi.hpp"
#include "util.hpp"
#include "MspFlash.h"
#include "config/hardware_config.hpp"
#include "storage/flash_manager.hpp"
#include "storage/persistent_storage.hpp"
#include "controllers/controller_factory.hpp"
#include "test/test_runner.hpp"

/**
 * @section TEST_DEFINES
 *
 * Uncomment these defines to enable various test modes.
 * Each define enables a specific test functionality.
 */
//#define NEXUS_TEST
//#define NEXUS_TEST_NOTE
//#define NEXUS_TEST_VOLUME
//#define NEXUS_TEST_PITCH_BEND
//#define NEXUS_TEST_PROGRAM_CHANGE
//#define NEXUS_TEST_PROGRAM_CHANGE_UP_DOWN
//#define NEXUS_TEST_PROGRAM_CHANGE_GROUP_UP_DOWN
//#define NEXUS_TEST_EFFECTS_1
//#define NEXUS_TEST_EFFECTS_2
//#define NEXUS_TEST_MODULATION
//#define NEXUS_TEST_SUSTAIN
//#define NEXUS_TEST_BANK_SELECT
//#define NEXUS_DUMP_FLASH

using namespace cycfi;

/**
 * @section CONTROL_MAPPING
 *
 * Mapping of physical controls to MIDI functions.
 *
 * Main controls:
 *   - program_change: 5-way switch
 *   - channel_volume: analog
 *   - pitch_change: analog
 *   - modulation: analog
 *   - effect_1: analog
 *   - effect_2: analog
 *   - sustain: momentary switch
 *
 * Auxiliary controls:
 *   - program_change +5: momentary switch
 *   - program_change -5: momentary switch
 *   - program_change +1: momentary switch
 *   - program_change -1: momentary switch
 *   - bank_select +1: momentary switch
 *   - bank_select -1: momentary switch
 */

// Hardware configuration is now in config/hardware_config.hpp
using namespace nexus::config;

/**
 * @section MIDI_STREAM
 *
 * The main MIDI output stream for sending MIDI messages.
 */
midi::midi_stream midi_out;

// Flash storage is now in storage/flash_manager.hpp and storage/persistent_storage.hpp
using namespace nexus::storage;

// All controllers are now managed by the controller factory
// Test functionality is now in test/test_runner.hpp

/**
 * @section SETUP
 *
 * Setup function to initialize the MIDI controller.
 */
/**
 * @brief Initialize pins, MIDI output, and load saved state
 *
 * Sets up all input pins, starts the MIDI output stream,
 * and loads saved program change and bank select values from flash.
 */
void setup()
{
   // Initialize hardware pins
   nexus::config::initialize_pins();

   midi_out.start();

   // Initialize all controllers
   nexus::controllers::controllers.initialize();

   // Dump flash if requested
   nexus::test::test_runner.dump_flash();
}

/**
 * @section LOOP
 *
 * Main program loop implementation.
 */

#ifdef NEXUS_TEST

/**
 * @brief Main loop function for test mode
 *
 * Runs test functionality based on compile-time flags
 */
void loop()
{
   nexus::test::test_runner.run();
}

#else // !NEXUS_TEST

/**
 * @brief Main loop function
 *
 * Processes all inputs and generates MIDI messages.
 * Runs at a maximum rate of 1kHz.
 */
void loop()
{
   nexus::controllers::controllers.process_inputs();
}

#endif // NEXUS_TEST
