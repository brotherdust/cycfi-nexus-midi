/**
 * @file debug_logger.cpp
 * @brief Debug logger static member definitions
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#include "config/feature_config.hpp"

#ifdef NEXUS_ENABLE_DEBUG_LOGGING

#include "debug/debug_logger.hpp"

namespace nexus { namespace debug {

// Static member definitions
RingBuffer<NEXUS_DEBUG_BUFFER_SIZE> DebugLogger::buffer;
uint32_t DebugLogger::last_timestamp = 0;

}} // namespace nexus::debug

#endif // NEXUS_ENABLE_DEBUG_LOGGING