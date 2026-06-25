# DSP Engine Details

## Overview

The DSP engine consists of two layers:

1. **ReverbEngine** (`src/dsp/ReverbEngine.cpp/h`): High-level wrapper that manages parameters and interfaces with the core DSP
2. **vial::Reverb** (`src/dsp/vial/effects/reverb.cpp/h`): The core reverb algorithm extracted from the vial synthesizer

## ReverbEngine

### Purpose

`ReverbEngine` serves as the bridge between the JUCE plugin layer and the low-level DSP. It handles:

- Parameter scaling and conversion
- Sample rate and buffer size management
- State initialization and reset
- Audio processing orchestration

### Key Methods

#### `prepare(int sampleRate, int blockSize)`

Initializes the engine for processing:

```cpp
void ReverbEngine::prepare(int sampleRate, int blockSize) {
    sample_rate_ = sampleRate;
    block_size_ = blockSize;
    reverb_.init();
    reverb_.setSampleRate(sampleRate);
    // ... allocate buffers
}
```

This must be called before any audio processing begins.

#### `setRawParameters(const std::array<float, kNumParams>& raw)`

Receives normalized parameter values (0.0-1.0) from the APVTS and converts them to engine values:

```cpp
void ReverbEngine::setRawParameters(const std::array<float, kNumParams>& raw) {
    for (size_t i = 0; i < kNumParams; ++i) {
        engine_values_[i] = toEngineValue(kParams[i], raw[i]);
    }
}
```

#### `process(float* left, float* right, int numSamples)`

Processes audio in-place:

```cpp
void ReverbEngine::process(float* left, float* right, int numSamples) {
    // Map parameters to vial::Reverb inputs
    reverb_.set(vial::Reverb::kWet, engine_values_[kDryWet]);
    reverb_.set(vial::Reverb::kDecayTime, engine_values_[kDecayTime]);
    // ... map all parameters
    
    // Process audio
    reverb_.processAudio(left, right, numSamples);
}
```

#### `reset()`

Clears all internal state (delay lines, filters):

```cpp
void ReverbEngine::reset() {
    reverb_.reset();
}
```

### Parameter Mapping

The engine maps the 13 user-facing parameters to the internal `vial::Reverb` inputs:

| User Parameter | vial::Reverb Input | Scaling |
|----------------|-------------------|---------|
| reverb_dry_wet | kWet | Linear (0-1) |
| reverb_decay_time | kDecayTime | Exponential (2^x) |
| reverb_pre_low_cutoff | kPreLowCutoff | Linear (MIDI note) |
| reverb_pre_high_cutoff | kPreHighCutoff | Linear (MIDI note) |
| reverb_low_shelf_cutoff | kLowShelfCutoff | Linear (MIDI note) |
| reverb_low_shelf_gain | kLowShelfGain | Linear (dB) |
| reverb_high_shelf_cutoff | kHighShelfCutoff | Linear (MIDI note) |
| reverb_high_shelf_gain | kHighShelfGain | Linear (dB) |
| reverb_delay | kDelay | Linear (seconds) |
| reverb_size | kSize | Linear (0-1) |
| reverb_chorus_amount | kChorusAmount | Quadratic (x²) |
| reverb_chorus_frequency | kChorusFrequency | Exponential (2^x) |
| reverb_stereo_width | kStereoWidth | Linear (0-1) |

## vial::Reverb

### Algorithm Overview

The core reverb algorithm is based on a network of **16 allpass filters** arranged in a specific topology. This design provides:

- Dense, smooth reverb tails
- Low computational cost
- Stable feedback behavior
- Natural-sounding decay

### Architecture

```
Input → Pre-Filter → Delay → Allpass Network → EQ → Output
              ↓                      ↑
              └─── Feedback Loop ────┘
```

#### 1. Pre-Filter Stage

Two one-pole filters (low and high cutoff) shape the input signal before it enters the reverb network:

```cpp
// From reverb.cpp
poly_float pre_filtered = pre_low_filter_.tick(input, low_coeff);
pre_filtered = pre_high_filter_.tick(pre_filtered, high_coeff);
```

These filters use the `OnePoleFilter` template class with SIMD-optimized coefficients.

#### 2. Delay Stage

A variable delay line implements the pre-delay parameter:

```cpp
poly_float delayed = delay_line_.get(delay_samples);
delay_line_.push(pre_filtered);
```

The delay line uses cubic interpolation for smooth parameter changes.

#### 3. Allpass Network

