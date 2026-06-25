# Unrelated Code Notes

## Overview

The vial-reverb project extracts the reverb algorithm from the vial synthesizer, but includes the entire `src/dsp/vial/` directory from the original codebase. Much of this code is not directly used by the reverb plugin but is included for completeness and potential future use.

This document identifies code that is **not directly used** by the reverb plugin and explains why it's still present.

## Unused Framework Code

### 1. Processor Graph System

**Location**: `src/dsp/vial/framework/processor.h/cpp`

**What it does**: Provides a graph-based audio processing framework where `Processor` nodes can be connected in a tree structure. Includes features like:
- Modulation routing
- Polyphonic voice management
- Parameter automation
- Graph traversal and optimization

**Why it's not used**: The reverb plugin uses a simpler, direct audio processing model via `ReverbEngine`. It doesn't need the complex graph system designed for a full synthesizer with multiple oscillators, filters, and modulation sources.

**Files**:
- `processor.h` (11,356 chars)
- `processor.cpp` (7,267 chars)

**Used by**: Original vial synthesizer, not by vial-reverb

### 2. Operator System

**Location**: `src/dsp/vial/framework/operators.h/cpp`

**What it does**: Defines a large set of audio processing operators (oscillators, filters, envelopes, etc.) that can be used in the processor graph. Includes:
- `Oscillator`: Various waveform generators
- `Filter`: State-variable, ladder, and other filter types
- `Envelope`: ADSR and other envelope generators
- `Effect`: Chorus, delay, distortion, etc.

**Why it's not used**: The reverb algorithm is self-contained in `vial::Reverb` and doesn't use the operator system. These operators are designed for building synthesizer voices, not for standalone effects.

**Files**:
- `operators.h` (14,607 chars)
- `operators.cpp` (14,133 chars)

**Used by**: Original vial synthesizer, not by vial-reverb

### 3. Poly Values and Modulation

**Location**: `src/dsp/vial/framework/poly_values.h`

**What it does**: Defines types for polyphonic parameter values and modulation routing. Supports:
- Per-voice parameter values
- Modulation matrix
- Voice allocation and management

**Why it's not used**: The reverb is a mono/stereo effect, not a polyphonic instrument. It doesn't need voice management or per-voice modulation.

**Files**:
- `poly_values.h` (partial, 200 lines shown)

**Used by**: Original vial synthesizer, not by vial-reverb

### 4. Synthesis Constants

**Location**: `src/dsp/vial/common/synth_constants.h`

**What it does**: Defines constants specific to the vial synthesizer, such as:
- Synced frequency ratios (for tempo-synced LFOs)
- Voice masks (for polyphonic processing)
- Modulation connection limits

**Why it's not used**: These constants are specific to the synthesizer's modulation system and voice architecture. The reverb doesn't use tempo sync or polyphonic voice masks.

**Files**:
- `synth_constants.h` (47 lines, 1,388 chars)

**Used by**: Original vial synthesizer, not by vial-reverb

## Partially Used Code

### 1. SIMD Types (poly_float)

**Location**: `src/dsp/vial/framework/poly_utils.h`, `src/dsp/vial/framework/common.h`

**What it does**: Defines SIMD-optimized vector types (`poly_float`, `poly_int`) that process 4 voices simultaneously using SSE2/NEON instructions.

**How it's used**: The reverb algorithm uses `poly_float` extensively for SIMD-optimized audio processing. However, it only uses the basic vector operations, not the polyphonic voice management features.

**Files**:
- `poly_utils.h` (352 lines, 11,380 chars)
- `common.h` (2,802 chars)

**Used by**: Both vial synthesizer and vial-reverb (but only basic SIMD operations)

### 2. Memory (Delay Lines)

**Location**: `src/dsp/vial/lookups/memory.h`

**What it does**: Implements delay line buffers with cubic interpolation. Supports:
- Variable delay times
- Stereo processing
- Efficient circular buffer management

**How it's used**: The reverb uses `Memory` and `StereoMemory` classes for its delay lines. However, the polyphonic features (processing multiple voices simultaneously) are not used.

**Files**:
- `memory.h` (156 lines, 5,459 chars)

**Used by**: Both vial synthesizer and vial-reverb

### 3. Filters

**Location**: `src/dsp/vial/filters/`

**What it does**: Implements various filter types:
- `OnePoleFilter`: Simple low-pass/high-pass filter
- State-variable filters
- Ladder filters

