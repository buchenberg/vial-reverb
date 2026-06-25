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

#include "matrix.h"
#include "utils.h"

#include <cmath>

namespace vial {

  namespace utils {
    force_inline poly_float mulAdd(poly_float a, poly_float b, poly_float c) {
      return poly_float::mulAdd(a, b, c);
    }

    force_inline poly_float interpolate(poly_float from, poly_float to, mono_float t) {
      return mulAdd(from, to - from, t);
    }

    force_inline poly_float interpolate(poly_float from, poly_float to, poly_float t) {
      return mulAdd(from, to - from, t);
    }

    force_inline poly_float interpolate(mono_float from, mono_float to, poly_float t) {
      return mulAdd(from, to - from, t);
    }

    force_inline poly_float clamp(poly_float value, mono_float min, mono_float max) {
      return poly_float::max(poly_float::min(value, max), min);
    }

    force_inline poly_float clamp(poly_float value, poly_float min, poly_float max) {
      return poly_float::max(poly_float::min(value, max), min);
    }

    force_inline poly_int clamp(poly_int value, poly_int min, poly_int max) {
      return poly_int::max(poly_int::min(value, max), min);
    }

    force_inline poly_float max(poly_float left, poly_float right) {
      return poly_float::max(left, right);
    }

    force_inline poly_float min(poly_float left, poly_float right) {
      return poly_float::min(left, right);
    }

    force_inline bool equal(poly_float left, poly_float right) {
      return poly_float::notEqual(left, right).sum() == 0;
    }

    force_inline poly_float maskLoad(poly_float zero_value, poly_float one_value, poly_mask reset_mask) {
      poly_float old_values = zero_value & ~reset_mask;
      poly_float new_values = one_value & reset_mask;

      return old_values + new_values;
    }

    force_inline poly_int maskLoad(poly_int zero_value, poly_int one_value, poly_mask reset_mask) {
      poly_int old_values = zero_value & ~reset_mask;
      poly_int new_values = one_value & reset_mask;

      return old_values | new_values;
    }

    force_inline poly_float pow(poly_float base, poly_float exponent) {
      poly_float result;
      int size = poly_float::kSize;
      for (int i = 0; i < size; ++i)
        result.set(i, powf(base[i], exponent[i]));
      return result;
    }

    force_inline poly_float swapStereo(poly_float value) {
    #if VITAL_AVX2
      return _mm256_shuffle_ps(value.value, value.value, _MM_SHUFFLE(2, 3, 0, 1));
    #elif VITAL_SSE2
      return _mm_shuffle_ps(value.value, value.value, _MM_SHUFFLE(2, 3, 0, 1));
    #elif VITAL_NEON
      return vrev64q_f32(value.value);
    #endif
    }

    force_inline poly_int swapStereo(poly_int value) {
    #if VITAL_AVX2
      return _mm256_shuffle_epi32(value.value, _MM_SHUFFLE(2, 3, 0, 1));
    #elif VITAL_SSE2
      return _mm_shuffle_epi32(value.value, _MM_SHUFFLE(2, 3, 0, 1));
    #elif VITAL_NEON
      return vrev64q_u32(value.value);
    #endif
    }

    force_inline poly_float swapVoices(poly_float value) {
    #if VITAL_AVX2
      return _mm256_shuffle_ps(value.value, value.value, _MM_SHUFFLE(1, 0, 3, 2));
    #elif VITAL_SSE2
      return _mm_shuffle_ps(value.value, value.value, _MM_SHUFFLE(1, 0, 3, 2));
    #elif VITAL_NEON
      return vextq_f32(value.value, value.value, 2);
    #endif
    }

    force_inline poly_int swapVoices(poly_int value) {
    #if VITAL_AVX2
      return _mm256_shuffle_epi32(value.value, _MM_SHUFFLE(1, 0, 3, 2));
    #elif VITAL_SSE2
      return _mm_shuffle_epi32(value.value, _MM_SHUFFLE(1, 0, 3, 2));
    #elif VITAL_NEON
      return vextq_u32(value.value, value.value, 2);
    #endif
    }

