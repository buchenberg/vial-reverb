# Parameter Reference

## Overview

The vial-reverb plugin exposes 13 continuously variable parameters. All parameters are defined in `src/dsp/ReverbParameters.h` and managed via JUCE's `AudioProcessorValueTreeState` (APVTS).

## Parameter Table

| # | ID | Display Name | Min | Max | Default | Scaling | Unit |
|---|-----|-------------|-----|-----|---------|---------|------|
| 1 | `reverb_dry_wet` | Dry/Wet | 0.0 | 1.0 | 0.25 | Linear | ratio |
| 2 | `reverb_decay_time` | Decay Time | -6.0 | 6.0 | 0.0 | Exponential | seconds (2^x) |
| 3 | `reverb_pre_low_cutoff` | Pre Low Cutoff | 0 | 128 | 0 | Linear | MIDI note |
| 4 | `reverb_pre_high_cutoff` | Pre High Cutoff | 0 | 128 | 110 | Linear | MIDI note |
| 5 | `reverb_low_shelf_cutoff` | Low Shelf Cutoff | 0 | 128 | 0 | Linear | MIDI note |
| 6 | `reverb_low_shelf_gain` | Low Shelf Gain | -6.0 | 0.0 | 0.0 | Linear | dB |
| 7 | `reverb_high_shelf_cutoff` | High Shelf Cutoff | 0 | 128 | 90 | Linear | MIDI note |
| 8 | `reverb_high_shelf_gain` | High Shelf Gain | -6.0 | 0.0 | -1.0 | Linear | dB |
| 9 | `reverb_delay` | Delay | 0.0 | 0.3 | 0.0 | Linear | seconds |
| 10 | `reverb_size` | Size | 0.0 | 1.0 | 0.5 | Linear | ratio |
| 11 | `reverb_chorus_amount` | Chorus Amount | 0.0 | 1.0 | 0.0 | Quadratic | ratio |
| 12 | `reverb_chorus_frequency` | Chorus Frequency | -8.0 | 3.0 | -3.0 | Exponential | Hz (2^x) |
| 13 | `reverb_stereo_width` | Stereo Width | 0.0 | 1.0 | 1.0 | Linear | ratio |

## Scaling Functions

Parameters use different scaling curves to match the original vial synthesizer behavior. The `toEngineValue()` function in `ReverbParameters.h` handles this conversion.

### Linear

Direct passthrough with optional offset:

```cpp
engine_value = raw_value * (max - min) + min
```

Used for: dry/wet, cutoff frequencies, shelf gains, delay, size, stereo width.

### Exponential

Power-of-two scaling:

```cpp
engine_value = 2^raw_value
```

Used for: decay time, chorus frequency.

**Example**: Decay time parameter ranges from -6.0 to 6.0:
- raw = -6.0 → 2^(-6) = 0.015625 seconds (very short)
- raw = 0.0 → 2^0 = 1.0 second
- raw = 6.0 → 2^6 = 64 seconds (very long)

### Quadratic

Square scaling:

```cpp
engine_value = raw_value^2
```

Used for: chorus amount.

**Example**: Chorus amount ranges from 0.0 to 1.0:
- raw = 0.0 → 0.0^2 = 0.0 (no modulation)
- raw = 0.5 → 0.5^2 = 0.25 (subtle modulation)
- raw = 1.0 → 1.0^2 = 1.0 (maximum modulation)

The quadratic curve provides finer control at low values.

## Parameter Descriptions

### 1. Dry/Wet (reverb_dry_wet)

**Range**: 0.0 - 1.0  
**Default**: 0.25  
**Scaling**: Linear

Controls the mix between the dry (unprocessed) signal and the wet (reverberated) signal.

- **0.0**: 100% dry, no reverb
- **0.5**: Equal mix of dry and wet
- **1.0**: 100% wet, only reverb

**Tip**: For subtle ambience, use values between 0.1-0.3. For dramatic effects, use 0.5-1.0.

### 2. Decay Time (reverb_decay_time)

