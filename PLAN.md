# Vial Reverb — Implementation Plan

> **For Hermes:** Use subagent-driven-development skill to implement this plan task-by-task.

**Goal:** Extract the reverb effect from vial-effects into a standalone JUCE plugin project (vial-reverb) with its own parameter table, DSP engine, web UI editor, and tests.

**Architecture:** Copy the vial-effects project structure as a template, then strip out chorus and delay — leaving only the reverb DSP, a simplified ReverbEngine, a reverb-only parameter table (14 params vs 34), and a tailored web UI editor. The project uses CMake + JUCE 8 + React/Vite web UI (same stack as vial-effects).

**Tech Stack:** C++17, JUCE 8 (vendored in third_party/JUCE), CMake 3.22+, React/Vite web UI, juce::WebSliderRelay parameter binding.

---

## Source Files to Copy from vial-effects

All under `~/Code/vial-effects/`:

| Destination | Source | Notes |
|---|---|---|
| `third_party/JUCE/` | `third_party/JUCE/` | Symlink or copy; same JUCE tree |
| `src/dsp/vial/` | `src/dsp/vial/` | Full vendored DSP framework + reverb effect (framework, effects, filters, lookups, common) |
| `src/plugin/` | `src/plugin/` | PluginProcessor + PluginEditor (will be rewritten for reverb-only) |
| `ui/` | `ui/` | React/Vite web UI (will be trimmed to reverb-only) |
| `tests/` | `tests/` | Test harness (will be updated for 14 params) |

## Files to DROP (not copy)

- `src/dsp/Chorus.h` / `Chorus.cpp` — chorus DSP
- `src/dsp/vial/effects/delay.h` / `delay.cpp` — delay DSP
- `src/dsp/EffectsEngine.h` / `EffectsEngine.cpp` — multi-effect engine (replaced by ReverbEngine)
- `src/dsp/EffectParameters.h` — full 34-param table (replaced by ReverbParameters.h)
- `installer/` — Windows WiX installer (not needed for macOS-first)

## Reverb Parameters (14 params)

Copied verbatim from vial-effects `EffectParameters.h`, just the reverb rows:

| id | min | max | default | scale | unit | name |
|---|---|---|---|---|---|---|
| `reverb_on` | 0 | 1 | 0 | Indexed | | Reverb Switch |
| `reverb_dry_wet` | 0 | 1 | 0.25 | Linear | % | Reverb Mix |
| `reverb_decay_time` | -6 | 6 | 0 | Exponential | secs | Reverb Decay Time |
| `reverb_pre_low_cutoff` | 0 | 128 | 0 | Linear | semitones | Reverb Pre Low Cutoff |
| `reverb_pre_high_cutoff` | 0 | 128 | 110 | Linear | semitones | Reverb Pre High Cutoff |
| `reverb_low_shelf_cutoff` | 0 | 128 | 0 | Linear | semitones | Reverb Low Cutoff |
| `reverb_low_shelf_gain` | -6 | 0 | 0 | Linear | dB | Reverb Low Gain |
| `reverb_high_shelf_cutoff` | 0 | 128 | 90 | Linear | semitones | Reverb High Cutoff |
| `reverb_high_shelf_gain` | -6 | 0 | -1 | Linear | dB | Reverb High Gain |
| `reverb_delay` | 0 | 0.3 | 0 | Linear | secs | Reverb Delay |
| `reverb_size` | 0 | 1 | 0.5 | Linear | % | Reverb Size |
| `reverb_chorus_amount` | 0 | 1 | 0.223607 | Quadratic | % | Reverb Chorus Amount |
| `reverb_chorus_frequency` | -8 | 3 | -2 | Exponential | Hz | Reverb Chorus Frequency |
| `reverb_stereo_width` | 0 | 1 | 1 | Linear | % | Reverb Stereo Width |

---

## Task 1: Scaffold Project and Copy Shared DSP

**Objective:** Create the project directory, copy vendored vial DSP framework + reverb source, and set up the JUCE symlink.

**Files:**
- Create: `~/Code/vial-reverb/` (project root)
- Copy: `src/dsp/vial/` (full directory — framework, effects/reverb, filters, lookups, common)
- Create: `third_party/` with JUCE symlink → `~/Code/Juce`

**Step 1: Create project structure**

