#include "ReverbEngine.h"

namespace vfx {

using vial::poly_float;
using vial::Reverb;

int ReverbEngine::indexOf (const char* id) const {
    for (int i = 0; i < (int) kParams.size(); ++i)
        if (std::string_view (kParams[i].id) == id)
            return i;
    jassertfalse; // unknown parameter id
    return 0;
}

ReverbEngine::ReverbEngine() {
    // Resolve all parameter indices once at construction time.
    idx_.reverbDryWet          = indexOf ("reverb_dry_wet");
    idx_.reverbDecayTime       = indexOf ("reverb_decay_time");
    idx_.reverbPreLowCutoff    = indexOf ("reverb_pre_low_cutoff");
    idx_.reverbPreHighCutoff   = indexOf ("reverb_pre_high_cutoff");
    idx_.reverbLowCutoff       = indexOf ("reverb_low_shelf_cutoff");
    idx_.reverbLowGain         = indexOf ("reverb_low_shelf_gain");
    idx_.reverbHighCutoff      = indexOf ("reverb_high_shelf_cutoff");
    idx_.reverbHighGain        = indexOf ("reverb_high_shelf_gain");
    idx_.reverbChorusAmount    = indexOf ("reverb_chorus_amount");
    idx_.reverbChorusFrequency = indexOf ("reverb_chorus_frequency");
    idx_.reverbSize            = indexOf ("reverb_size");
    idx_.reverbDelay           = indexOf ("reverb_delay");

    for (const auto& p : kParams)
        raw_[indexOf (p.id)] = p.defaultValue;

    reverb_ = std::make_unique<Reverb>();

    for (int i = 1; i < Reverb::kNumInputs; ++i)
        reverb_->plug (&reverbIn_[i], i);
}

ReverbEngine::~ReverbEngine() = default;

void ReverbEngine::prepare (int sampleRate, int /*maxBlockSize*/) {
    sampleRate_ = sampleRate;
    reverb_->setSampleRate (sampleRate);
    reset();
}

void ReverbEngine::reset() {
    reverb_->hardReset();
}

void ReverbEngine::process (float* left, float* right, int numSamples) {
    const int kBlock = vial::kMaxBufferSize;
    for (int offset = 0; offset < numSamples; offset += kBlock) {
        int n = std::min (kBlock, numSamples - offset);

        poly_float* io = ioBuffer_.buffer;
        for (int s = 0; s < n; ++s)
            io[s] = poly_float (left[offset + s], right[offset + s]);

        reverbIn_[Reverb::kWet].buffer[0]            = raw (idx_.reverbDryWet);
        reverbIn_[Reverb::kDecayTime].buffer[0]      = scaled (idx_.reverbDecayTime);
        reverbIn_[Reverb::kPreLowCutoff].buffer[0]   = raw (idx_.reverbPreLowCutoff);
        reverbIn_[Reverb::kPreHighCutoff].buffer[0]  = raw (idx_.reverbPreHighCutoff);
        reverbIn_[Reverb::kLowCutoff].buffer[0]      = raw (idx_.reverbLowCutoff);
        reverbIn_[Reverb::kLowGain].buffer[0]        = raw (idx_.reverbLowGain);
        reverbIn_[Reverb::kHighCutoff].buffer[0]     = raw (idx_.reverbHighCutoff);
        reverbIn_[Reverb::kHighGain].buffer[0]       = raw (idx_.reverbHighGain);
        reverbIn_[Reverb::kChorusAmount].buffer[0]   = scaled (idx_.reverbChorusAmount);
        reverbIn_[Reverb::kChorusFrequency].buffer[0]= scaled (idx_.reverbChorusFrequency);
        reverbIn_[Reverb::kSize].buffer[0]           = raw (idx_.reverbSize);
        reverbIn_[Reverb::kDelay].buffer[0]          = raw (idx_.reverbDelay);
        reverb_->processWithInput (io, n);

        const poly_float* result = reverb_->output()->buffer;
        for (int s = 0; s < n; ++s) {
            left[offset + s]  = result[s][0];
            right[offset + s] = result[s][1];
        }
    }
}

} // namespace vfx