The heart of the reverb is a network of 16 allpass filters. Each allpass filter has the transfer function:

```
H(z) = (g + z⁻ⁿ) / (1 + g·z⁻ⁿ)
```

Where:
- `g` = feedback coefficient (determines decay)
- `n` = delay length in samples

The 16 allpass filters are arranged in a specific pattern to create dense reflections:

```cpp
// Simplified representation
for (int i = 0; i < 16; ++i) {
    poly_float delayed = allpass_delays_[i].get(allpass_lengths_[i]);
    poly_float feedback = delayed * decay_coeff;
    poly_float output = input + feedback;
    allpass_delays_[i].push(output);
    input = delayed - input * decay_coeff;
}
```

The delay lengths are carefully chosen to be mutually prime, creating a dense, non-repeating pattern.

#### 4. EQ Stage

Two shelf filters (low and high) shape the frequency response within the feedback loop:

```cpp
poly_float eq_output = low_shelf_.tick(allpass_output, low_shelf_coeff);
eq_output = high_shelf_.tick(eq_output, high_shelf_coeff);
```

This allows the reverb tail to be brighter or darker independent of the input signal.

#### 5. Chorus Modulation

A slow LFO modulates the delay line read positions, creating a chorus effect:

```cpp
poly_float modulated_delay = base_delay + chorus_lfo_.tick() * chorus_amount;
```

This adds movement and richness to the reverb tail.

#### 6. Stereo Processing

The reverb processes left and right channels independently but with shared parameters. The stereo width parameter controls the amount of cross-talk between channels:

```cpp
poly_float left_out = process_channel(left_in);
poly_float right_out = process_channel(right_in);
// Apply stereo width
output_left = left_out * width + right_out * (1 - width);
output_right = right_out * width + left_out * (1 - width);
```

### SIMD Optimization

The entire algorithm uses `poly_float` types that process 4 voices simultaneously:

```cpp
// poly_float is a SIMD vector (4 floats on SSE2/NEON)
poly_float input = load_stereo(left, right);
poly_float filtered = filter.tick(input, coeff);
```

This provides 4x throughput compared to scalar processing.

### Memory Management

The reverb uses several types of delay lines:

1. **Memory** (`vial::Memory`): General-purpose delay line with cubic interpolation
2. **StereoMemory** (`vial::StereoMemory`): Optimized for stereo processing
3. **AllpassDelay**: Specialized for allpass filter feedback

All delay lines use power-of-two sizes with bitmask wrapping for efficient indexing:

```cpp
int index = (offset_ - delay) & bitmask_;  // Fast modulo
```

### Performance Characteristics

- **CPU**: ~2-5% at 44.1kHz (varies by decay time and sample rate)
- **Memory**: ~10MB (mostly delay line buffers)
- **Latency**: 0 samples (no lookahead)
- **Thread Safety**: Safe for real-time audio thread (no allocations in process())

### Numerical Stability

The algorithm includes several safeguards:

1. **Feedback Clamping**: Decay coefficients are clamped to prevent instability
2. **DC Blocking**: High-pass filter removes DC offset from feedback loop
3. **Denormal Protection**: Small offset added to prevent denormal floats
4. **Buffer Clearing**: All buffers zeroed on reset to prevent clicks

## Testing

The DSP engine is tested via unit tests in `tests/TestMain.cpp`:

### Tier 1: Parameter Scaling Tests

- Verifies `toEngineValue()` produces correct output for all scaling types
- Tests boundary conditions (min, max, default values)
- Validates parameter table invariants (13 params, unique IDs, valid ranges)

### Tier 2: Engine Smoke Tests

- Processes noise through the engine and verifies finite output
- Tests reset behavior (tail decays to silence)
- Validates no NaN/Inf values are produced

## Debugging Tips

### Common Issues

1. **Clicks/Pops on Parameter Change**
   - Check that parameter smoothing is enabled
   - Verify `toEngineValue()` doesn't produce discontinuities

2. **Unstable Feedback (Exploding Output)**
   - Check decay time parameter scaling
   - Verify feedback coefficients are < 1.0
   - Ensure delay lines are properly initialized

3. **Silent Output**
   - Check dry/wet parameter (should be > 0)
   - Verify pre-filters aren't cutting all frequencies
   - Check EQ shelf gains aren't muting signal

### Profiling

Use JUCE's built-in profiling or external tools (Instruments on macOS, VTune on Windows) to identify bottlenecks. The main processing loop is in `vial::Reverb::processAudio()`.
