#include "PluginProcessor.h"
#include "PluginEditor.h"

using vfx::kParams;
using vfx::ParamSpec;
using vfx::Scale;

juce::AudioProcessorValueTreeState::ParameterLayout VialReverbProcessor::createLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    for (const auto& p : kParams) {
        juce::ParameterID id { p.id, 1 };
        juce::String name (p.displayName);

        juce::NormalisableRange<float> range (p.min, p.max);
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            id, name, range, p.defaultValue));
    }

    return { params.begin(), params.end() };
}

VialReverbProcessor::VialReverbProcessor()
    : AudioProcessor (BusesProperties()
          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createLayout()) {
    for (size_t i = 0; i < kParams.size(); ++i)
        paramPtrs[i] = apvts.getRawParameterValue (kParams[i].id);
}

bool VialReverbProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const {
    const auto& mainOut = layouts.getMainOutputChannelSet();
    const auto& mainIn = layouts.getMainInputChannelSet();
    if (mainOut != juce::AudioChannelSet::stereo())
        return false;
    return mainIn == mainOut;
}

void VialReverbProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
    engine.prepare ((int) sampleRate, samplesPerBlock);
}

void VialReverbProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    for (auto ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    if (buffer.getNumChannels() < 2 || numSamples == 0)
        return;

    // Push current parameter snapshot to the engine.
    std::array<float, kParams.size()> raw;
    for (size_t i = 0; i < kParams.size(); ++i)
        raw[i] = paramPtrs[i]->load();
    engine.setRawParameters (raw);

    engine.process (buffer.getWritePointer (0), buffer.getWritePointer (1), numSamples);
}

juce::AudioProcessorEditor* VialReverbProcessor::createEditor() {
    return new VialReverbEditor (*this);
}

void VialReverbProcessor::getStateInformation (juce::MemoryBlock& destData) {
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void VialReverbProcessor::setStateInformation (const void* data, int sizeInBytes) {
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new VialReverbProcessor();
}
