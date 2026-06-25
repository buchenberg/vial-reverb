/* Copyright 2013-2019 Matt Tytel
 *
 * vial is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vial is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with vial.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "common.h"

namespace vial {

  constexpr int kMaxModulationConnections = 64;

  namespace constants {
    constexpr int kNumSyncedFrequencyRatios = 13;
    constexpr vial::mono_float kSyncedFrequencyRatios[kNumSyncedFrequencyRatios] = {
      0.0f,
      1.0f / 128.0f,
      1.0f / 64.0f,
      1.0f / 32.0f,
      1.0f / 16.0f,
      1.0f / 8.0f,
      1.0f / 4.0f,
      1.0f / 2.0f,
      1.0f,
      2.0f,
      4.0f,
      8.0f,
      16.0f
    };

    const poly_float kFirstVoiceOne(1.0f, 1.0f, 0.0f, 0.0f);
    const poly_mask kFullMask = poly_float::equal(0.0f, 0.0f);
    const poly_mask kFirstMask = poly_float::equal(kFirstVoiceOne, 1.0f);
  } // namespace constants
} // namespace vial
