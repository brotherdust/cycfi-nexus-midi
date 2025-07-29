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

/**
 * @section TEST_NOTE
 *
 * Play notes functionality for testing purposes only.
 * Only compiled when NEXUS_TEST is defined.
 */
#ifdef NEXUS_TEST
/**
 * @brief Test structure for playing MIDI notes
 *
 * Handles note on/off events based on switch state.
 */
struct note
{
   /**
    * @brief Process switch state to generate MIDI note events
    *
    * @param sw Switch state (true = pressed, false = released)
    */
   void operator()(bool sw)
   {
      int state = edge(sw);
      if (state == 1)
         midi_out << midi::note_on{0, 80, 127};
      else if (state == -1)
         midi_out << midi::note_off{0, 80, 127};
   }

   edge_detector<> edge;
};

note _note;
#endif

// Controller template is now in controllers/base_controller.hpp
using namespace nexus::controllers;

// Pitch bend controller is now in controllers/pitch_bend_controller.hpp

// Program change controller is now in controllers/program_change_controller.hpp
// Sustain controller is now in controllers/sustain_controller.hpp
// Bank select controller is now in controllers/bank_select_controller.hpp

// All controllers are now managed by the controller factory

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

#ifdef NEXUS_DUMP_FLASH
   unsigned char* seg_b = SEGMENT_B;
   midi_out << midi::sysex<16> {0x5555, seg_b};
   midi_out << midi::sysex<16> {0x5555, seg_b + 16};
   midi_out << midi::sysex<16> {0x5555, seg_b + 32};
   midi_out << midi::sysex<16> {0x5555, seg_b + 48};

   unsigned char* seg_c = SEGMENT_C;
   midi_out << midi::sysex<16> {0x5555, seg_c};
   midi_out << midi::sysex<16> {0x5555, seg_c + 16};
   midi_out << midi::sysex<16> {0x5555, seg_c + 32};
   midi_out << midi::sysex<16> {0x5555, seg_c + 48};
#endif
}

/**
 * @section LOOP
 *
 * Main program loop implementation.
 */

#ifdef NEXUS_TEST
/**
 * @brief Main loop function
 *
 * Processes all inputs and generates MIDI messages.
 * Runs at a maximum rate of 1kHz.
 */
void loop()
{
#ifdef NEXUS_TEST_NOTE
   _note(digitalRead(aux1));
#endif

#ifdef NEXUS_TEST_VOLUME
   nexus::controllers::controllers.volume(nexus::config::analog_read(ch10));
#endif

#ifdef NEXUS_TEST_PITCH_BEND
   nexus::controllers::controllers.pitch_bend(nexus::config::analog_read(ch10));
#endif

#ifdef NEXUS_TEST_PROGRAM_CHANGE
   nexus::controllers::controllers.program_change(nexus::config::analog_read(ch15));
#endif

#ifdef NEXUS_TEST_PROGRAM_CHANGE_UP_DOWN
   nexus::controllers::controllers.program_change.up(digitalRead(ch12));
   nexus::controllers::controllers.program_change.down(digitalRead(ch13));
#endif

#ifdef NEXUS_TEST_PROGRAM_CHANGE_GROUP_UP_DOWN
   nexus::controllers::controllers.program_change.group_up(digitalRead(ch12));
   nexus::controllers::controllers.program_change.group_down(digitalRead(ch13));
#endif

#ifdef NEXUS_TEST_EFFECTS_1
   nexus::controllers::controllers.fx1(nexus::config::analog_read(ch11));
#endif

#ifdef NEXUS_TEST_EFFECTS_2
   nexus::controllers::controllers.fx2(nexus::config::analog_read(ch11));
#endif

#ifdef NEXUS_TEST_MODULATION
   nexus::controllers::controllers.modulation(nexus::config::analog_read(ch11));
#endif

#ifdef NEXUS_TEST_SUSTAIN
   nexus::controllers::controllers.sustain(digitalRead(ch12));
#endif

#ifdef NEXUS_TEST_BANK_SELECT
   nexus::controllers::controllers.bank_select.up(digitalRead(aux1));
   nexus::controllers::controllers.bank_select.down(digitalRead(aux2));
#endif

   // Save controller states if needed
   nexus::controllers::controllers.save_states();
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
