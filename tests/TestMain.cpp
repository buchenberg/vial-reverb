/* Vial Reverb — native tests (console app, JUCE UnitTest runner).
 *
 * Tier 1: pure parameter scaling + table invariants (vfx::toEngineValue, kParams).
 * Tier 2: ReverbEngine smoke/invariant tests (finite output, reset).
 */
#include <juce_audio_basics/juce_audio_basics.h>

#include <array>
#include <cmath>
#include <set>
#include <string>

#include "ReverbParameters.h"
#include "ReverbEngine.h"

using namespace vfx;

//==============================================================================
// Tier 1 — parameter scaling + table
//==============================================================================
class ParameterScalingTests : public juce::UnitTest {
public:
    ParameterScalingTests() : juce::UnitTest ("Parameter scaling", "vfx") {}

    void runTest() override {
        beginTest ("table invariants");
        {
            expectEquals ((int) kParams.size(), 13);

            std::set<std::string> ids;
            for (const auto& p : kParams) {
                expect (ids.insert (p.id).second, juce::String ("duplicate id: ") + p.id);
                expect (p.min <= p.max, juce::String ("min>max: ") + p.id);
                expect (p.defaultValue >= p.min && p.defaultValue <= p.max,
                        juce::String ("default out of range: ") + p.id);
            }
        }

        beginTest ("linear passthrough");
        {
            const auto* dw = findParam ("reverb_dry_wet"); // linear
            expect (dw != nullptr, "param not found");
            if (dw) {
                expectWithinAbsoluteError (toEngineValue (*dw, 0.5f), 0.5f, 1e-6f);
                expectWithinAbsoluteError (toEngineValue (*dw, 0.0f), 0.0f, 1e-6f);
                expectWithinAbsoluteError (toEngineValue (*dw, 1.0f), 1.0f, 1e-6f);
            }
        }

        beginTest ("exponential = 2^x, clamped to [min,max]");
        {
            const auto* decay = findParam ("reverb_decay_time"); // exp, -6..6
            expect (decay != nullptr, "param not found");
            if (decay) {
                expectWithinAbsoluteError (toEngineValue (*decay, 0.0f), 1.0f, 1e-5f);
                expectWithinAbsoluteError (toEngineValue (*decay, 6.0f), 64.0f, 1e-3f);
                expectWithinAbsoluteError (toEngineValue (*decay, -6.0f), 0.015625f, 1e-5f);
                expectWithinAbsoluteError (toEngineValue (*decay, 99.0f), 64.0f, 1e-3f);
            }

            const auto* freq = findParam ("reverb_chorus_frequency"); // exp, -8..3
            expect (freq != nullptr, "param not found");
            if (freq) {
                expectWithinAbsoluteError (toEngineValue (*freq, 0.0f), 1.0f, 1e-5f);
                expectWithinAbsoluteError (toEngineValue (*freq, 3.0f), 8.0f, 1e-4f);
            }
        }

        beginTest ("quadratic = x^2 (clamped >= 0)");
        {
            const auto* amt = findParam ("reverb_chorus_amount"); // quadratic, post_offset 0
            expect (amt != nullptr, "param not found");
            if (amt) {
                expectWithinAbsoluteError (toEngineValue (*amt, 0.5f), 0.25f, 1e-6f);
                expectWithinAbsoluteError (toEngineValue (*amt, 1.0f), 1.0f, 1e-6f);
                expectWithinAbsoluteError (toEngineValue (*amt, -0.3f), 0.0f, 1e-6f);
            }
        }
    }
};

//==============================================================================
// Tier 2 — engine smoke / invariants
//==============================================================================
class EngineSmokeTests : public juce::UnitTest {
public:
    EngineSmokeTests() : juce::UnitTest ("Engine smoke", "vfx") {}

    static std::array<float, kParams.size()> defaults() {
        std::array<float, kParams.size()> raw {};
        for (size_t i = 0; i < kParams.size(); ++i)
            raw[i] = kParams[i].defaultValue;
        return raw;
    }

    static bool allFinite (const juce::AudioBuffer<float>& b) {
        for (int c = 0; c < b.getNumChannels(); ++c)
            for (int i = 0; i < b.getNumSamples(); ++i)
                if (! std::isfinite (b.getSample (c, i)))
                    return false;
        return true;
    }

    void fillNoise (juce::AudioBuffer<float>& b, juce::Random& r) {
        for (int c = 0; c < b.getNumChannels(); ++c)
            for (int i = 0; i < b.getNumSamples(); ++i)
                b.setSample (c, i, r.nextFloat() * 2.0f - 1.0f);
    }

    void runTest() override {
        const int blockSize = 512;
        juce::Random rng (1234);

        beginTest ("reverb always on: finite, bounded output");
        {
            ReverbEngine engine;
            engine.prepare (44100, blockSize);
            engine.setRawParameters (defaults());

            juce::AudioBuffer<float> buf (2, blockSize);
            for (int n = 0; n < 200; ++n) {
                fillNoise (buf, rng);
                engine.process (buf.getWritePointer (0), buf.getWritePointer (1), blockSize);
                expect (allFinite (buf), "non-finite sample produced");
                expect (buf.getMagnitude (0, blockSize) < 50.0f, "output exploded");
            }
        }

        beginTest ("reset clears tails toward silence");
        {
            ReverbEngine engine;
            engine.prepare (44100, blockSize);
            engine.setRawParameters (defaults());

            juce::AudioBuffer<float> buf (2, blockSize);
            fillNoise (buf, rng);
            engine.process (buf.getWritePointer (0), buf.getWritePointer (1), blockSize);

            engine.reset();

            for (int n = 0; n < 50; ++n) {
                buf.clear();
                engine.process (buf.getWritePointer (0), buf.getWritePointer (1), blockSize);
                expect (allFinite (buf), "non-finite after reset");
            }
            expect (buf.getMagnitude (0, blockSize) < 1e-3f, "tail did not decay after reset");
        }
    }
};

//==============================================================================
static ParameterScalingTests parameterScalingTests;
static EngineSmokeTests engineSmokeTests;

int main (int, char**) {
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);
    runner.runTestsInCategory ("vfx");

    int failures = 0, passes = 0;
    for (int i = 0; i < runner.getNumResults(); ++i) {
        const auto* r = runner.getResult (i);
        failures += r->failures;
        passes += r->passes;
        std::cout << "  [" << (r->failures == 0 ? "PASS" : "FAIL") << "] "
                  << r->unitTestName << " / " << r->subcategoryName
                  << "  (" << r->passes << " checks)" << std::endl;
    }

    std::cout << (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED")
              << " — " << runner.getNumResults() << " sections, "
              << passes << " checks, " << failures << " failures" << std::endl;
    return failures == 0 ? 0 : 1;
}
