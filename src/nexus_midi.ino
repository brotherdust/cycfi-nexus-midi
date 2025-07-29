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

/**
 * @section CONSTANTS
 *
 * Pin definitions and other constants used throughout the program.
 */
int const ch9  = P2_0; // Digital input
int const ch10 = P1_0; // Analog and digital input
int const ch11 = P1_3; // Analog and digital input
int const ch12 = P1_4; // Analog and digital input
int const ch13 = P1_5; // Analog and digital input
int const ch14 = P1_6; // Analog and digital input
int const ch15 = P1_7; // Analog and digital input

int const aux1 = P2_1; // Digital input
int const aux2 = P2_2; // Digital input
int const aux3 = P2_3; // Digital input
int const aux4 = P2_4; // Digital input
int const aux5 = P2_5; // Digital input
int const aux6 = P2_6; // Digital input

#ifdef NEXUS_TEST
int const noise_window = 4;
#else
int const noise_window = 2;
#endif

/**
 * @section MIDI_STREAM
 *
 * The main MIDI output stream for sending MIDI messages.
 */
midi::midi_stream midi_out;

/**
 * @section FLASH_UTILITY
 *
 * Flash utility for persisting MIDI 7-bit data.
 *
 * The MSP430 has SEGMENT_B to SEGMENT_D available for applications to use,
 * where each segment has 64 bytes. When erased, data in the flash is
 * read as 0xff. You can only write once to a data slot, per segment,
 * per erase cycle (actually you can write more than once but once a bit
 * is reset, you cannot set it with a subsequent write).
 *
 * MSP430 is spec'd to allow a minimum guaranteed 10,000 erase cycles
 * (100,000 erase cycles typical). We store 7-bits data into flash memory
 * in a ring buffer fashion to minimize erase cycles and increase the
 * possible write cycles. See link below:
 *
 * http://processors.wiki.ti.com/index.php/Emulating_EEPROM_in_MSP430_Flash
 */
/**
 * @brief Flash memory management structure
 *
 * Provides functionality to read, write, and erase flash memory segments
 * in a way that maximizes the lifespan of the flash memory.
 */
struct flash
{
   typedef unsigned char byte;

   /**
    * @brief Constructor
    * @param segment_ Pointer to the flash memory segment
    */
   flash(byte* segment_)
      : _segment(segment_)
   {}

   /**
    * @brief Erase the flash memory segment
    */
   void erase()
   {
      Flash.erase(_segment);
   }

   /**
    * @brief Check if the flash memory segment is empty
    * @return true if empty, false otherwise
    */
   bool empty() const
   {
      return (*_segment == 0xff);
   }

   /**
    * @brief Read a byte from the flash memory
    * @return The byte value or 0xff if empty
    */
   byte read() const
   {
      if (empty())
         return 0xff;
      if (byte* p = find_free())
         return *(p - 1);
      return _segment[63];
   }

   /**
    * @brief Write a byte to the flash memory
    * @param val The byte value to write
    */
   void write(byte val)
   {
      byte* p = find_free();
      if (p == 0)
      {
         erase();
         Flash.write(_segment, &val, 1);
      }
      else
      {
         Flash.write(p, &val, 1);
      }
   }

   private:

   /**
    * @brief Find a free byte in the flash memory segment
    * @return Pointer to the free byte or 0 if none found
    */
   byte* find_free() const
   {
      for (int i = 0; i != 64; ++i)
         if (_segment[i] == 0xff)
            return &_segment[i];
      return 0;
   }

   byte* _segment; // Pointer to the flash memory segment
};

/**
 * @brief Flash memory instances for program change and bank select data
 *
 * We use SEGMENT_B and SEGMENT_C to store program change and bank select data
 */
flash flash_b(SEGMENT_B);
flash flash_c(SEGMENT_C);

/**
 * @brief Save delay mechanism for flash memory
 *
 * We lazily save data to flash to minimize writes to flash
 * and thus conserve erase cycles. To do this, we avoid eagerly saving
 * data when the user is actively changing states (e.g. buttons are pushed).
 * We delay the actual save N milliseconds after the last state change.
 */

uint32_t const save_delay = 3000;  // 3 seconds delay before saving to flash
int32_t save_delay_start_time = -1;

/**
 * @brief Reset the save delay timer
 *
 * Called when a state change occurs to start the countdown
 * before saving to flash memory.
 */
void reset_save_delay()
{
   save_delay_start_time = millis();
}

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

/**
 * @section CONTROLLER
 *
 * Generic controller handling with coarse and fine controls.
 * Implements MIDI continuous controller functionality.
 */
/**
 * @brief Generic MIDI controller implementation
 *
 * @tparam ctrl The MIDI controller type from midi::cc::controller
 */
