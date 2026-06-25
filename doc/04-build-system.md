# Build System

## Overview

The project uses **CMake** as the build system, leveraging the **JUCE** framework for audio plugin development. The build system supports multiple targets and platforms.

## CMake Configuration

### Root CMakeLists.txt

The main `CMakeLists.txt` file defines:

1. **Project metadata**: Name, version, bundle identifier
2. **JUCE integration**: Fetches JUCE from GitHub and configures it
3. **Plugin targets**: VST3, AU, Standalone
4. **Test target**: Native unit tests
5. **UI build**: Embeds the built WebView UI into the plugin binary

### Key CMake Options

```cmake
# Plugin formats to build
FORMATS VST3 AU Standalone

# Plugin metadata
PLUGIN_CODE Vrv1
BUNDLE_ID com.buchenberg.vialreverb

# C++ standard
CMAKE_CXX_STANDARD 17
```

### Build Targets

The build system creates the following targets:

| Target | Description | Output |
|--------|-------------|--------|
| `VialReverb_VST3` | VST3 plugin | `VialReverb.vst3` |
| `VialReverb_AU` | Audio Unit plugin (macOS only) | `VialReverb.component` |
| `VialReverb_Standalone` | Standalone application | `VialReverb.app` (macOS) / `VialReverb.exe` (Windows) |
| `VialReverb_Tests` | Unit tests | `VialReverb_Tests` executable |

## Build Process

### 1. Configuration Phase

```bash
cmake -B build -G <generator>
```

CMake generates build files for the specified generator:
- **macOS**: `Xcode` or `Unix Makefiles`
- **Windows**: `Visual Studio 17 2022` (or other VS versions)

### 2. Build Phase

```bash
cmake --build build --config Release
```

This compiles:
- JUCE framework sources
- Plugin sources (`src/plugin/`)
- DSP sources (`src/dsp/`)
- UI assets (embedded as binary data)

### 3. UI Build Integration

The CMake build includes a custom command that:

1. Runs `npm install` in the `ui/` directory
2. Runs `npm run build` to create production UI assets
3. Embeds the built assets into the plugin binary using JUCE's `juce_add_binary_data()`

This ensures the UI is always up-to-date with the plugin binary.

## Platform-Specific Notes

### macOS

**Generators**: Xcode, Unix Makefiles, Ninja

**Architectures**: Universal binary (x86_64 + arm64)

**Code Signing**: The build system supports code signing via CMake variables:
```cmake
-DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY="Developer ID Application: ..."
```

**Notarization**: For distribution outside the Mac App Store, the standalone app and plugins must be notarized with Apple.

### Windows

**Generators**: Visual Studio 17 2022 (or 2019)

**Architecture**: x64 only (32-bit not supported)

**Runtime**: Links against MSVC runtime (static or dynamic based on CMake configuration)

**Installer**: Consider using Inno Setup or NSIS to create an installer that:
- Copies VST3 to `C:\Program Files\Common Files\VST3\`
- Creates Start Menu shortcuts for the standalone app
- Optionally installs the standalone app

## Dependencies

### JUCE Framework

JUCE is fetched automatically from GitHub during the CMake configuration phase:

```cmake
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.12
)
```

**Version**: 8.0.12 (or later 8.x versions)

**Modules Used**:
- `juce_audio_basics`: Core audio types and utilities
- `juce_audio_processors`: Plugin hosting and parameter management
- `juce_audio_formats`: Audio file I/O (for standalone)
- `juce_audio_utils`: Plugin editors and standalone runner
- `juce_gui_basics`: UI framework (for WebView hosting)
- `juce_gui_extra`: WebView component

### WebView Dependencies

The UI is built with:
- **Node.js 18+**: JavaScript runtime
- **npm**: Package manager
- **React 18**: UI framework
- **Vite**: Build tool and dev server
- **TypeScript**: Type-safe JavaScript
- **MUI (Material-UI)**: Component library

## Build Artifacts

After a successful build, artifacts are located in:

```
build/VialReverb_artefacts/Release/
├── VST3/
│   └── VialReverb.vst3
├── AU/
│   └── VialReverb.component (macOS only)
├── Standalone/
│   ├── VialReverb.app (macOS)
│   └── VialReverb.exe (Windows)
└── VialReverb_Tests (unit test executable)
```

## Debugging Builds

### Debug Configuration

Build with debug symbols:

```bash
cmake --build build --config Debug
```

This enables:
- Debug symbols for all targets
- No optimizations (easier to step through code)
- Assertions enabled in JUCE and DSP code

### IDE Integration

**Xcode (macOS)**:
```bash
cmake -B build -G Xcode
open build/VialReverb.xcodeproj
```

**Visual Studio (Windows)**:
```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
# Open build\VialReverb.sln in Visual Studio
```

**VS Code**:
Use the CMake Tools extension to configure, build, and debug directly from VS Code.

## Continuous Integration

The project can be integrated with CI systems (GitHub Actions, GitLab CI, etc.) using:

```yaml
# Example GitHub Actions workflow
- name: Build vial-reverb
  run: |
    cmake -B build -G "Unix Makefiles"
    cmake --build build --config Release
    ./build/VialReverb_artefacts/Release/VialReverb_Tests
```

## Troubleshooting

### Common Build Issues

#### 1. JUCE Fetch Fails

**Problem**: CMake cannot fetch JUCE from GitHub.

**Solution**: 
- Check internet connection
- Verify Git is installed and in PATH
- Manually clone JUCE and point CMake to it:
  ```cmake
  -DJUCE_PATH=/path/to/JUCE
  ```

#### 2. UI Build Fails

**Problem**: `npm install` or `npm run build` fails.

**Solution**:
- Ensure Node.js 18+ is installed
- Delete `ui/node_modules` and `ui/package-lock.json`, then retry
- Check `ui/package.json` for dependency issues

#### 3. Code Signing Errors (macOS)

**Problem**: Build fails with code signing errors.

**Solution**:
- Disable code signing for development:
  ```cmake
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY=""
  ```
- Or configure a valid signing identity

#### 4. Missing VST3 SDK

**Problem**: CMake cannot find the VST3 SDK.

**Solution**:
- JUCE includes the VST3 SDK automatically
- If issues persist, set `VST3_SDK_PATH` to the VST3 SDK location

#### 5. Linker Errors

**Problem**: Undefined symbols or linker errors.

**Solution**:
- Clean the build directory: `rm -rf build`
- Reconfigure and rebuild
- Check that all source files are listed in `CMakeLists.txt`

## Performance Optimization

### Release Build Flags

The Release configuration enables:
- `-O3` or `/O2`: Maximum optimization
- `-DNDEBUG`: Disables assertions
- Link-time optimization (LTO) for supported compilers

### Platform-Specific Optimizations

**macOS**:
- Universal binary (x86_64 + arm64)
- SIMD auto-detection (SSE2 on Intel, NEON on ARM)

**Windows**:
- x64 with AVX2 support (if available)
- Fast math optimizations

## Distribution

### macOS

1. **Notarize** the app and plugins with Apple
2. **Package** as a DMG or ZIP archive
3. **Include** installation instructions

### Windows

1. **Sign** the executables with a code signing certificate
2. **Create** an installer (Inno Setup, NSIS, or MSI)
3. **Include** Visual C++ Redistributable if using dynamic runtime

### Linux (Future)

While not currently supported, the JUCE framework supports Linux. To add Linux support:

1. Install JUCE dependencies: `sudo apt-get install libasound2-dev libx11-dev ...`
2. Build with: `cmake -B build -G "Unix Makefiles"`
3. Package as `.deb` or `.rpm`
