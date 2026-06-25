/* Vial Reverb — shared parameter specification.
 *
 * Single source of truth for every plugin parameter. Ranges, defaults and
 * value-scales are copied verbatim from vial's common/synth_parameters.cpp so
 * the extracted reverb behaves identically to the synth. Used by:
 *   - PluginProcessor   (to build the APVTS layout)
 *   - ReverbEngine     (to convert a raw parameter value into the value the
 *                        DSP Processor input expects)
 *   - the native tests  (to validate the scaling matches the JS UI)
 *
 * The reverb is always on — the DAW's built-in bypass handles muting.
 */
#pragma once

#include <array>
#include <cmath>
#include <string_view>

namespace vfx {

// Mirrors vial::ValueDetails::ValueScale (the subset the reverb uses).
enum class Scale { Linear, Indexed, Quadratic, Exponential, SquareRoot };

struct ParamSpec {
    const char* id;            // APVTS id == vial parameter name
    float min;
    float max;
    float defaultValue;
    float postOffset;          // post_offset column
    float displayMultiply;     // display_multiply column (UI only)
    Scale scale;
    bool displayInvert;        // display_invert column (UI only; engine ignores)
    const char* unit;          // display_units (UI only)
    const char* displayName;   // human label
    const char* description;   // tooltip description (UI only)
};

// ----------------------------------------------------------------------------
// Parameter table. Order is irrelevant; lookups are by id.
// Columns: id, min, max, default, postOffset, displayMultiply, scale, invert, unit, name, description
// ----------------------------------------------------------------------------
inline constexpr std::array<ParamSpec, 13> kParams { {
    // ---- Reverb (always on; DAW bypass handles muting) ----
    { "reverb_dry_wet",        0.0f,   1.0f,    0.25f,    0.0f, 100.0f, Scale::Linear,      false, "%",         "Reverb Mix",              "Balance between dry and wet signal" },
    { "reverb_decay_time",    -6.0f,   6.0f,    0.0f,     0.0f, 1.0f,   Scale::Exponential, false, " secs",     "Reverb Decay Time",       "How long the reverb tail lasts" },
    { "reverb_pre_low_cutoff", 0.0f, 128.0f,    0.0f,     0.0f, 1.0f,   Scale::Linear,      false, " semitones","Remove Lows",             "High-pass filter before reverb. Removes low frequencies from the reverb input" },
    { "reverb_pre_high_cutoff",0.0f, 128.0f,  110.0f,     0.0f, 1.0f,   Scale::Linear,      false, " semitones","Remove Highs",            "Low-pass filter before reverb. Removes high frequencies from the reverb input" },
    { "reverb_low_shelf_cutoff",0.0f, 128.0f,   0.0f,     0.0f, 1.0f,   Scale::Linear,      false, " semitones","Low Freq",                "Cutoff frequency for the low shelf EQ in the reverb tail" },
    { "reverb_low_shelf_gain",-6.0f,   0.0f,    0.0f,     0.0f, 1.0f,   Scale::Linear,      false, " dB",       "Low Gain",                "Gain adjustment for low frequencies in the reverb tail" },
    { "reverb_high_shelf_cutoff",0.0f,128.0f,  90.0f,     0.0f, 1.0f,   Scale::Linear,      false, " semitones","High Freq",               "Cutoff frequency for the high shelf EQ in the reverb tail" },
    { "reverb_high_shelf_gain",-6.0f,   0.0f,   -1.0f,     0.0f, 1.0f,   Scale::Linear,      false, " dB",       "High Gain",               "Gain adjustment for high frequencies in the reverb tail" },
    { "reverb_delay",          0.0f,   0.3f,    0.0f,     0.0f, 1.0f,   Scale::Linear,      false, " secs",     "Reverb Delay",            "Pre-delay before the reverb starts" },
    { "reverb_size",           0.0f,   1.0f,    0.5f,     0.0f, 100.0f, Scale::Linear,      false, "%",         "Reverb Size",             "Size of the virtual room" },
    { "reverb_chorus_amount",   0.0f,   1.0f,    0.223607f,0.0f, 100.0f, Scale::Quadratic,   false, "%",         "Reverb Chorus Amount",    "Modulation depth for chorus effect on the reverb tail" },
    { "reverb_chorus_frequency",-8.0f,  3.0f,   -2.0f,     0.0f, 1.0f,   Scale::Exponential, false, " Hz",       "Reverb Chorus Frequency", "Speed of the chorus modulation" },
    { "reverb_stereo_width",    0.0f,   1.0f,    1.0f,     0.0f, 100.0f, Scale::Linear,      false, "%",         "Reverb Stereo Width",     "Stereo spread of the reverb output" },
} };

inline const ParamSpec* findParam (const char* id) {
    for (const auto& p : kParams)
        if (std::string_view (p.id) == id)
            return &p;
    return nullptr; // caller must check
}

// Small local clamp so this header has no JUCE dependency (usable in tests).
inline float clampf (float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// Convert a RAW parameter value (already in [min,max]) into the engine value
// the vial Processor input expects. Mirrors the cr:: scale operators in
// operators.h that SynthModule::createMonoModControl inserts.
inline float toEngineValue (const ParamSpec& s, float raw) {
    switch (s.scale) {
        case Scale::Indexed:
            return std::round (raw);
        case Scale::Quadratic: {
            float v = raw < 0.0f ? 0.0f : raw;          // cr::Square clamps >= 0
            return v * v + s.postOffset;                // cr::Quadratic adds offset (0 here)
        }
        case Scale::Exponential: {
            float v = clampf (raw, s.min, s.max);
            return std::pow (2.0f, v);                   // cr::ExponentialScale(scale=2). Engine ignores displayInvert.
        }
        case Scale::SquareRoot: {
            float v = raw < 0.0f ? 0.0f : raw;
            return std::sqrt (v) + s.postOffset;
        }
        case Scale::Linear:
        default:
            return raw;
    }
}

} // namespace vfx