```bash
mkdir -p ~/Code/vial-reverb/{src/{dsp,plugin},third_party,tests,ui}
```

**Step 2: Copy vendored vial DSP (framework + reverb only)**

```bash
# Copy the full vial DSP tree — includes framework, effects/reverb.h+reverb.cpp, filters, lookups, common
cp -R ~/Code/vial-effects/src/dsp/vial ~/Code/vial-reverb/src/dsp/vial

# Remove delay files — only reverb is needed
rm ~/Code/vial-reverb/src/dsp/vial/effects/delay.cpp
rm ~/Code/vial-reverb/src/dsp/vial/effects/delay.h
```

**Step 3: Symlink JUCE**

```bash
ln -s ~/Code/Juce ~/Code/vial-reverb/third_party/JUCE
```

**Verify:**

```bash
find ~/Code/vial-reverb/src/dsp/vial -type f | sort
# Should show: framework/*.cpp/h, effects/reverb.cpp/reverb.h, filters/one_pole_filter.h, lookups/memory.h, common/synth_constants.h
# NO delay.cpp/delay.h
```

---

## Task 2: Create ReverbParameters.h

**Objective:** Define the reverb-only parameter table (14 params) with the same scaling infrastructure as vial-effects' EffectParameters.h, but without chorus/delay rows.

**Files:**
- Create: `src/dsp/ReverbParameters.h`

**Step 1: Write the parameter header**

Copy the struct/enum/utility definitions from `EffectParameters.h` verbatim, then replace the `kParams` array with only the 14 reverb rows shown above. Keep `findParam()`, `clampf()`, `toEngineValue()` unchanged.

**Verify:**

```bash
grep -c '{ "' src/dsp/ReverbParameters.h
# Should print 14
```

---

## Task 3: Create ReverbEngine

**Objective:** Write a simplified DSP engine that owns only the vial::Reverb processor — no chorus or delay chain. Same pattern as EffectsEngine but single-effect.

**Files:**
- Create: `src/dsp/ReverbEngine.h`
- Create: `src/dsp/ReverbEngine.cpp`

**Step 1: Write ReverbEngine.h**

```cpp
#pragma once
#include <array>
#include <memory>
#include "ReverbParameters.h"
#include "reverb.h"

namespace vfx {

class ReverbEngine {
public:
    ReverbEngine();
    ~ReverbEngine();

    void prepare(int sampleRate, int maxBlockSize);
    void reset();

    void setRawParameters(const std::array<float, kParams.size()>& raw) { raw_ = raw; }
    void process(float* left, float* right, int numSamples);

private:
    struct Idx {
        int reverbOn, reverbDryWet, reverbDecayTime;
        int reverbPreLowCutoff, reverbPreHighCutoff;
        int reverbLowCutoff, reverbLowGain, reverbHighCutoff, reverbHighGain;
        int reverbChorusAmount, reverbChorusFrequency, reverbSize, reverbDelay;
    } idx_;

    int indexOf(const char* id) const;
    float raw(int i) const { return raw_[i]; }
    float scaled(int i) const { return toEngineValue(kParams[i], raw_[i]); }

    std::unique_ptr<vial::Reverb> reverb_;
    vial::cr::Output reverbIn_[vial::Reverb::kNumInputs];
    vial::Output ioBuffer_;

    std::array<float, kParams.size()> raw_{};
    int sampleRate_ = 44100;
    bool reverbWasOn_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbEngine)
};

} // namespace vfx
```

**Step 2: Write ReverbEngine.cpp**

Same pattern as EffectsEngine.cpp but:
- Constructor: only resolves reverb `idx_` fields, creates `vial::Reverb`, plugs `reverbIn_` inputs
- `prepare()`: only calls `reverb_->setSampleRate()` — no delay or chorus
- `reset()`: only calls `reverb_->hardReset()`
- `process()`: only the reverb block from EffectsEngine::process() (lines 188-208 of original)

**Verify:** Compiles (tested in Task 6).

---

## Task 4: Create PluginProcessor and PluginEditor

**Objective:** Write reverb-only JUCE plugin wrapper — processor uses ReverbEngine + ReverbParameters, editor hosts WebView with WebSliderRelays for the 14 reverb params.

