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
#include "controllers/base_controller.hpp"
#include "controllers/pitch_bend_controller.hpp"
#include "controllers/sustain_controller.hpp"
#include "controllers/program_change_controller.hpp"

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

/**
 * @section BANK_SELECT
 *
 * Bank select controller implementation.
 */
/**
 * @brief MIDI bank select controller
 *
 * Handles bank selection via buttons.
 * Persists bank select values to flash memory.
 */
struct bank_select_controller
{
   bank_select_controller()
      : curr{0}
   {}

   /**
    * @brief Load bank select value from flash memory
    */
   void load()
   {
      if (!flash_c.empty())
         curr = flash_c.read();
   }

   /**
    * @brief Save bank select value to flash memory
    *
    * Clamps the value to the valid MIDI range (0-127) before saving.
    */
   void save()
   {
      uint8_t curr_ = max(min(curr, 127), 0);
      if (curr_ != flash_c.read())
         flash_c.write(curr_);
   }

   /**
    * @brief Transmit the current bank select value as a MIDI message
    */
   void transmit()
   {
      midi_out << midi::control_change{0, midi::cc::bank_select, curr};
   }

   /**
    * @brief Increment bank select value by 1
    *
    * @param sw Switch state (true = pressed, false = released)
    */
   void up(bool sw)
   {
      if (btn_up(sw) && (curr < 127))
      {
         ++curr;
         reset_save_delay();
         transmit();
      }
   }

   /**
    * @brief Decrement bank select value by 1
    *
    * @param sw Switch state (true = pressed, false = released)
    */
   void down(bool sw)
   {
      if (btn_down(sw) && (curr > 0))
      {
         --curr;
         reset_save_delay();
         transmit();
      }
   }

   uint8_t curr;
   repeat_button<> btn_up;
   repeat_button<> btn_down;
};

/**
 * @section CONTROLS
 *
 * Instantiation of all controller objects.
 */
Controller<midi::cc::channel_volume>   volume_control;
Controller<midi::cc::effect_1>         fx1_control;
Controller<midi::cc::effect_2>         fx2_control;
Controller<midi::cc::modulation>       modulation_control;
PitchBendController                    pitch_bend;
ProgramChangeController                program_change;
SustainController                      sustain_control;
bank_select_controller                 bank_select_control;

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

   // Load the program_change and bank_select_control states from flash
   program_change.load();
   bank_select_control.load();

   // Transmit the current program_change and bank select state
   program_change.transmit();
   bank_select_control.transmit();

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
   volume_control(nexus::config::analog_read(ch10));
#endif

#ifdef NEXUS_TEST_PITCH_BEND
   pitch_bend(nexus::config::analog_read(ch10));
#endif

#ifdef NEXUS_TEST_PROGRAM_CHANGE
   program_change(nexus::config::analog_read(ch15));
#endif

#ifdef NEXUS_TEST_PROGRAM_CHANGE_UP_DOWN
   program_change.up(digitalRead(ch12));
   program_change.down(digitalRead(ch13));
#endif

#ifdef NEXUS_TEST_PROGRAM_CHANGE_GROUP_UP_DOWN
   program_change.group_up(digitalRead(ch12));
   program_change.group_down(digitalRead(ch13));
#endif

#ifdef NEXUS_TEST_EFFECTS_1
   fx1_control(nexus::config::analog_read(ch11));
#endif

#ifdef NEXUS_TEST_EFFECTS_2
   fx2_control(nexus::config::analog_read(ch11));
#endif

#ifdef NEXUS_TEST_MODULATION
   modulation_control(nexus::config::analog_read(ch11));
#endif

#ifdef NEXUS_TEST_SUSTAIN
   sustain_control(digitalRead(ch12));
#endif

#ifdef NEXUS_TEST_BANK_SELECT
   bank_select_control.up(digitalRead(aux1));
   bank_select_control.down(digitalRead(aux2));
#endif

   // Save the program_change and bank_select_control if needed
   if (should_save())
   {
      program_change.save();
      bank_select_control.save();
      mark_saved();
   }
}

#else // !NEXUS_TEST

uint32_t prev_time = 0;

/**
 * @brief Main loop function
 *
 * Processes all inputs and generates MIDI messages.
 * Runs at a maximum rate of 1kHz.
 */
void loop()
{
   // Limit processing rate to 1kHz maximum
   // TODO: Consider implementing timer interrupts for more precise timing
   if (prev_time != millis())
   {
      sustain_control(digitalRead(ch9));
      volume_control(nexus::config::analog_read(ch10));
      fx1_control(nexus::config::analog_read(ch11));
      fx2_control(nexus::config::analog_read(ch12));
      pitch_bend(nexus::config::analog_read(ch13));
      program_change(nexus::config::analog_read(ch14));
      modulation_control(nexus::config::analog_read(ch15));

      program_change.up(!digitalRead(aux1));
      program_change.down(!digitalRead(aux2));
      program_change.group_up(!digitalRead(aux3));
      program_change.group_down(!digitalRead(aux4));
      bank_select_control.up(!digitalRead(aux5));
      bank_select_control.down(!digitalRead(aux6));

      prev_time = millis();
   }

   // Save the program_change and bank_select_control if needed
   if (should_save())
   {
      program_change.save();
      bank_select_control.save();
      mark_saved();
   }
}

#endif // NEXUS_TEST