    force_inline void zeroBuffer(mono_float* buffer, int size) {
      for (int i = 0; i < size; ++i)
        buffer[i] = 0.0f;
    }

    force_inline void zeroBuffer(poly_float* buffer, int size) {
      for (int i = 0; i < size; ++i)
        buffer[i] = 0.0f;
    }

    force_inline bool isFinite(poly_float value) {
      for (int i = 0; i < poly_float::kSize; ++i) {
        mono_float val = value[i];
        if (!std::isfinite(val))
          return false;
      }
      return true;
    }

    force_inline bool isFinite(const poly_float* buffer, int size) {
      for (int i = 0; i < size; ++i) {
        if (!isFinite(buffer[i]))
          return false;
      }
      return true;
    }

    force_inline poly_float toPolyFloatFromUnaligned(const mono_float* unaligned) {
    #if VITAL_AVX2
      return _mm256_loadu_ps(unaligned);
    #elif VITAL_SSE2
      return _mm_loadu_ps(unaligned);
    #elif VITAL_NEON
      return vld1q_f32(unaligned);
    #endif
    }

    force_inline matrix getValueMatrix(const mono_float* buffer, poly_int indices) {
      return matrix(toPolyFloatFromUnaligned(buffer + indices[0]),
                    toPolyFloatFromUnaligned(buffer + indices[1]),
                    toPolyFloatFromUnaligned(buffer + indices[2]),
                    toPolyFloatFromUnaligned(buffer + indices[3]));
    }

    force_inline matrix getValueMatrix(const mono_float* const* buffers, poly_int indices) {
      return matrix(toPolyFloatFromUnaligned(buffers[0] + indices[0]),
                    toPolyFloatFromUnaligned(buffers[1] + indices[1]),
                    toPolyFloatFromUnaligned(buffers[2] + indices[2]),
                    toPolyFloatFromUnaligned(buffers[3] + indices[3]));
    }

    force_inline matrix getPolynomialInterpolationMatrix(poly_float t_from) {
      static constexpr mono_float kMultPrev = -1.0f / 6.0f;
      static constexpr mono_float kMultFrom = 1.0f / 2.0f;
      static constexpr mono_float kMultTo = -1.0f / 2.0f;
      static constexpr mono_float kMultNext = 1.0f / 6.0f;

      poly_float t_prev = t_from + 1.0f;
      poly_float t_to = t_from - 1.0f;
      poly_float t_next = t_from - 2.0f;

      poly_float t_prev_from = t_prev * t_from;
      poly_float t_to_next = t_to * t_next;

      return matrix(t_from * t_to_next * kMultPrev,
                    t_prev * t_to_next * kMultFrom,
                    t_prev_from * t_next * kMultTo,
                    t_prev_from * t_to * kMultNext);
    }

    force_inline matrix getCatmullInterpolationMatrix(poly_float t) {
      poly_float half_t = t * 0.5f;
      poly_float half_t2 = t * half_t;
      poly_float half_t3 = half_t2 * t;
      poly_float half_three_t3 = half_t3 * 3.0f;

      return matrix(half_t2 * 2.0f - half_t3 - half_t,
                    poly_float::mulSub(half_three_t3, half_t2, 5.0f) + 1.0f,
                    mulAdd(half_t, half_t2, 4.0f) - half_three_t3,
                    half_t3 - half_t2);
    }

    force_inline poly_float toFloat(poly_int integers) {
      VITAL_ASSERT(poly_float::kSize == poly_int::kSize);

    #if VITAL_AVX2
      return _mm256_cvtepi32_ps(integers.value);
    #elif VITAL_SSE2
      return _mm_cvtepi32_ps(integers.value);
    #elif VITAL_NEON
      return vcvtq_f32_s32(vreinterpretq_s32_u32(integers.value));
    #endif
    }

    force_inline poly_int toInt(poly_float floats) {
      VITAL_ASSERT(poly_float::kSize == poly_int::kSize);

    #if VITAL_AVX2
      return _mm256_cvtps_epi32(floats.value);
    #elif VITAL_SSE2
      return _mm_cvtps_epi32(floats.value);
    #elif VITAL_NEON
      return vreinterpretq_u32_s32(vcvtq_s32_f32(floats.value));
    #endif
    }

