# Quick Reference

## Build Commands

### macOS

```bash
# Configure
cmake -B build -G Xcode

# Build all targets
cmake --build build --config Release

# Build specific target
cmake --build build --config Release --target VialReverb_VST3
cmake --build build --config Release --target VialReverb_AU
cmake --build build --config Release --target VialReverb_Standalone

# Run tests
./build/VialReverb_artefacts/Release/VialReverb_Tests
```

### Windows

```powershell
# Configure
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build all targets
cmake --build build --config Release

# Build specific target
cmake --build build --config Release --target VialReverb_VST3
cmake --build build --config Release --target VialReverb_Standalone

# Run tests
.\build\VialReverb_artefacts\Release\VialReverb_Tests.exe
```

### UI Development

```bash
cd ui

# Install dependencies
npm install

# Development mode (hot reload)
npm run dev

# Production build
npm run build

# Run tests
npm test
```

## Parameter Quick Reference

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Dry/Wet | 0.0 - 1.0 | 0.25 | Mix ratio |
| Decay Time | -6.0 - 6.0 | 0.0 | 2^x seconds |
| Pre Low Cutoff | 0 - 128 | 0 | MIDI note |
| Pre High Cutoff | 0 - 128 | 110 | MIDI note |
| Low Shelf Cutoff | 0 - 128 | 0 | MIDI note |
| Low Shelf Gain | -6.0 - 0.0 | 0.0 | dB |
| High Shelf Cutoff | 0 - 128 | 90 | MIDI note |
| High Shelf Gain | -6.0 - 0.0 | -1.0 | dB |
| Delay | 0.0 - 0.3 | 0.0 | seconds |
| Size | 0.0 - 1.0 | 0.5 | ratio |
| Chorus Amount | 0.0 - 1.0 | 0.0 | ratio |
| Chorus Frequency | -8.0 - 3.0 | -3.0 | 2^x Hz |
| Stereo Width | 0.0 - 1.0 | 1.0 | ratio |

## Common Presets

### Small Room
```
Dry/Wet: 0.2
Decay Time: -2.0 (0.25s)
Pre Low Cutoff: 30
Pre High Cutoff: 120
Size: 0.2
Chorus Amount: 0.0
Stereo Width: 0.8
```

### Large Hall
```
Dry/Wet: 0.4
Decay Time: 3.0 (8s)
Pre Low Cutoff: 20
Pre High Cutoff: 100
Low Shelf Gain: -3.0
High Shelf Gain: -2.0
Delay: 0.05
Size: 0.9
Chorus Amount: 0.2
Chorus Frequency: -3.0
Stereo Width: 1.0
```

### Plate Reverb
```
Dry/Wet: 0.3
Decay Time: 1.0 (2s)
Pre Low Cutoff: 40
Pre High Cutoff: 110
Low Shelf Cutoff: 60
Low Shelf Gain: -2.0
High Shelf Cutoff: 100
High Shelf Gain: -1.0
Size: 0.5
Chorus Amount: 0.3
Chorus Frequency: -2.0
Stereo Width: 1.0
```

### Ambient Wash
```
Dry/Wet: 0.8
Decay Time: 5.0 (32s)
Pre Low Cutoff: 10
Pre High Cutoff: 90
Low Shelf Gain: -4.0
High Shelf Gain: -3.0
Delay: 0.2
Size: 1.0
Chorus Amount: 0.5
Chorus Frequency: -4.0
Stereo Width: 1.0
```

### Vocal Reverb
```
Dry/Wet: 0.25
Decay Time: 0.5 (1.4s)
Pre Low Cutoff: 50
Pre High Cutoff: 100
Low Shelf Gain: -2.0
High Shelf Gain: -1.5
Delay: 0.03
Size: 0.4
Chorus Amount: 0.1
Stereo Width: 0.9
```

### Drum Room
```
Dry/Wet: 0.3
Decay Time: -1.0 (0.5s)
Pre Low Cutoff: 60
Pre High Cutoff: 115
Low Shelf Gain: -3.0
High Shelf Gain: -2.0
Size: 0.3
Chorus Amount: 0.0
Stereo Width: 0.7
```

## File Locations

### Source Code
```
src/
├── plugin/              # JUCE wrapper
│   ├── PluginProcessor.cpp/h
│   └── PluginEditor.cpp/h
└── dsp/                 # DSP engine
    ├── ReverbEngine.cpp/h
    ├── ReverbParameters.h
    └── vial/            # Core DSP (from vial synth)
```

### Build Artifacts
```
build/VialReverb_artefacts/Release/
├── VST3/VialReverb.vst3
├── AU/VialReverb.component (macOS)
├── Standalone/VialReverb.app (macOS)
└── VialReverb_Tests
```

### Installation Paths

**macOS**:
- VST3: `~/Library/Audio/Plug-Ins/VST3/`
- AU: `~/Library/Audio/Plug-Ins/Components/`

**Windows**:
- VST3: `C:\Program Files\Common Files\VST3\`

## Troubleshooting

### Plugin Not Showing in DAW

1. **Check installation path** - Verify plugin is in correct folder
2. **Rescan plugins** - Use DAW's plugin manager to rescan
3. **Check compatibility** - Ensure correct format (VST3/AU) and architecture (64-bit)

### UI Not Loading

1. **Check WebView dependencies** - Windows: WebView2, macOS: WebKit
2. **Rebuild plugin** - UI assets may not be embedded correctly
3. **Check console** - Look for JavaScript errors in WebView console

### Audio Issues

1. **Clicks/Pops** - Check buffer size (increase if too small)
2. **No Sound** - Verify dry/wet > 0, check pre-filter settings
3. **Distortion** - Reduce decay time, check EQ settings

### Build Issues

1. **JUCE not found** - Run `cmake -B build` to fetch JUCE
2. **UI build fails** - Delete `ui/node_modules`, run `npm install`
3. **Linker errors** - Clean build directory, reconfigure

## Performance Tips

### CPU Optimization

1. **Reduce decay time** - Shorter tails use less CPU
2. **Lower sample rate** - 44.1kHz vs 96kHz = 50% less CPU
3. **Increase buffer size** - Reduces callback frequency

### Memory Optimization

1. **Use smaller size** - Reduces delay line memory
2. **Limit instances** - Each instance uses ~10MB RAM

### Latency Reduction

1. **Reduce buffer size** - Lower latency but higher CPU
2. **Disable pre-delay** - Set delay to 0.0

## MIDI CC Mapping

Suggested MIDI CC assignments:

| CC | Parameter |
|----|-----------|
| 16 | Dry/Wet |
| 17 | Decay Time |
| 18 | Pre Low Cutoff |
| 19 | Pre High Cutoff |
| 20 | Low Shelf Cutoff |
| 21 | Low Shelf Gain |
| 22 | High Shelf Cutoff |
| 23 | High Shelf Gain |
| 24 | Delay |
| 25 | Size |
| 26 | Chorus Amount |
| 27 | Chorus Frequency |
| 28 | Stereo Width |

## Keyboard Shortcuts (Standalone)

| Key | Action |
|-----|--------|
| `Cmd/Ctrl + Q` | Quit |
| `Cmd/Ctrl + ,` | Preferences |
| `F11` | Fullscreen |
| `Escape` | Exit fullscreen |

## Version Information

- **Plugin Version**: 1.0.0
- **JUCE Version**: 8.0.12
- **Build System**: CMake 3.22+
- **C++ Standard**: C++17
- **UI Framework**: React 18 + Vite

## Support

- **Documentation**: See `doc/` folder for detailed guides
- **Issues**: Report on GitHub
- **License**: GPL v3
