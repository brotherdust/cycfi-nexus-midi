/**
 * @file hardware_config.cpp
 * @brief Hardware configuration implementation for Nexus MIDI controller
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#include "config/hardware_config.hpp"
#include <Arduino.h>

namespace nexus {
namespace config {

/**
 * @section PIN_DEFINITIONS
 *
 * Pin definitions - maintaining original values from nexus_midi.ino
 */

// Main control pins
const int ch9  = P2_0; // Digital input
const int ch10 = P1_0; // Analog and digital input
const int ch11 = P1_3; // Analog and digital input
const int ch12 = P1_4; // Analog and digital input
const int ch13 = P1_5; // Analog and digital input
const int ch14 = P1_6; // Analog and digital input
const int ch15 = P1_7; // Analog and digital input

// Auxiliary control pins
const int aux1 = P2_1; // Digital input
const int aux2 = P2_2; // Digital input
const int aux3 = P2_3; // Digital input
const int aux4 = P2_4; // Digital input
const int aux5 = P2_5; // Digital input
const int aux6 = P2_6; // Digital input

/**
 * @section CONFIGURATION_CONSTANTS
 */

// Control range adjustment
const uint16_t min_x = 1024 * 0.02;  // 2% of full range
const uint16_t max_x = 1024 * 0.98;  // 98% of full range

/**
 * @brief Initialize all hardware pins
 */
void initialize_pins() {
    // Main control pins
    pinMode(ch9 , INPUT_PULLUP);
    pinMode(ch10, INPUT);
    pinMode(ch11, INPUT);
    pinMode(ch12, INPUT);
    pinMode(ch13, INPUT);
    pinMode(ch14, INPUT);
    pinMode(ch15, INPUT);
    
    // Auxiliary control pins
    pinMode(aux1, INPUT_PULLUP);
    pinMode(aux2, INPUT_PULLUP);
    pinMode(aux3, INPUT_PULLUP);
    pinMode(aux4, INPUT_PULLUP);
    pinMode(aux5, INPUT_PULLUP);
    pinMode(aux6, INPUT_PULLUP);
}

/**
 * @brief Read and normalize analog input
 *
 * @param pin The analog pin to read
 * @return Normalized analog value in the range 0-1023
 */
uint16_t analog_read(uint16_t pin) {
    uint16_t x = analogRead(pin);
    
    // Clamp to effective range
    if (x < min_x)
        x = min_x;
    else if (x > max_x)
        x = max_x;
    
    // Map to full range
    return map(x, min_x, max_x, 0, 1023);
}

} // namespace config
} // namespace nexus