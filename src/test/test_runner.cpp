/**
 * @file test_runner.cpp
 * @brief Test runner implementation
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#include <Arduino.h>  // Must be first for Arduino functions
#include "test/test_runner.hpp"
#include "controllers/controller_factory.hpp"
#include "config/hardware_config.hpp"
#include "midi.hpp"
#include "util.hpp"
#include "MspFlash.h"

// Global MIDI output stream reference
extern cycfi::midi::midi_stream midi_out;

namespace nexus {
namespace test {

// Global test runner instance
TestRunner test_runner;

#ifdef NEXUS_TEST

// Test note structure
struct note {
    void operator()(bool sw) {
        int state = edge(sw);
        if (state == 1)
            midi_out << cycfi::midi::note_on{0, 80, 127};
        else if (state == -1)
            midi_out << cycfi::midi::note_off{0, 80, 127};
    }
    
    cycfi::edge_detector<> edge;
};

static note _note;

#endif // NEXUS_TEST

void TestRunner::run() {
#ifdef NEXUS_TEST
    test_note();
    test_volume();
    test_pitch_bend();
    test_program_change();
    test_program_change_up_down();
    test_program_change_group_up_down();
    test_effects_1();
    test_effects_2();
    test_modulation();
    test_sustain();
    test_bank_select();
    
    // Save controller states if needed
    controllers::controllers.save_states();
#endif
}

void TestRunner::dump_flash() {
#ifdef NEXUS_DUMP_FLASH
    unsigned char* seg_b = SEGMENT_B;
    midi_out << cycfi::midi::sysex<16> {0x5555, seg_b};
    midi_out << cycfi::midi::sysex<16> {0x5555, seg_b + 16};
    midi_out << cycfi::midi::sysex<16> {0x5555, seg_b + 32};
    midi_out << cycfi::midi::sysex<16> {0x5555, seg_b + 48};
    
    unsigned char* seg_c = SEGMENT_C;
    midi_out << cycfi::midi::sysex<16> {0x5555, seg_c};
    midi_out << cycfi::midi::sysex<16> {0x5555, seg_c + 16};
    midi_out << cycfi::midi::sysex<16> {0x5555, seg_c + 32};
    midi_out << cycfi::midi::sysex<16> {0x5555, seg_c + 48};
#endif
}

void TestRunner::test_note() {
#ifdef NEXUS_TEST_NOTE
    _note(digitalRead(config::aux1));
#endif
}

void TestRunner::test_volume() {
#ifdef NEXUS_TEST_VOLUME
    controllers::controllers.volume(config::analog_read(config::ch10));
#endif
}

void TestRunner::test_pitch_bend() {
#ifdef NEXUS_TEST_PITCH_BEND
    controllers::controllers.pitch_bend(config::analog_read(config::ch10));
#endif
}

void TestRunner::test_program_change() {
#ifdef NEXUS_TEST_PROGRAM_CHANGE
    controllers::controllers.program_change(config::analog_read(config::ch15));
#endif
}

void TestRunner::test_program_change_up_down() {
#ifdef NEXUS_TEST_PROGRAM_CHANGE_UP_DOWN
    controllers::controllers.program_change.up(digitalRead(config::ch12));
    controllers::controllers.program_change.down(digitalRead(config::ch13));
#endif
}

void TestRunner::test_program_change_group_up_down() {
#ifdef NEXUS_TEST_PROGRAM_CHANGE_GROUP_UP_DOWN
    controllers::controllers.program_change.group_up(digitalRead(config::ch12));
    controllers::controllers.program_change.group_down(digitalRead(config::ch13));
#endif
}

void TestRunner::test_effects_1() {
#ifdef NEXUS_TEST_EFFECTS_1
    controllers::controllers.fx1(config::analog_read(config::ch11));
#endif
}

void TestRunner::test_effects_2() {
#ifdef NEXUS_TEST_EFFECTS_2
    controllers::controllers.fx2(config::analog_read(config::ch11));
#endif
}

void TestRunner::test_modulation() {
#ifdef NEXUS_TEST_MODULATION
    controllers::controllers.modulation(config::analog_read(config::ch11));
#endif
}

void TestRunner::test_sustain() {
#ifdef NEXUS_TEST_SUSTAIN
    controllers::controllers.sustain(digitalRead(config::ch12));
#endif
}

void TestRunner::test_bank_select() {
#ifdef NEXUS_TEST_BANK_SELECT
    controllers::controllers.bank_select.up(digitalRead(config::aux1));
    controllers::controllers.bank_select.down(digitalRead(config::aux2));
#endif
}

} // namespace test
} // namespace nexus