template <midi::cc::controller ctrl>
struct controller
{
   static midi::cc::controller const ctrl_lsb = midi::cc::controller(ctrl | 0x20);

   /**
    * @brief Process analog input value and send MIDI control change messages
    *
    * Applies lowpass filtering and noise gating before sending MIDI messages.
    * Sends both MSB and LSB control change messages for high resolution.
    *
    * @param val_ Raw analog input value
    */
   void operator()(uint32_t val_)
   {
      uint32_t val = lp2(lp1(val_));
      if (gt(val))
      {
         uint8_t const msb = val >> 3;
         uint8_t const lsb = (val << 4) & 0x7F;
         midi_out << midi::control_change{0, ctrl_lsb, lsb};
         midi_out << midi::control_change{0, ctrl, msb};
      }
   }

   lowpass<8, int32_t> lp1;
   lowpass<16, int32_t> lp2;
   gate<noise_window, int32_t> gt;
};

/**
 * @section PITCH_BEND
 *
 * Pitch bend controller implementation.
 */
/**
 * @brief MIDI pitch bend controller
 *
 * Processes analog input to generate MIDI pitch bend messages.
 */
struct pitch_bend_controller
{
   /**
    * @brief Process analog input value and send MIDI pitch bend messages
    *
    * Applies lowpass filtering and noise gating before sending MIDI messages.
    *
    * @param val_ Raw analog input value
    */
   void operator()(uint32_t val_)
   {
      uint32_t val = lp2(lp1(val_));
      if (gt(val))
         midi_out << midi::pitch_bend{0, uint16_t{(val << 4) + (val % 16)}};
   }

   lowpass<8, int32_t> lp1;
   lowpass<16, int32_t> lp2;
   gate<noise_window, int32_t> gt;
};

/**
 * @section PROGRAM_CHANGE
 *
 * Program change controller implementation.
 */
/**
 * @brief MIDI program change controller
 *
 * Handles program change selection via analog input or buttons.
 * Supports individual and group increment/decrement.
 * Persists program change values to flash memory.
 */
struct program_change_controller
{
   program_change_controller()
      : curr{0}
      , base{0}
   {}

   /**
    * @brief Load program change base value from flash memory
    */
   void load()
   {
      if (!flash_b.empty())
         base = flash_b.read();
   }

   /**
    * @brief Save program change base value to flash memory
    *
    * Clamps the value to the valid MIDI range (0-127) before saving.
    */
   void save()
   {
      uint8_t base_ = max(min(base, 127), 0);
      if (base_ != flash_b.read())
         flash_b.write(base_);
   }

   /**
    * @brief Get the current program change value
    *
    * @return Combined and clamped program change value
    */
   uint8_t get()
   {
      return uint8_t{max(min(curr + base, 127), 0)};
   }

   /**
    * @brief Transmit the current program change value as a MIDI message
    */
   void transmit()
   {
      midi_out << midi::program_change{0, get()};
   }

   /**
    * @brief Process analog input for program change selection
    *
    * Implements hysteresis to prevent jitter and maps the analog
    * value to the appropriate program change range.
    *
    * @param val_ Raw analog input value
    */
   void operator()(uint32_t val_)
   {
      uint32_t curr_ = curr * 205;
      int diff = curr_ - val_;
      if (diff < 0)
         diff = -diff;
      if (diff < 8)
         return;

      uint8_t val = (val_ * 5) / 1024;
      if (val != curr)
      {
         curr = val;
         transmit();
      }
   }

   /**
    * @brief Increment program change base value by 1
    *
    * @param sw Switch state (true = pressed, false = released)
    */
   void up(bool sw)
   {
      if (btn_up(sw) && (base < 127))
      {
         ++base;
         reset_save_delay();
         transmit();
      }
   }

   /**
    * @brief Decrement program change base value by 1
    *
    * @param sw Switch state (true = pressed, false = released)
    */
   void down(bool sw)
   {
      if (btn_down(sw) && (base > 0))
      {
         --base;
         reset_save_delay();
         transmit();
      }
   }

   /**
    * @brief Increment program change base value by 5
    *
    * @param sw Switch state (true = pressed, false = released)
    */
   void group_up(bool sw)
   {
      if (grp_btn_up(sw) && (base < 127))
      {
         base += 5;
         reset_save_delay();
         transmit();
      }
   }

   /**
    * @brief Decrement program change base value by 5
    *
    * @param sw Switch state (true = pressed, false = released)
    */
   void group_down(bool sw)
   {
      if (grp_btn_down(sw) && (base > 0))
      {
         base -= 5;
         reset_save_delay();
         transmit();
      }
   }