**Range**: -6.0 - 6.0  
**Default**: 0.0  
**Scaling**: Exponential (2^x seconds)

Controls how long the reverb tail lasts before fading to silence.

- **-6.0**: ~0.016 seconds (very short, almost no reverb)
- **0.0**: 1.0 second (medium room)
- **6.0**: 64 seconds (cathedral-like)

**Tip**: Use negative values for tight, percussive spaces. Use positive values for lush, ambient tails.

### 3. Pre Low Cutoff (reverb_pre_low_cutoff)

**Range**: 0 - 128 (MIDI note numbers)  
**Default**: 0  
**Scaling**: Linear

Low-frequency cutoff filter applied **before** the reverb network. Frequencies below this cutoff are attenuated.

- **0**: No cutoff (all frequencies pass)
- **60**: ~261 Hz (C4) - removes bass
- **128**: ~20 kHz (highest note) - removes almost everything

**Tip**: Use values 20-40 to remove muddy low frequencies before reverb, preventing boomy tails.

### 4. Pre High Cutoff (reverb_pre_high_cutoff)

**Range**: 0 - 128 (MIDI note numbers)  
**Default**: 110  
**Scaling**: Linear

High-frequency cutoff filter applied **before** the reverb network. Frequencies above this cutoff are attenuated.

- **0**: Removes all high frequencies (dark reverb)
- **110**: ~1.2 kHz (default, warm sound)
- **128**: No cutoff (bright reverb)

**Tip**: Lower values create darker, more vintage-sounding reverb. Higher values preserve brightness.

### 5. Low Shelf Cutoff (reverb_low_shelf_cutoff)

**Range**: 0 - 128 (MIDI note numbers)  
**Default**: 0  
**Scaling**: Linear

Cutoff frequency for the internal low shelf EQ, applied **within** the feedback loop.

- **0**: No low shelf effect
- **40**: ~130 Hz (boost/cut low frequencies in the tail)
- **80**: ~880 Hz (boost/cut lower midrange)

**Tip**: Combine with Low Shelf Gain to add warmth (positive gain) or clarity (negative gain) to the reverb tail.

### 6. Low Shelf Gain (reverb_low_shelf_gain)

**Range**: -6.0 - 0.0 dB  
**Default**: 0.0 dB  
**Scaling**: Linear

Gain applied to frequencies below the Low Shelf Cutoff.

- **0.0 dB**: No change (flat)
- **-3.0 dB**: Reduce low frequencies by 3 dB
- **-6.0 dB**: Reduce low frequencies by 6 dB

**Note**: Only negative values are allowed (cut only, no boost).

**Tip**: Use -2 to -4 dB to reduce muddiness in the reverb tail.

### 7. High Shelf Cutoff (reverb_high_shelf_cutoff)

**Range**: 0 - 128 (MIDI note numbers)  
**Default**: 90  
**Scaling**: Linear

Cutoff frequency for the internal high shelf EQ, applied **within** the feedback loop.

- **0**: Affects all frequencies
- **90**: ~1 kHz (default)
- **128**: No effect (highest frequency)

**Tip**: Lower values affect more of the frequency spectrum. Higher values target only the brightest frequencies.

### 8. High Shelf Gain (reverb_high_shelf_gain)

**Range**: -6.0 - 0.0 dB  
**Default**: -1.0 dB  
**Scaling**: Linear

Gain applied to frequencies above the High Shelf Cutoff.

- **0.0 dB**: No change (flat)
- **-1.0 dB**: Reduce high frequencies by 1 dB (default)
- **-6.0 dB**: Reduce high frequencies by 6 dB

**Note**: Only negative values are allowed (cut only, no boost).

**Tip**: The default -1.0 dB provides subtle darkening. Use -3 to -6 dB for very dark, smooth tails.

### 9. Delay (reverb_delay)

**Range**: 0.0 - 0.3 seconds  
**Default**: 0.0 seconds  
**Scaling**: Linear

Pre-delay before the reverb effect begins. Creates separation between the dry signal and the reverb onset.

