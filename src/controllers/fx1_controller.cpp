/**
 * @file fx1_controller.cpp
 * @brief MIDI effects 1 controller implementation
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#include <Arduino.h>  // Must be first for Arduino functions
#include "controllers/base_controller.hpp"

// Explicit instantiation of effects 1 controller
template class nexus::controllers::Controller<cycfi::midi::cc::effect_1>;