/**
 * @file volume_controller.cpp
 * @brief MIDI volume controller implementation
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#include <Arduino.h>  // Must be first for Arduino functions
#include "controllers/base_controller.hpp"

// Explicit instantiation of volume controller
template class nexus::controllers::Controller<cycfi::midi::cc::channel_volume>;