- **0.0**: No pre-delay (reverb starts immediately)
- **0.05**: 50ms (subtle separation)
- **0.3**: 300ms (pronounced slap-back effect)

**Tip**: Use 20-80ms to add clarity to vocals and snare drums. Use 100-300ms for dramatic effects.

### 10. Size (reverb_size)

**Range**: 0.0 - 1.0  
**Default**: 0.5  
**Scaling**: Linear

Controls the perceived size of the reverb space by adjusting the internal delay line lengths.

- **0.0**: Very small (tiny room)
- **0.5**: Medium (default room size)
- **1.0**: Very large (cathedral, hall)

**Tip**: Smaller values work well for drums and percussion. Larger values work well for pads and atmospheric sounds.

### 11. Chorus Amount (reverb_chorus_amount)

**Range**: 0.0 - 1.0  
**Default**: 0.0  
**Scaling**: Quadratic (x²)

Depth of the chorus modulation applied to the reverb tail.

- **0.0**: No modulation (pure reverb)
- **0.5**: 0.25 (subtle movement)
- **1.0**: 1.0 (maximum modulation)

**Note**: The quadratic scaling provides fine control at low values.

**Tip**: Use 0.1-0.3 for subtle richness. Use 0.5+ for lush, modulated tails.

### 12. Chorus Frequency (reverb_chorus_frequency)

**Range**: -8.0 - 3.0  
**Default**: -3.0  
**Scaling**: Exponential (2^x Hz)

Speed of the chorus modulation LFO.

- **-8.0**: 2^(-8) = 0.0039 Hz (very slow)
- **-3.0**: 2^(-3) = 0.125 Hz (default, slow modulation)
- **3.0**: 2^3 = 8 Hz (fast modulation)

**Tip**: Slower rates (0.05-0.2 Hz) create subtle movement. Faster rates (1-4 Hz) create more pronounced chorus effects.

### 13. Stereo Width (reverb_stereo_width)

**Range**: 0.0 - 1.0  
**Default**: 1.0  
**Scaling**: Linear

Controls the stereo spread of the reverb output.

- **0.0**: Mono (no stereo separation)
- **0.5**: Narrow stereo
- **1.0**: Full stereo (default)

**Tip**: Use 0.0 for mono compatibility. Use 0.7-1.0 for wide stereo images.

## Parameter Interactions

Some parameters work together to shape the sound:

### Pre-Filtering (Pre Low/High Cutoff)

These two parameters shape the input signal before it enters the reverb:
- **Pre Low Cutoff**: Removes low frequencies (prevents boominess)
- **Pre High Cutoff**: Removes high frequencies (creates darker sound)

**Typical Setup**: Pre Low = 20-40, Pre High = 100-120

### Internal EQ (Shelf Cutoff/Gain)

These four parameters shape the reverb tail within the feedback loop:
- **Low Shelf Cutoff + Gain**: Controls low-frequency content in the tail
- **High Shelf Cutoff + Gain**: Controls high-frequency content in the tail

**Typical Setup**: Low Shelf Gain = -2 to -4 dB, High Shelf Gain = -1 to -3 dB

### Chorus (Amount + Frequency)

These two parameters control the modulation effect:
- **Chorus Amount**: How much modulation is applied
- **Chorus Frequency**: How fast the modulation occurs

**Typical Setup**: Amount = 0.1-0.3, Frequency = -4 to -2 (0.06-0.25 Hz)

## Automation

All parameters can be automated in your DAW. Parameter IDs are stable and can be referenced in automation lanes.

**Tip**: Automate Decay Time and Dry/Wet for dynamic reverb effects that change over time.

## MIDI Learn

The plugin supports MIDI learn for all parameters (if your DAW supports it). Right-click a parameter in your DAW's plugin interface and select "Learn MIDI CC" to map a MIDI controller.

## Presets

Currently, the plugin does not include a preset system. Presets can be implemented in a future version using JUCE's `AudioProcessor::getStateInformation()` and `setStateInformation()` methods.