    force_inline poly_float floor(poly_float value) {
      poly_float truncated = toFloat(toInt(value));
      return truncated + (poly_float(-1.0f) & poly_float::greaterThan(truncated, value));
    }

    force_inline poly_int roundToInt(poly_float value) {
      return toInt(floor(value + 0.5f));
    }

    template<size_t shift>
    force_inline poly_int shiftRight(poly_int integer) {
    #if VITAL_AVX2
      return _mm256_srli_epi32(integers.value, shift);
    #elif VITAL_SSE2
      return _mm_srli_epi32(integer.value, shift);
    #elif VITAL_NEON
      return vshrq_n_u32(integer.value, shift);
    #endif
    }

    template<size_t shift>
    force_inline poly_int shiftLeft(poly_int integer) {
    #if VITAL_AVX2
      return _mm256_slli_epi32(integers.value, shift);
    #elif VITAL_SSE2
      return _mm_slli_epi32(integer.value, shift);
    #elif VITAL_NEON
      return vshlq_n_u32(integer.value, shift);
    #endif
    }

    force_inline poly_float reinterpretToFloat(poly_int value) {
    #if VITAL_AVX2
      return _mm256_castsi256_ps(value.value);
    #elif VITAL_SSE2
      return _mm_castsi128_ps(value.value);
    #elif VITAL_NEON
      return vreinterpretq_f32_u32(value.value);
    #endif
    }

    force_inline poly_int reinterpretToInt(poly_float value) {
    #if VITAL_AVX2
      return _mm256_castps_si256(value.value);
    #elif VITAL_SSE2
      return _mm_castps_si128(value.value);
    #elif VITAL_NEON
      return vreinterpretq_u32_f32(value.value);
    #endif
    }

    force_inline poly_float pow2ToFloat(poly_int value) {
      return reinterpretToFloat(shiftLeft<23>(value + 127));
    }

    force_inline poly_float midiCentsToFrequency(poly_float value) {
      poly_float result;
      for (int i = 0; i < poly_float::kSize; ++i)
        result.set(i, midiCentsToFrequency(value[i]));
      return result;
    }

    force_inline poly_float midiNoteToFrequency(poly_float value) {
      poly_float result;
      for (int i = 0; i < poly_float::kSize; ++i)
        result.set(i, midiNoteToFrequency(value[i]));
      return result;
    }

    force_inline poly_float sin(poly_float value) {
      poly_float result;
      for (int i = 0; i < poly_float::kSize; ++i)
        result.set(i, sinf(value[i]));
      return result;
    }

    force_inline poly_float cos(poly_float value) {
      poly_float result;
      for (int i = 0; i < poly_float::kSize; ++i)
        result.set(i, cosf(value[i]));
      return result;
    }

    force_inline poly_float tan(poly_float value) {
      poly_float result;
      for (int i = 0; i < poly_float::kSize; ++i)
        result.set(i, tanf(value[i]));
      return result;
    }

    force_inline poly_float dbToMagnitude(poly_float db) {
      poly_float result;
      for (int i = 0; i < poly_float::kSize; ++i)
        result.set(i, dbToMagnitude(db[i]));
      return result;
    }

    force_inline poly_float magnitudeToDb(poly_float magnitude) {
      poly_float result;
      for (int i = 0; i < poly_float::kSize; ++i)
        result.set(i, magnitudeToDb(magnitude[i]));
      return result;
    }

    force_inline poly_float sqrt(poly_float value) {
    #if VITAL_AVX2
      return _mm256_sqrt_ps(value.value);
    #elif VITAL_SSE2
      return _mm_sqrt_ps(value.value);
    #elif VITAL_NEON
      poly_float result;
      for (int i = 0; i < poly_float::kSize; ++i)
        result.set(i, sqrtf(value[i]));
      return result;
    #endif
    }
  } // namespace utils
} // namespace vial