**How it's used**: The reverb uses `OnePoleFilter` for its pre-filtering and EQ stages. Other filter types are not used.

**Files**:
- `one_pole_filter.h` (81 lines, 3,030 chars)
- Other filter files (not used by reverb)

**Used by**: Both vial synthesizer and vial-reverb (but only OnePoleFilter)

### 4. Utility Functions

**Location**: `src/dsp/vial/framework/utils.h/cpp`, `src/dsp/vial/framework/futils.h`

**What it does**: Provides mathematical and signal processing utilities:
- Fast math functions (exp2, log2, sin, cos)
- Interpolation functions
- Clamping and masking operations
- PCM data conversion

**How it's used**: The reverb uses many utility functions for:
- Fast math in the DSP core
- Interpolation in delay lines
- Clamping to prevent instability

**Files**:
- `utils.h` (8,486 chars)
- `utils.cpp` (3,293 chars)
- `futils.h` (4,535 chars)

**Used by**: Both vial synthesizer and vial-reverb

## Why Keep Unused Code?

### 1. Dependency Management

The reverb algorithm depends on several framework components:
- SIMD types (`poly_float`)
- Delay lines (`Memory`)
- Filters (`OnePoleFilter`)
- Utility functions

Removing unused code would require carefully untangling these dependencies, which is error-prone and time-consuming.

### 2. Future Extensibility

Keeping the full framework allows for future enhancements:
- Adding more effects from the vial synthesizer
- Implementing a preset system using the processor graph
- Adding modulation capabilities

### 3. Code Provenance

The code is extracted from the vial synthesizer (GPL v3 licensed). Keeping it intact makes it easier to:
- Track changes from the upstream project
- Merge updates from vial
- Maintain proper attribution

### 4. Minimal Overhead

The unused code doesn't significantly impact:
- **Binary size**: Dead code elimination removes unused functions
- **Compile time**: Only files directly included are compiled
- **Runtime performance**: Unused code is not executed

## What Could Be Removed?

If binary size or compile time becomes a concern, the following could potentially be removed:

### Safe to Remove

1. **Processor Graph** (`processor.h/cpp`)
   - Not used by reverb
   - No dependencies from reverb code

2. **Operators** (`operators.h/cpp`)
   - Not used by reverb
   - No dependencies from reverb code

3. **Poly Values** (`poly_values.h`)
   - Not used by reverb
   - No dependencies from reverb code

4. **Unused Filters** (all except `one_pole_filter.h`)
   - Reverb only uses OnePoleFilter
   - Other filters can be removed

5. **Synth Constants** (`synth_constants.h`)
   - Only contains synthesizer-specific constants
   - Not used by reverb

### Must Keep

1. **SIMD Types** (`poly_float`, `poly_int`)
   - Used extensively in reverb DSP

2. **Memory** (`memory.h`)
   - Used for delay lines

3. **OnePoleFilter** (`one_pole_filter.h`)
   - Used for pre-filtering and EQ

4. **Utility Functions** (`utils.h/cpp`, `futils.h`)
   - Used throughout the DSP code

## Refactoring Considerations

If you want to remove unused code, consider:

### Option 1: Create a Minimal Subset

Create a new directory `src/dsp/vial-minimal/` with only the files needed by the reverb:
- Copy only used files
- Update include paths
- Test thoroughly

**Pros**: Cleaner codebase, faster compilation  
**Cons**: Harder to merge upstream changes, potential for bugs

### Option 2: Use Preprocessor Guards

Add `#ifdef VIAL_REVERB` guards around unused code:
- Keeps code in place but excludes it from compilation
- Easy to re-enable if needed

**Pros**: Easy to toggle, maintains code structure  
**Cons**: Clutters code with preprocessor directives

### Option 3: Keep as Is

Leave the code unchanged:
- Dead code elimination handles binary size
- Easier to maintain and update

**Pros**: Minimal effort, easy to extend  
**Cons**: Larger source tree, slightly slower compilation

## Recommendation

**Keep the code as is** for now. The benefits of maintaining the full framework outweigh the minimal overhead. If binary size or compile time becomes a significant issue in the future, consider Option 1 (minimal subset) with thorough testing.

## Related Documentation

- [Architecture Overview](01-architecture-overview.md) - High-level structure
- [DSP Engine](02-dsp-engine.md) - How the reverb actually works
- [Build System](04-build-system.md) - Compilation and dependencies