**Files:**
- Create: `src/plugin/PluginProcessor.h`
- Create: `src/plugin/PluginProcessor.cpp`
- Create: `src/plugin/PluginEditor.h`
- Create: `src/plugin/PluginEditor.cpp`

**Step 1: Write PluginProcessor**

Same structure as vial-effects' PluginProcessor but:
- Class name: `VialReverbProcessor`
- `getName()` returns `"Vial Reverb"`
- Uses `ReverbEngine` instead of `EffectsEngine`
- `kParams` comes from `ReverbParameters.h` (14 params)
- No `delay_style` or `*_sync` combo handling — all params are either bool (`reverb_on`) or float sliders
- `createLayout()`: only iterates reverb params; `reverb_on` becomes AudioParameterBool, rest are AudioParameterFloat
- No BPM/playhead handling (reverb has no tempo sync)

**Step 2: Write PluginEditor**

Same structure as vial-effects' PluginEditor but:
- Class name: `VialReverbEditor`
- Only iterates `kParams` from ReverbParameters.h (14 relays)
- `isOn()` check: `reverb_on` → toggle relay; `isCombo()` → always false (no combos in reverb-only)
- WebView data folder name: `VialReverbWebView`
- Resize: 800×600 min, 1200×900 initial

**Verify:** Compiles (tested in Task 6).

---

## Task 5: Write CMakeLists.txt

**Objective:** CMake build file for the reverb-only plugin targeting VST3 + AU + Standalone.

**Files:**
- Create: `CMakeLists.txt`

**Key differences from vial-effects' CMakeLists.txt:**
- Project name: `VialReverb`, version `0.1.0`
- `VIAL_DSP_SOURCES`: only `src/dsp/vial/framework/*.cpp` + `src/dsp/vial/effects/reverb.cpp` (no delay.cpp)
- `VFX_DSP_SOURCES`: only `src/dsp/ReverbEngine.cpp` (no Chorus.cpp, no EffectsEngine.cpp)
- `juce_add_plugin`: PRODUCT_NAME `"Vial Reverb"`, BUNDLE_ID `"com.buchenberg.vialreverb"`, PLUGIN_CODE `Vrv1`, VST3_CATEGORIES `Fx Reverb`
- AU_MAIN_TYPE: `kAudioUnitType_Effect`
- Same WebView binary data pattern for `ui/dist/`
- Same test target structure (console app, DSP-only, links juce_audio_basics + juce_dsp)

---

## Task 6: Build and Verify

**Objective:** Configure CMake, build all targets, run tests.

**Step 1: Configure**

```bash
cd ~/Code/vial-reverb
cmake -B build -G Xcode \
  -DCMAKE_BUILD_TYPE=Release \
  -DVIAL_JUCE_DIR=~/Code/Juce
```

**Step 2: Build plugin + tests**

```bash
cmake --build build --config Release --target VialReverb VialReverbTests
```

**Step 3: Run tests**

```bash
./build/VialReverb_artefacts/Release/VialReverbTests
```

**Step 4: Verify plugin bundles**

```bash
ls -la build/VialReverb_artefacts/Release/VST3/
ls -la build/VialReverb_artefacts/Release/AU/
```

**Step 5: Install to system**

```bash
cp -R "build/VialReverb_artefacts/Release/VST3/Vial Reverb.vst3" ~/Library/Audio/Plug-Ins/VST3/
cp -R "build/VialReverb_artefacts/Release/AU/Vial Reverb.component" ~/Library/Audio/Plug-Ins/Components/
```

---

## Task 7: Web UI (Stripped)

**Objective:** Create a minimal React/Vite web UI for the reverb-only plugin showing 14 parameter controls.

**Files:**
- Modify: `ui/src/` — strip chorus/delay panels, keep only reverb panel
- `ui/package.json`, `vite.config.ts` — same as vial-effects

**Approach:** Copy the full `ui/` directory from vial-effects, then strip out:
- Any chorus-related UI components and parameter bindings
- Any delay-related UI components and parameter bindings
- Update component to only render reverb controls

**Build:**

```bash
cd ~/Code/vial-reverb/ui
npm install --include=dev
npm run build
```

Then rebuild the CMake target to embed the new `ui/dist/` as BinaryData.

---

## Out of Scope (Future)

- Tempo sync / BPM (reverb has no tempo-dependent params)
- Preset system (can add later)
- Standalone window chrome customization
- Windows installer (WiX)
