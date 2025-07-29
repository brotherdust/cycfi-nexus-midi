/**
 * @file hardware_config.hpp
 * @brief Hardware configuration and pin definitions for Nexus MIDI controller
 *
 * This file contains all hardware-specific configurations including pin
 * definitions, hardware constants, and initialization functions.
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#ifndef NEXUS_HARDWARE_CONFIG_HPP
#define NEXUS_HARDWARE_CONFIG_HPP

#include <stdint.h>

namespace nexus {
namespace config {

/**
 * @section PIN_DEFINITIONS
 *
 * Pin definitions for the MSP430 microcontroller
 * These are declared as extern to maintain original linkage
 */

// Main control pins - analog and digital inputs
extern const int ch9;  // Digital input
extern const int ch10; // Analog and digital input
extern const int ch11; // Analog and digital input
extern const int ch12; // Analog and digital input
extern const int ch13; // Analog and digital input
extern const int ch14; // Analog and digital input
extern const int ch15; // Analog and digital input

// Auxiliary control pins - digital inputs
extern const int aux1; // Digital input
extern const int aux2; // Digital input
extern const int aux3; // Digital input
extern const int aux4; // Digital input
extern const int aux5; // Digital input
extern const int aux6; // Digital input

/**
 * @section CONFIGURATION_CONSTANTS
 *
 * Hardware-specific configuration values
 */

// Noise window for analog readings
#ifdef NEXUS_TEST
constexpr int noise_window = 4;
#else
constexpr int noise_window = 2;
#endif

// Control range adjustment constants
// The effective range of our controls (e.g. pots) is within 2% of the travel
// to avoid noise at the extremes of the potentiometer range
extern const uint16_t min_x;
extern const uint16_t max_x;

/**
 * @brief Initialize all hardware pins
 *
 * Sets up all input pins with appropriate modes (INPUT or INPUT_PULLUP)
 */
void initialize_pins();

/**
 * @brief Read and normalize analog input
 *
 * Reads an analog input and maps it to the effective range,
 * compensating for noise at the extremes of potentiometer travel.
 *
 * @param pin The analog pin to read
 * @return Normalized analog value in the range 0-1023
 */
uint16_t analog_read(uint16_t pin);

} // namespace config
} // namespace nexus

#endif // NEXUS_HARDWARE_CONFIG_HPP