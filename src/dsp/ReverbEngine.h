/* Vial Reverb — DSP engine.
 *
 * Owns the vial::Reverb processor extracted from vial. Parameters arrive as
 * RAW values (in each param's [min,max]); the engine converts them to engine
 * units (ReverbParameters.h) and writes them into the Processor's control-rate
 * inputs each block. The reverb is always on.
 */
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

    void prepare (int sampleRate, int maxBlockSize);
    void reset();

    void setRawParameters (const std::array<float, kParams.size()>& raw) { raw_ = raw; }

    void process (float* left, float* right, int numSamples);

private:
    // Pre-resolved indices into kParams / raw_ — resolved once in the constructor
    // so the audio thread never does string comparisons or hash lookups.
    struct Idx {
        int reverbDryWet, reverbDecayTime;
        int reverbPreLowCutoff, reverbPreHighCutoff;
        int reverbLowCutoff, reverbLowGain, reverbHighCutoff, reverbHighGain;
        int reverbChorusAmount, reverbChorusFrequency, reverbSize, reverbDelay;
    } idx_;

    int indexOf (const char* id) const;

    float raw (int i) const { return raw_[i]; }
    float scaled (int i) const { return toEngineValue (kParams[i], raw_[i]); }

    std::unique_ptr<vial::Reverb> reverb_;

    vial::cr::Output reverbIn_[vial::Reverb::kNumInputs];

    vial::Output ioBuffer_;

    std::array<float, kParams.size()> raw_ {};

    int sampleRate_ = 44100;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbEngine)
};

} // namespace vfx
