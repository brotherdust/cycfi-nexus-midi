/**
 * @file midi.hpp
 * @brief MIDI message definitions and I/O routines
 *
 * This file contains the implementation of MIDI message structures and
 * I/O routines for sending MIDI messages through a serial interface.
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#if !defined(CYCFI_MIDI_HPP_OCTOBER_8_2012)
#define CYCFI_MIDI_HPP_OCTOBER_8_2012

#include <stdint.h>
// #include <energia.h>

namespace cycfi { namespace midi
{
   /**
    * @brief MIDI status byte definitions
    *
    * Contains the standard MIDI status byte values for various MIDI messages.
    */
   namespace status
   {
      enum
      {
         note_off             = 0x80, ///< Note Off message
         note_on              = 0x90, ///< Note On message
         poly_aftertouch      = 0xA0, ///< Polyphonic Aftertouch message
         control_change       = 0xB0, ///< Control Change message
         program_change       = 0xC0, ///< Program Change message
         channel_aftertouch   = 0xD0, ///< Channel Aftertouch message
         pitch_bend           = 0xE0, ///< Pitch Bend message
         sysex                = 0xF0, ///< System Exclusive message start
         song_position        = 0xF2, ///< Song Position Pointer message
         song_select          = 0xF3, ///< Song Select message
         tune_request         = 0xF6, ///< Tune Request message
         sysex_end            = 0xF7, ///< System Exclusive message end
         timing_tick          = 0xF8, ///< Timing Clock message
         start                = 0xFA, ///< Start message
         continue_            = 0xFB, ///< Continue message
         stop                 = 0xFC, ///< Stop message
         active_sensing       = 0xFE, ///< Active Sensing message
         reset                = 0xFF  ///< Reset message
      };
   }

   /**
    * @brief MIDI Control Change controller numbers
    *
    * Contains the standard MIDI Control Change controller numbers.
    */
   namespace cc
   {
      enum controller
      {
         bank_select          = 0x00, ///< Bank Select controller
         modulation           = 0x01, ///< Modulation controller
         breath               = 0x02, ///< Breath controller
         undef_cc_3           = 0x03, ///< Undefined controller
         foot                 = 0x04, ///< Foot controller
         portamento_time      = 0x05, ///< Portamento Time controller
         data_entry           = 0x06, ///< Data Entry controller
         channel_volume       = 0x07, ///< Channel Volume controller
         balance              = 0x08, ///< Balance controller
         undef_cc_9           = 0x09, ///< Undefined controller
         pan                  = 0x0A, ///< Pan controller
         expression           = 0x0B, ///< Expression controller
         effect_1             = 0x0C, ///< Effect 1 controller
         effect_2             = 0x0D, ///< Effect 2 controller
         undef_cc_14          = 0x0E, ///< Undefined controller
         undef_cc_15          = 0x0F, ///< Undefined controller
         general_1            = 0x10, ///< General Purpose controller 1
         general_2            = 0x11, ///< General Purpose controller 2
         general_3            = 0x12, ///< General Purpose controller 3
         general_4            = 0x13, ///< General Purpose controller 4
         undef_cc_20          = 0x14, ///< Undefined controller
         undef_cc_21          = 0x15, ///< Undefined controller
         undef_cc_22          = 0x16, ///< Undefined controller
         undef_cc_23          = 0x17, ///< Undefined controller
         undef_cc_24          = 0x18, ///< Undefined controller
         undef_cc_25          = 0x19, ///< Undefined controller
         undef_cc_26          = 0x1A, ///< Undefined controller
         undef_cc_27          = 0x1B, ///< Undefined controller
         undef_cc_28          = 0x1C, ///< Undefined controller
         undef_cc_29          = 0x1D, ///< Undefined controller
         undef_cc_30          = 0x1E, ///< Undefined controller
         undef_cc_31          = 0x1F, ///< Undefined controller
         bank_select_lsb      = 0x20, ///< Bank Select LSB
         modulation_lsb       = 0x21, ///< Modulation LSB
         breath_lsb           = 0x22, ///< Breath Controller LSB
         foot_lsb             = 0x24, ///< Foot Controller LSB
         portamento_time_lsb  = 0x25, ///< Portamento Time LSB
         data_entry_lsb       = 0x26, ///< Data Entry LSB
         channel_volume_lsb   = 0x27, ///< Channel Volume LSB
         balance_lsb          = 0x28, ///< Balance LSB
         pan_lsb              = 0x2A, ///< Pan LSB
         expression_lsb       = 0x2B, ///< Expression LSB
         effect_1_lsb         = 0x2C, ///< Effect 1 LSB
         effect_2_lsb         = 0x2D, ///< Effect 2 LSB
         general_1_lsb        = 0x30, ///< General Purpose 1 LSB
         general_2_lsb        = 0x31, ///< General Purpose 2 LSB
         general_3_lsb        = 0x32, ///< General Purpose 3 LSB
         general_4_lsb        = 0x33, ///< General Purpose 4 LSB

         sustain              = 0x40, ///< Sustain Pedal
         portamento           = 0x41, ///< Portamento On/Off
         sostenuto            = 0x42, ///< Sostenuto Pedal
         soft_pedal           = 0x43, ///< Soft Pedal
         legato               = 0x44, ///< Legato Footswitch
         hold_2               = 0x45, ///< Hold 2

         sound_controller_1   = 0x46, ///< Sound Controller 1 (default: sound variation)
         sound_controller_2   = 0x47, ///< Sound Controller 2 (default: timbre/harmonic content)
         sound_controller_3   = 0x48, ///< Sound Controller 3 (default: release time)
         sound_controller_4   = 0x49, ///< Sound Controller 4 (default: attack time)
         sound_controller_5   = 0x4A, ///< Sound Controller 5 (default: brightness)
         sound_controller_6   = 0x4B, ///< Sound Controller 6 (no default)
         sound_controller_7   = 0x4C, ///< Sound Controller 7 (no default)
         sound_controller_8   = 0x4D, ///< Sound Controller 8 (no default)
         sound_controller_9   = 0x4E, ///< Sound Controller 9 (no default)
         sound_controller_10  = 0x4F, ///< Sound Controller 10 (no default)

         general_5            = 0x50, ///< General Purpose Controller 5
         general_6            = 0x51, ///< General Purpose Controller 6
         general_7            = 0x52, ///< General Purpose Controller 7
         general_8            = 0x53, ///< General Purpose Controller 8

         portamento_control   = 0x54, ///< Portamento Control
         effects_1_depth      = 0x5B, ///< Effects 1 Depth (previously reverb send)
         effects_2_depth      = 0x5C, ///< Effects 2 Depth (previously tremolo depth)
         effects_3_depth      = 0x5D, ///< Effects 3 Depth (previously chorus depth)
         effects_4_depth      = 0x5E, ///< Effects 4 Depth (previously celeste/detune depth)
         effects_5_depth      = 0x5F, ///< Effects 5 Depth (previously phaser effect depth)
         data_inc             = 0x60, ///< Data Increment (+1)
         data_dec             = 0x61, ///< Data Decrement (-1)

         nonrpn_lsb           = 0x62, ///< Non-Registered Parameter Number LSB
         nonrpn_msb           = 0x63, ///< Non-Registered Parameter Number MSB
         rpn_lsb              = 0x64, ///< Registered Parameter Number LSB
         rpn_msb              = 0x65, ///< Registered Parameter Number MSB
         undef_cc_102         = 0x66, ///< Undefined controller
         undef_cc_103         = 0x67, ///< Undefined controller
         undef_cc_104         = 0x68, ///< Undefined controller
         undef_cc_105         = 0x69, ///< Undefined controller
         undef_cc_106         = 0x6A, ///< Undefined controller
         undef_cc_107         = 0x6B, ///< Undefined controller
         undef_cc_108         = 0x6C, ///< Undefined controller
         undef_cc_109         = 0x6D, ///< Undefined controller
         undef_cc_110         = 0x6E, ///< Undefined controller
         undef_cc_111         = 0x6F, ///< Undefined controller
         undef_cc_112         = 0x70, ///< Undefined controller
         undef_cc_113         = 0x71, ///< Undefined controller
         undef_cc_114         = 0x72, ///< Undefined controller
         undef_cc_115         = 0x73, ///< Undefined controller
         undef_cc_116         = 0x74, ///< Undefined controller
         undef_cc_117         = 0x75, ///< Undefined controller
         undef_cc_118         = 0x76, ///< Undefined controller
         undef_cc_119         = 0x77, ///< Undefined controller
         all_sounds_off       = 0x78, ///< All Sounds Off
         reset                = 0x79, ///< Reset All Controllers
         local                = 0x7A, ///< Local Control On/Off
         all_notes_off        = 0x7B, ///< All Notes Off
         omni_off             = 0x7C, ///< Omni Mode Off
         omni_on              = 0x7D, ///< Omni Mode On
         mono                 = 0x7E, ///< Mono Mode On
         poly                 = 0x7F  ///< Poly Mode On
      };
   }

   /**
    * @section MIDI_MESSAGES
    *
    * MIDI message structures and I/O routines.
    */
   /**
    * @brief Generic MIDI message base class
    *
    * Base template for all MIDI message types.
    *
    * @tparam size_ The size of the MIDI message in bytes
    */
   template <int size_>
   struct message
   {
      static int const size = size_; ///< Size of the message in bytes
      uint8_t data[size];            ///< Raw message data
   };

   /**
    * @brief MIDI output stream
    *
    * Provides functionality to send MIDI messages through a serial interface.
    */
   struct midi_stream
   {
      /**
       * @brief Constructor
       */
      midi_stream()
      {
      }

      /**
       * @brief Initialize the MIDI stream
       *
       * Sets up the serial interface at the standard MIDI baud rate of 31250.
       */
      void start()
      {
         Serial.begin(31250);
      }

      /**
       * @brief Send a single byte to the MIDI stream
       *
       * @param val The byte to send
       * @return Reference to this stream for chaining
       */
      midi_stream& operator<<(uint8_t val)
      {
         Serial.write(val);
         return *this;
      }
   };

   /**
    * @brief Send a MIDI message to a MIDI stream
    *
    * @tparam size Size of the MIDI message in bytes
    * @param out The MIDI stream to send to
    * @param msg The MIDI message to send
    * @return Reference to the MIDI stream for chaining
    */
   template <int size>
   inline midi_stream&
   operator<<(midi_stream& out, message<size> const& msg)
   {
      for (int i = 0; i < size; ++i)
         out << msg.data[i];
      return out;
   }

   /**
    * @brief MIDI Note Off message
    *
    * Represents a MIDI Note Off message (status byte 0x80).
    */
   struct note_off : message<3>
   {
      /**
       * @brief Constructor
       *
       * @param channel MIDI channel (0-15)
       * @param key MIDI note number (0-127)
       * @param velocity Note-off velocity (0-127)
       */
      note_off(uint8_t channel, uint8_t key, uint8_t velocity)
      {
         data[0] = channel | status::note_off;
         data[1] = key;
         data[2] = velocity;
      }

      /**
       * @brief Get the MIDI channel
       * @return MIDI channel (0-15)
       */
      uint8_t channel() const { return data[0] & 0x0F; }
      
      /**
       * @brief Get the note number
       * @return MIDI note number (0-127)
       */
      uint8_t key() const { return data[1]; }
      
      /**
       * @brief Get the note-off velocity
       * @return Velocity value (0-127)
       */
      uint8_t velocity() const { return data[2]; }
   };

   /**
    * @brief MIDI Note On message
    *
    * Represents a MIDI Note On message (status byte 0x90).
    * Note that a Note On with velocity 0 is equivalent to a Note Off.
    */
   struct note_on : message<3>
   {
      /**
       * @brief Constructor
       *
       * @param channel MIDI channel (0-15)
       * @param key MIDI note number (0-127)
       * @param velocity Note-on velocity (0-127)
       */
      note_on(uint8_t channel, uint8_t key, uint8_t velocity)
      {
         data[0] = channel | status::note_on;
         data[1] = key;
         data[2] = velocity;
      }

      /**
       * @brief Get the MIDI channel
       * @return MIDI channel (0-15)
       */
      uint8_t channel() const { return data[0] & 0x0F; }
      
      /**
       * @brief Get the note number
       * @return MIDI note number (0-127)
       */
      uint8_t key() const { return data[1]; }
      
      /**
       * @brief Get the note-on velocity
       * @return Velocity value (0-127)
       */
      uint8_t velocity() const { return data[2]; }
   };

   /**
    * @brief MIDI Polyphonic Aftertouch message
    *
    * Represents a MIDI Polyphonic Aftertouch message (status byte 0xA0).
    * This message applies pressure to a specific note.
    */
   struct poly_aftertouch : message<3>
   {
      /**
       * @brief Constructor
       *
       * @param channel MIDI channel (0-15)
       * @param key MIDI note number (0-127)
       * @param pressure Aftertouch pressure value (0-127)
       */
      poly_aftertouch(uint8_t channel, uint8_t key, uint8_t pressure)
      {
         data[0] = channel | status::poly_aftertouch;
         data[1] = key;
         data[2] = pressure;
      }

      /**
       * @brief Get the MIDI channel
       * @return MIDI channel (0-15)
       */
      uint8_t channel() const { return data[0] & 0x0F; }
      
      /**
       * @brief Get the note number
       * @return MIDI note number (0-127)
       */
      uint8_t key() const { return data[1]; }
      
      /**
       * @brief Get the aftertouch pressure
       * @return Pressure value (0-127)
       */
      uint8_t pressure() const { return data[2]; }
   };

   /**
    * @brief MIDI Control Change message
    *
    * Represents a MIDI Control Change message (status byte 0xB0).
    * Used to control various parameters of a MIDI device.
    */
   struct control_change : message<3>
   {
      typedef cc::controller controller_type;
      
      /**
       * @brief Constructor
       *
       * @param channel MIDI channel (0-15)
       * @param ctrl Controller number from cc::controller enum
       * @param value Controller value (0-127)
       */
      control_change(uint8_t channel, controller_type ctrl, uint8_t value)
      {
         data[0] = channel | status::control_change;
         data[1] = ctrl;
         data[2] = value;
      }

      /**
       * @brief Get the MIDI channel
       * @return MIDI channel (0-15)
       */
      uint8_t channel() const { return data[0] & 0x0F; }
      
      /**
       * @brief Get the controller number
       * @return Controller number as controller_type enum
       */
      controller_type controller() const { return controller_type(data[1]); }
      
      /**
       * @brief Get the controller value
       * @return Controller value (0-127)
       */
      uint8_t value() const { return data[2]; }
   };

   /**
    * @brief MIDI Program Change message
    *
    * Represents a MIDI Program Change message (status byte 0xC0).
    * Used to change the program (patch, preset) on a MIDI device.
    */
   struct program_change : message<2>
   {
      /**
       * @brief Constructor
       *
       * @param channel MIDI channel (0-15)
       * @param preset Program number (0-127)
       */
      program_change(uint8_t channel, uint8_t preset)
      {
         data[0] = channel | status::program_change;
         data[1] = preset;
      }

      /**
       * @brief Get the MIDI channel
       * @return MIDI channel (0-15)
       */
      uint8_t channel() const { return data[0] & 0x0F; }
      
      /**
       * @brief Get the program number
       * @return Program number (0-127)
       */
      uint8_t preset() const { return data[1]; }
   };

   /**
    * @brief MIDI Channel Aftertouch message
    *
    * Represents a MIDI Channel Aftertouch message (status byte 0xD0).
    * This message applies pressure to all notes on a channel.
    */
   struct channel_aftertouch : message<2>
   {
      /**
       * @brief Constructor
       *
       * @param channel MIDI channel (0-15)
       * @param pressure Aftertouch pressure value (0-127)
       */
      channel_aftertouch(uint8_t channel, uint8_t pressure)
      {
         data[0] = channel | status::channel_aftertouch;
         data[1] = pressure;
      }

      /**
       * @brief Get the MIDI channel
       * @return MIDI channel (0-15)
       */
      uint8_t channel() const { return data[0] & 0x0F; }
      
      /**
       * @brief Get the aftertouch pressure
       * @return Pressure value (0-127)
       */
      uint8_t pressure() const { return data[1]; }
   };

   /**
    * @brief MIDI Pitch Bend message
    *
    * Represents a MIDI Pitch Bend message (status byte 0xE0).
    * Used to bend the pitch of notes up or down.
    */
   struct pitch_bend : message<3>
   {
      /**
       * @brief Constructor with combined value
       *
       * @param channel MIDI channel (0-15)
       * @param value Combined pitch bend value (0-16383)
       */
      pitch_bend(uint8_t channel, uint16_t value)
      {
         data[0] = channel | status::pitch_bend;
         data[1] = value & 0x7F;
         data[2] = value >> 7;
      }

      /**
       * @brief Constructor with separate LSB and MSB
       *
       * @param channel MIDI channel (0-15)
       * @param lsb Least significant 7 bits (0-127)
       * @param msb Most significant 7 bits (0-127)
       */
      pitch_bend(uint8_t channel, uint16_t lsb, uint8_t msb)
      {
         data[0] = channel | status::pitch_bend;
         data[1] = lsb;
         data[2] = msb;
      }

      /**
       * @brief Get the MIDI channel
       * @return MIDI channel (0-15)
       */
      uint8_t channel() const { return data[0] & 0x0F; }
      
      /**
       * @brief Get the combined pitch bend value
       * @return Combined pitch bend value (0-16383)
       */
      uint16_t value() const { return data[1] | (data[2] << 7); }
   };

   /**
    * @brief MIDI Song Position Pointer message
    *
    * Represents a MIDI Song Position Pointer message (status byte 0xF2).
    * Specifies the position in a song in MIDI beats (16th notes).
    */
   struct song_position : message<3>
   {
      /**
       * @brief Constructor with combined value
       *
       * @param position Song position in MIDI beats (0-16383)
       */
      song_position(uint16_t position)
      {
         data[0] = status::song_position;
         data[1] = position & 0x7F;
         data[2] = position >> 7;
      }

      /**
       * @brief Constructor with separate LSB and MSB
       *
       * @param lsb Least significant 7 bits (0-127)
       * @param msb Most significant 7 bits (0-127)
       */
      song_position(uint8_t lsb, uint8_t msb)
      {
         data[0] = status::song_position;
         data[1] = lsb;
         data[2] = msb;
      }

      /**
       * @brief Get the song position
       * @return Song position in MIDI beats (0-16383)
       */
      uint16_t position() const { return data[1] | (data[2] << 7); }
   };

   /**
    * @brief MIDI Song Select message
    *
    * Represents a MIDI Song Select message (status byte 0xF3).
    * Used to select a song number on a MIDI device.
    */
   struct song_select : message<2>
   {
      /**
       * @brief Constructor
       *
       * @param song_number Song number to select (0-127)
       */
      song_select(uint8_t song_number)
      {
         data[0] = status::song_select;
         data[1] = song_number;
      }
   
      /**
       * @brief Get the song number
       * @return Selected song number (0-127)
       */
      uint16_t song_number() const { return data[1]; }
   };

   /**
    * @brief MIDI Tune Request message
    *
    * Represents a MIDI Tune Request message (status byte 0xF6).
    * Requests analog synthesizers to tune their oscillators.
    */
   struct tune_request : message<1>
   {
      /**
       * @brief Constructor
       */
      tune_request()
      {
         data[0] = status::tune_request;
      }
   };

   /**
    * @brief MIDI Timing Clock message
    *
    * Represents a MIDI Timing Clock message (status byte 0xF8).
    * Sent 24 times per quarter note to synchronize MIDI devices.
    */
   struct timing_tick : message<1>
   {
      /**
       * @brief Constructor
       */
      timing_tick()
      {
         data[0] = status::timing_tick;
      }
   };

   /**
    * @brief MIDI Start message
    *
    * Represents a MIDI Start message (status byte 0xFA).
    * Commands a MIDI device to start playing from the beginning.
    */
   struct start : message<1>
   {
      /**
       * @brief Constructor
       */
      start()
      {
         data[0] = status::start;
      }
   };

   /**
    * @brief MIDI Continue message
    *
    * Represents a MIDI Continue message (status byte 0xFB).
    * Commands a MIDI device to resume playing from the current position.
    */
   struct continue_ : message<1>
   {
      /**
       * @brief Constructor
       */
      continue_()
      {
         data[0] = status::continue_;
      }
   };

   /**
    * @brief MIDI Stop message
    *
    * Represents a MIDI Stop message (status byte 0xFC).
    * Commands a MIDI device to stop playing.
    */
   struct stop : message<1>
   {
      /**
       * @brief Constructor
       */
      stop()
      {
         data[0] = status::stop;
      }
   };

   /**
    * @brief MIDI Active Sensing message
    *
    * Represents a MIDI Active Sensing message (status byte 0xFE).
    * Sent periodically to indicate the MIDI connection is still active.
    * If a device receives no MIDI data for 300ms after receiving an Active Sensing message,
    * it should turn off all voices and return to normal operation.
    */
   struct active_sensing : message<1>
   {
      /**
       * @brief Constructor
       */
      active_sensing()
      {
         data[0] = status::active_sensing;
      }
   };

   /**
    * @brief MIDI System Reset message
    *
    * Represents a MIDI System Reset message (status byte 0xFF).
    * Commands a MIDI device to reset itself to power-up status.
    */
   struct reset : message<1>
   {
      /**
       * @brief Constructor
       */
      reset()
      {
         data[0] = status::reset;
      }
   };

   /**
    * @brief MIDI System Exclusive message
    *
    * Represents a MIDI System Exclusive (SysEx) message (status byte 0xF0).
    * Used for manufacturer-specific or device-specific data transfer.
    *
    * @tparam size_ The size of the SysEx data payload in bytes
    */
   template <int size_>
   struct sysex : message<size_ + 5>
   {
      /**
       * @brief Constructor
       *
       * @param id Manufacturer ID
       * @param data_in Pointer to the SysEx data payload
       */
      sysex(uint16_t id, uint8_t const* data_in)
      {
         this->data[0] = status::sysex;
         this->data[1] = 0;
         this->data[2] = id >> 8;
         this->data[3] = id & 0x7f;
         this->data[size_ + 4] = status::sysex_end;
         for (int i = 0; i != size_; ++i)
            this->data[i+4] = data_in[i] & 0x7f;
      }
   
      /**
       * @brief Get the manufacturer ID
       * @return Manufacturer ID
       */
      uint16_t id() const { return (this->data[2] << 8) | this->data[3]; }
   };
}}

#endif
