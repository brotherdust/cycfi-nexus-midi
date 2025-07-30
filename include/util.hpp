/**
 * @file util.hpp
 * @brief Utility classes for embedded applications
 *
 * This file contains various utility classes for embedded applications,
 * including switch debouncers, edge detectors, button handlers, and filters.
 *
 * The debouncing mechanism can operate in two modes:
 * 1. Standard mode (default): Debounces both press and release events
 * 2. Immediate mode: Provides immediate press detection with release-only debouncing
 *
 * To enable immediate mode, define NEXUS_ENABLE_IMMEDIATE_DEBOUNCE in feature_config.hpp
 * or before including this file:
 * @code
 * #define NEXUS_ENABLE_IMMEDIATE_DEBOUNCE
 * #include "util.hpp"
 * @endcode
 *
 * @copyright Copyright (c) 2016 Cycfi Research
 * @license Distributed under the MIT License [ https://opensource.org/licenses/MIT ]
 */
#if !defined(CYCFI_UTIL_HPP_NOVEMBER_11_2016)
#define CYCFI_UTIL_HPP_NOVEMBER_11_2016

#include <stdint.h>
// #include <energia.h>
#include "config/feature_config.hpp"

namespace cycfi
{
   /**
    * @brief A switch debouncer
    *
    * Implements a counter-based debouncing algorithm for switches.
    * 
    * Standard mode: The switch state is considered changed only after a 
    * consistent reading for a specified number of samples.
    * 
    * Immediate mode (when NEXUS_ENABLE_IMMEDIATE_DEBOUNCE is defined):
    * Press detection is immediate, release requires consistent samples.
    *
    * @tparam samples Number of consistent samples required to change state (default: 10)
    */
   template <int samples = 10>
   struct debouncer
   {
      /**
       * @brief Constructor
       */
      debouncer()
       : counter(0)
       , result(false)
      {}

      /**
       * @brief Process a switch reading
       *
       * @param sw Current raw switch state
       * @return Debounced switch state
       */
      bool operator()(bool sw)
      {
         if (sw)
         {
#ifdef NEXUS_ENABLE_IMMEDIATE_DEBOUNCE
            // Immediate press detection
            counter = samples;
            result = true;
#else
            // Standard debounced press
            if (counter == samples)
               result = true;
            else
               ++counter;
#endif
         }
         else
         {
            // Release is always debounced
            if (counter == 0)
               result = false;
            else
               --counter;
         }
         return result;
      }

   private:
      int counter;  ///< Sample counter
      bool result;  ///< Current debounced state
   };

   /**
    * @brief A switch edge detector
    *
    * Detects rising and falling edges of a debounced switch.
    * Returns 1 for rising edge, -1 for falling edge, and 0 for no change.
    *
    * When NEXUS_ENABLE_IMMEDIATE_DEBOUNCE is defined, rising edges are
    * detected immediately while falling edges are debounced.
    *
    * @tparam samples Number of consistent samples required to change state (default: 10)
    */
   template <int samples = 10>
   struct edge_detector : debouncer<samples>
   {
      typedef debouncer<samples> base_type;

      /**
       * @brief Constructor
       */
      edge_detector()
       : prev(false)
      {}

      /**
       * @brief Process a switch reading and detect edges
       *
       * @param sw Current raw switch state
       * @return 1 for rising edge, -1 for falling edge, 0 for no change
       */
      int operator()(bool sw)
      {
         bool curr = base_type::operator()(sw);
         if (prev != curr)
         {
            prev = curr;
            return curr ? 1 : -1;
         }
         return 0;
      }

   private:
      bool prev;  ///< Previous debounced state
   };

   /**
    * @brief A key detector with delay and repeat rate
    *
    * Implements a button handler with initial delay and repeat rate functionality,
    * similar to keyboard key repeat behavior. After the initial press and delay,
    * the button will repeatedly trigger at the specified rate while held down.
    *
    * When NEXUS_ENABLE_IMMEDIATE_DEBOUNCE is defined, the initial press is
    * detected immediately for more responsive user interaction.
    *
    * @tparam delay_ Initial delay in milliseconds before repeating (default: 1000)
    * @tparam rate Repeat rate in milliseconds (default: 100)
    * @tparam samples Number of consistent samples required to change state (default: 10)
    */
   template <int delay_ = 1000, int rate = 100, int samples = 10>
   struct repeat_button : edge_detector<samples>
   {
      typedef edge_detector<samples> base_type;

      /**
       * @brief Constructor
       */
      repeat_button()
       : start_time(-1)
      {}

      /**
       * @brief Process a switch reading with repeat functionality
       *
       * @param sw Current raw switch state
       * @return True on initial press or when repeating, false otherwise
       */
      bool operator()(bool sw)
      {
         int state = base_type::operator()(sw);

         // rising edge
         if (state == 1)
         {
            start_time = millis();
            delay = delay_; // initial delay
            return true;
         }

         // falling edge
         else if (state == -1)
         {
            // reset
            start_time = -1;
            return false;
         }

         if (start_time == -1)
            return 0;

         // repeat button handling
         int now = millis();
         if (now > (start_time + delay))
         {
            start_time = now;
            delay = rate; // repeat delay
            return true;
         }
         return 0;
      }
      
      int start_time;  ///< Time when button was last triggered
      int delay;       ///< Current delay (initial or repeat)
   };

   /**
    * @brief Basic leaky-integrator lowpass filter
    *
    * Implements a simple digital RC filter (leaky integrator).
    * The parameter k determines the filter effect - higher values
    * result in slower response (more filtering).
    *
    * This simulates the RC filter in digital form using the equation:
    *    y[i] = rho * y[i-1] + s
    * where rho < 1. To avoid floating point, we use k instead which
    * allows for integer operations. In terms of k, rho = 1 - (1/k).
    * The actual formula used is:
    *    y[i] += s - (y[i-1] / k)
    *
    * @tparam k Filter coefficient (recommended to use power of 2 for efficiency)
    * @tparam T Data type for calculations (default: int)
    */
   template <int k, typename T = int>
   struct lowpass
   {
      /**
       * @brief Constructor
       */
      lowpass()
       : y(0)
      {}

      /**
       * @brief Process a sample through the filter
       *
       * @param s Input sample
       * @return Filtered output sample
       */
      T operator()(T s)
      {
         y += s - (y / k);
         return y / k;
      }

      T y;  ///< Filter state
   };

   /**
    * @brief Noise gate for signal processing
    *
    * Returns true if the signal is outside a window around the previous value.
    * For example, if window is 5, the previous signal is 20, and the current
    * signal is outside the range 15-25, the function returns true and updates
    * the stored value. Otherwise, it returns false.
    *
    * @tparam window Size of the window around the previous value
    * @tparam T Data type for calculations (default: int)
    */
   template <unsigned window, typename T = int>
   struct gate
   {
      /**
       * @brief Constructor
       */
      gate()
       : val(0)
      {}

      /**
       * @brief Process a sample through the gate
       *
       * @param s Input sample
       * @return True if sample is outside the window, false otherwise
       */
      bool operator()(T s)
      {
         if ((s < (val-window)) || (s > (val+window)))
         {
            val = s;
            return true;
         }
         return false;
      }

      T val;  ///< Previous value
   };
}

#endif
