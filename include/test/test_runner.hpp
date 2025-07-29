/**
 * @file test_runner.hpp
 * @brief Test functionality for the Nexus MIDI controller
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_TEST_RUNNER_HPP
#define NEXUS_TEST_RUNNER_HPP

#include <stdint.h>

namespace nexus {
namespace test {

/**
 * @brief Test runner class for handling various test modes
 *
 * Provides methods to run specific tests based on compile-time flags
 */
class TestRunner {
public:
    /**
     * @brief Run all enabled tests
     *
     * Executes tests based on compile-time NEXUS_TEST_* defines
     */
    void run();
    
    /**
     * @brief Dump flash memory contents via MIDI sysex
     *
     * Sends flash segments B and C as MIDI sysex messages
     */
    void dump_flash();
    
private:
    // Test methods
    void test_note();
    void test_volume();
    void test_pitch_bend();
    void test_program_change();
    void test_program_change_up_down();
    void test_program_change_group_up_down();
    void test_effects_1();
    void test_effects_2();
    void test_modulation();
    void test_sustain();
    void test_bank_select();
};

// Global test runner instance
extern TestRunner test_runner;

} // namespace test
} // namespace nexus

#endif // NEXUS_TEST_RUNNER_HPP