   int16_t curr;
   int16_t base;
   repeat_button<> btn_up;
   repeat_button<> btn_down;
   repeat_button<> grp_btn_up;
   repeat_button<> grp_btn_down;
};

/**
 * @section SUSTAIN
 *
 * Sustain controller implementation.
 */
/**
 * @brief MIDI sustain pedal controller
 *
 * Processes switch input to generate MIDI sustain pedal messages.
 */
struct sustain_controller
{
   /**
    * @brief Process switch state to generate MIDI sustain pedal messages
    *
    * @param sw Switch state (true = pressed, false = released)
    */
   void operator()(bool sw)
   {
      int state = edge(sw);
      if (state == 1)
         midi_out << midi::control_change{0, midi::cc::sustain, 0};
      else if (state == -1)
         midi_out << midi::control_change{0, midi::cc::sustain, 127};
   }

   edge_detector<> edge;
};

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
controller<midi::cc::channel_volume>   volume_control;
controller<midi::cc::effect_1>         fx1_control;
controller<midi::cc::effect_2>         fx2_control;
controller<midi::cc::modulation>       modulation_control;
pitch_bend_controller                  pitch_bend;
program_change_controller              program_change;
sustain_controller                     sustain_control;
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
   pinMode(ch9 , INPUT_PULLUP);
   pinMode(ch10, INPUT);
   pinMode(ch11, INPUT);
   pinMode(ch12, INPUT);
   pinMode(ch13, INPUT);
   pinMode(ch14, INPUT);
   pinMode(ch15, INPUT);

   pinMode(aux1, INPUT_PULLUP);
   pinMode(aux2, INPUT_PULLUP);
   pinMode(aux3, INPUT_PULLUP);
   pinMode(aux4, INPUT_PULLUP);
   pinMode(aux5, INPUT_PULLUP);
   pinMode(aux6, INPUT_PULLUP);

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
   volume_control(analog_read(ch10));
#endif

#ifdef NEXUS_TEST_PITCH_BEND
   pitch_bend(analog_read(ch10));
#endif

#ifdef NEXUS_TEST_PROGRAM_CHANGE
   program_change(analog_read(ch15));
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
   fx1_control(analog_read(ch11));
#endif

#ifdef NEXUS_TEST_EFFECTS_2
   fx2_control(analog_read(ch11));
#endif

#ifdef NEXUS_TEST_MODULATION
   modulation_control(analog_read(ch11));
#endif

#ifdef NEXUS_TEST_SUSTAIN
   sustain_control(digitalRead(ch12));
#endif

#ifdef NEXUS_TEST_BANK_SELECT
   bank_select_control.up(digitalRead(aux1));
   bank_select_control.down(digitalRead(aux2));
#endif

   // Save the program_change and bank_select_control if needed
   if ((save_delay_start_time != -1)
      && (millis() > (save_delay_start_time + save_delay)))
   {
      program_change.save();
      bank_select_control.save();
      save_delay_start_time = -1;
   }
}

#else // !NEXUS_TEST

/**
 * @brief Control range adjustment
 *
 * The effective range of our controls (e.g. pots) is within 2% of the travel
 * to avoid noise at the extremes of the potentiometer range.
 */
constexpr uint16_t min_x = 1024 * 0.02;
constexpr uint16_t max_x = 1024 * 0.98;

/**
 * @brief Read and normalize analog input
 *
 * Reads an analog input and maps it to the effective range.
 *
 * @param pin The analog pin to read
 * @return Normalized analog value in the range 0-1023
 */
uint16_t analog_read(uint16_t pin)
{
   uint16_t x = analogRead(pin);
   if (x < min_x)
      x = min_x;
   else if (x > max_x)
      x = max_x;
   return map(x, min_x, max_x, 0, 1023);
}

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
      volume_control(analog_read(ch10));
      fx1_control(analog_read(ch11));
      fx2_control(analog_read(ch12));
      pitch_bend(analog_read(ch13));
      program_change(analog_read(ch14));
      modulation_control(analog_read(ch15));

      program_change.up(!digitalRead(aux1));
      program_change.down(!digitalRead(aux2));
      program_change.group_up(!digitalRead(aux3));
      program_change.group_down(!digitalRead(aux4));
      bank_select_control.up(!digitalRead(aux5));
      bank_select_control.down(!digitalRead(aux6));

      prev_time = millis();
   }

   // Save the program_change and bank_select_control if needed
   if ((save_delay_start_time != -1)
      && (millis() > (save_delay_start_time + save_delay)))
   {
      program_change.save();
      bank_select_control.save();
      save_delay_start_time = -1;
   }
}

#endif // NEXUS_TEST
