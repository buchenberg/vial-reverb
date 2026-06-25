# Architecture Overview

## High-Level Structure

The vial-reverb plugin is organized into three main layers:

```
┌─────────────────────────────────────────────────────────────┐
│                    JUCE Plugin Wrapper                      │
│  (PluginProcessor.cpp/h, PluginEditor.cpp/h)                │
│  - DAW integration (AU/VST3/Standalone)                     │
│  - Parameter management (APVTS)                             │
│  - UI hosting (WebView)                                     │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                    Reverb Engine Layer                      │
│  (ReverbEngine.cpp/h, ReverbParameters.h)                   │
│  - Parameter scaling and mapping                            │
│  - High-level DSP control                                   │
│  - State management                                         │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                    DSP Core (vial namespace)                │
│  (src/dsp/vial/)                                            │
│  - SIMD-optimized poly_float types (SSE2/NEON)              │
│  - Reverb algorithm (16 allpass filters)                    │
│  - Framework components (Processor, Operator, Memory)       │
│  - Filters, delays, modulation                              │
└─────────────────────────────────────────────────────────────┘
```

## Data Flow

### Audio Processing Path

1. **DAW → PluginProcessor**: Audio buffers arrive via `processBlock()`
2. **Parameter Sync**: Current parameter values are read from APVTS
3. **ReverbEngine::process()**: 
   - Converts raw parameter values to engine values using `toEngineValue()`
   - Maps parameters to `vial::Reverb` inputs
   - Calls `vial::Reverb::process()` for DSP
4. **vial::Reverb**: Executes the SIMD-optimized reverb algorithm
5. **Output → DAW**: Processed audio returned to DAW

### Parameter Flow

```
User UI → JUCE Parameter → APVTS → ReverbEngine::setRawParameters()
                                          ↓
                                   toEngineValue() scaling
                                          ↓
                                   vial::Reverb input mapping
```

## Key Design Decisions

### 1. Always-On Processing

The reverb has no internal bypass toggle. This simplifies the architecture:
- No click/pop from bypass switching
- DAW's built-in bypass handles muting
- Consistent behavior across hosts

### 2. Parameter Scaling Mirrors Original

The `toEngineValue()` function in `ReverbParameters.h` uses the same scaling algorithms as the original vial synthesizer:
- **Linear**: Direct passthrough (dry/wet, size, stereo width)
- **Exponential**: 2^x scaling (decay time, chorus frequency)
- **Quadratic**: x² scaling (chorus amount)
- **Indexed**: Discrete value lookup (not used in current params)
- **SquareRoot**: √x scaling (not used in current params)

This ensures identical behavior to the original synth.

### 3. SIMD-First DSP

The entire DSP core uses `poly_float` types that process 4 voices simultaneously using SIMD instructions:
- **SSE2** on x86/x64 (Intel/AMD)
- **NEON** on ARM (Apple Silicon, mobile)

This provides significant performance gains without sacrificing code readability.

### 4. WebView UI

The UI is built with React + Vite and hosted in a JUCE WebView:
- Modern web technologies (TypeScript, React, MUI)
- Hot reload during development
- Platform-native look and feel
- Easy to iterate on UI design

## File Organization

```
vial-reverb/
├── CMakeLists.txt              # Build configuration
├── README.md                   # High-level documentation
├── doc/                        # Detailed documentation (this folder)
├── src/
│   ├── plugin/                 # JUCE wrapper layer
│   │   ├── PluginProcessor.cpp/h
│   │   └── PluginEditor.cpp/h
│   └── dsp/                    # DSP layer
│       ├── ReverbEngine.cpp/h  # High-level engine
│       ├── ReverbParameters.h  # Parameter definitions
│       └── vial/               # Core DSP (from vial synth)
│           ├── effects/        # Reverb algorithm
│           ├── framework/      # Processor graph, SIMD types
│           ├── filters/        # One-pole, state-variable filters
│           ├── lookups/        # Memory (delay lines)
│           └── common/         # Constants, types
├── tests/                      # Unit tests
│   └── TestMain.cpp
└── ui/                         # WebView UI (React + Vite)
    ├── src/
    │   ├── components/         # React components
    │   ├── juce/               # JUCE interop bindings
    │   └── ...
    └── package.json
```

## Dependencies

### Build-Time
- **CMake 3.22+**: Build system
- **JUCE 8.x**: Audio plugin framework
- **C++17**: Language standard

### Runtime
- **WebView2** (Windows): For UI rendering
- **WebKit** (macOS): For UI rendering

### Development
- **Node.js 18+**: For UI development
- **npm**: Package manager for UI dependencies

## Performance Characteristics

- **CPU Usage**: ~2-5% per instance at 44.1kHz (varies by decay time)
- **Memory**: ~10MB per instance (mostly delay line buffers)
- **Latency**: 0 samples (no lookahead or buffering)
- **Thread Safety**: Safe for real-time audio thread

## Extensibility

The architecture supports several extension points:

1. **Adding Parameters**: Define in `ReverbParameters.h`, map in `ReverbEngine.cpp`
2. **Custom DSP**: Extend `vial::Processor` or `vial::Operator` classes
3. **UI Components**: Add React components in `ui/src/components/`
4. **New Algorithms**: Replace or augment `vial::Reverb` implementation

## Future Improvements

Potential areas for enhancement:

1. **Preset System**: Save/load parameter presets
2. **MIDI Learn**: Map parameters to MIDI controllers
3. **Automation Curves**: Custom parameter scaling curves
4. **Multi-Band Reverb**: Split frequency bands for independent processing
5. **Convolution Hybrid**: Combine algorithmic and impulse response reverb
6. **GPU Acceleration**: Offload DSP to GPU for complex configurations
