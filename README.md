# Vial Reverb

A high-quality algorithmic reverb audio plugin extracted from the [vial synthesizer](https://github.com/buchenberg/vial) (a fork of [Vital](https://github.com/mtytel/vital) by Matt Tytel). This plugin implements a sophisticated reverb algorithm based on a network of 16 allpass filters with feedback delay lines, featuring:

- 13 continuously variable parameters
- SIMD-optimized DSP using poly_float (SSE2/NEON)
- Stereo processing with independent left/right channels
- Pre-filtering (low/high cutoff) before the reverb network
- Internal EQ (low/high shelf) within the feedback loop
- Modulation chorus effect for lush tails
- WebView-based UI (React + Vite)

## Features

- **Formats**: VST3, AU, Standalone
- **Platforms**: macOS (Intel + Apple Silicon), Windows (x64)
- **Always-on processing**: No bypass toggle — use your DAW's built-in bypass
- **Parameter scaling**: Matches the original vial synthesizer behavior exactly

## Building from Source

### Prerequisites

**All platforms:**
- CMake 3.22 or higher
- C++17 compatible compiler
- Git

**macOS:**
- Xcode 13 or later (includes clang)
- macOS 10.13 or later

**Windows:**
- Visual Studio 2019 or later with C++ desktop development tools
- Windows 10 or later

### Clone the Repository

```bash
git clone https://github.com/buchenberg/vial-reverb.git
cd vial-reverb
```

### Build Instructions

#### macOS

```bash
# Configure the project
cmake -B build -G Xcode

# Build all targets (VST3, AU, Standalone)
cmake --build build --config Release

# Or build specific targets:
cmake --build build --config Release --target VialReverb_VST3
cmake --build build --config Release --target VialReverb_AU
cmake --build build --config Release --target VialReverb_Standalone
```

The built plugins will be in:
- **VST3**: `build/VialReverb_artefacts/Release/VST3/VialReverb.vst3`
- **AU**: `build/VialReverb_artefacts/Release/AU/VialReverb.component`
- **Standalone**: `build/VialReverb_artefacts/Release/Standalone/VialReverb.app`

To install system-wide:
```bash
# Copy VST3 to your system VST3 folder
cp -R build/VialReverb_artefacts/Release/VST3/VialReverb.vst3 ~/Library/Audio/Plug-Ins/VST3/

# Copy AU to your system Components folder
cp -R build/VialReverb_artefacts/Release/AU/VialReverb.component ~/Library/Audio/Plug-Ins/Components/
```

#### Windows

```powershell
# Configure the project
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build all targets
cmake --build build --config Release

# Or build specific targets:
cmake --build build --config Release --target VialReverb_VST3
cmake --build build --config Release --target VialReverb_Standalone
```

The built plugins will be in:
- **VST3**: `build\VialReverb_artefacts\Release\VST3\VialReverb.vst3`
- **Standalone**: `build\VialReverb_artefacts\Release\Standalone\VialReverb.exe`

To install system-wide:
```powershell
# Copy VST3 to your system VST3 folder
Copy-Item -Recurse build\VialReverb_artefacts\Release\VST3\VialReverb.vst3 "$env:CommonProgramFiles\VST3\"
```

### Building the UI

The plugin uses a WebView-based UI built with React and Vite.

```bash
cd ui

# Install dependencies
npm install

# Development mode (hot reload)
npm run dev

# Production build
npm run build
```

The built UI assets are automatically embedded into the plugin binary during the CMake build process.

### Running Tests

The project includes unit tests for parameter scaling and DSP engine invariants:

```bash
# Build the test executable
cmake --build build --config Release --target VialReverb_Tests

# Run the tests
./build/VialReverb_artefacts/Release/VialReverb_Tests
```

Tests verify:
- Parameter table invariants (13 parameters, valid ranges, unique IDs)
- Parameter scaling functions (Linear, Exponential, Quadratic, etc.)
- DSP engine smoke tests (finite output, reset behavior)

## Parameters

The reverb exposes 13 parameters:

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Dry/Wet | 0.0 - 1.0 | 0.25 | Mix between dry and wet signal |
| Decay Time | -6.0 - 6.0 | 0.0 | Exponential scaling: 2^x seconds |
| Pre Low Cutoff | 0 - 128 | 0 | MIDI note (Hz) low cutoff before reverb |
| Pre High Cutoff | 0 - 128 | 110 | MIDI note (Hz) high cutoff before reverb |
| Low Shelf Cutoff | 0 - 128 | 0 | MIDI note (Hz) for internal low shelf |
| Low Shelf Gain | -6.0 - 0.0 | 0.0 | dB gain for low shelf |
| High Shelf Cutoff | 0 - 128 | 90 | MIDI note (Hz) for internal high shelf |
| High Shelf Gain | -6.0 - 0.0 | -1.0 | dB gain for high shelf |
| Delay | 0.0 - 0.3 | 0.0 | Pre-delay in seconds |
| Size | 0.0 - 1.0 | 0.5 | Reverb room size |
| Chorus Amount | 0.0 - 1.0 | 0.0 | Modulation depth |
| Chorus Frequency | -8.0 - 3.0 | -3.0 | Exponential scaling: 2^x Hz |
| Stereo Width | 0.0 - 1.0 | 1.0 | Stereo spread |

## Architecture

The plugin is structured in three layers:

1. **JUCE Plugin Wrapper** (`src/plugin/`): AudioProcessor and AudioProcessorEditor that handle DAW integration and parameter management
2. **Reverb Engine** (`src/dsp/`): High-level engine that maps parameters to the DSP core
3. **DSP Core** (`src/dsp/vial/`): The original vial reverb algorithm using SIMD-optimized poly_float types

See the [detailed documentation](doc/) for more information about the architecture and implementation.

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

The reverb algorithm is derived from the vial synthesizer by Matt Tytel, originally licensed under GPL v3.

## Credits

- **Original Algorithm**: Matt Tytel (vial/Vital synthesizer)
- **Plugin Extraction**: Gregory Buchenberger
- **Framework**: [JUCE](https://juce.com/) by Raw Material Software

## Contributing

Contributions are welcome! Please open an issue or pull request on GitHub.

## Known Issues

- The WebView UI is currently a placeholder implementation
- Some framework code in `src/dsp/vial/framework/` is not directly used by the reverb (see [Unrelated Code Notes](doc/06-unrelated-code.md))
