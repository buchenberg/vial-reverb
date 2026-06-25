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

#include "poly_utils.h"
#include <cmath>

// These are faster but less accurate versions of utility functions.

namespace vial {

  namespace {
    constexpr mono_float kDbGainConversionMult = 6.02059991329f;
    constexpr mono_float kDbMagnitudeConversionMult = 1.0f / kDbGainConversionMult;
  }

  namespace futils {

    force_inline poly_float exp2(poly_float exponent) {
      static constexpr mono_float kCoefficient0 = 1.0f;
      static constexpr mono_float kCoefficient1 = 16970.0f / 24483.0f;
      static constexpr mono_float kCoefficient2 = 1960.0f / 8161.0f;
      static constexpr mono_float kCoefficient3 = 1360.0f / 24483.0f;
      static constexpr mono_float kCoefficient4 = 80.0f / 8161.0f;
      static constexpr mono_float kCoefficient5 = 32.0f / 24483.0f;

      poly_int integer = utils::roundToInt(exponent);
      poly_float t = exponent - utils::toFloat(integer);
      poly_float int_pow = utils::pow2ToFloat(integer);

      poly_float cubic = t * (t * (t * kCoefficient5 + kCoefficient4) + kCoefficient3) + kCoefficient2;
      poly_float interpolate = t * (t * cubic + kCoefficient1) + kCoefficient0;
      return int_pow * interpolate;
    }

    force_inline poly_float log2(poly_float value) {
      static constexpr mono_float kCoefficient0 = -1819.0f / 651.0f;
      static constexpr mono_float kCoefficient1 = 5.0f;
      static constexpr mono_float kCoefficient2 = -10.0f / 3.0f;
      static constexpr mono_float kCoefficient3 = 10.0f / 7.0f;
      static constexpr mono_float kCoefficient4 = -1.0f / 3.0f;
      static constexpr mono_float kCoefficient5 = 1.0f / 31.0f;

      poly_int floored_log2 = utils::shiftRight<23>(utils::reinterpretToInt(value)) - 0x7f;
      poly_float t = (value & 0x7fffff) | (0x7f << 23);

      poly_float cubic = t * (t * (t * kCoefficient5 + kCoefficient4) + kCoefficient3) + kCoefficient2;
      poly_float interpolate = t * (t * cubic + kCoefficient1) + kCoefficient0;
      return utils::toFloat(floored_log2) + interpolate;
    }

    force_inline mono_float exp2(mono_float value) {
      poly_float input = value;
      poly_float result = exp2(input);
      return result[0];
    }

    force_inline mono_float log2(mono_float value) {
      poly_float input = value;
      poly_float result = log2(input);
      return result[0];
    }

    force_inline mono_float pow(mono_float base, mono_float exponent) {
      return exp2(log2(base) * exponent);
    }

    force_inline poly_float pow(poly_float base, poly_float exponent) {
      return exp2(log2(base) * exponent);
    }

    force_inline mono_float dbToMagnitude(mono_float decibels) {
      return exp2(decibels * kDbMagnitudeConversionMult);
    }

    force_inline poly_float dbToMagnitude(poly_float decibels) {
      return exp2(decibels * kDbMagnitudeConversionMult);
    }

    // Version of fast sin where phase is is [0, 1]
    force_inline mono_float quickSin1(mono_float phase) {
      phase = 0.5f - phase;
      return phase * (8.0f - 16.0f * fabsf(phase));
    }

    force_inline poly_float quickSin1(poly_float phase) {
      poly_float adjusted_phase = poly_float(0.5f) - phase;
      return adjusted_phase * poly_float::mulAdd(8.0f, poly_float::abs(adjusted_phase), -16.0f);
    }

    force_inline mono_float sin1(mono_float phase) {
      mono_float approx = quickSin1(phase);
      return approx * (0.776f + 0.224f * fabsf(approx));
    }

    force_inline poly_float sin1(poly_float phase) {
      poly_float approx = quickSin1(phase);
      return approx * poly_float::mulAdd(0.776f, poly_float::abs(approx), 0.224f);
    }

    force_inline poly_float equalPowerFade(poly_float t) {
      return sin1(t * 0.25f);
    }

    force_inline poly_float equalPowerFadeInverse(poly_float t) {
      return sin1((t + 1.0f) * 0.25f);
    }
  } // namespace futils
} // namespace vial
