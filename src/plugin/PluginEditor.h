/* Vial Reverb — WebView editor.
 *
 * Hosts a juce::WebBrowserComponent that loads the React/Vite UI (bundled as
 * BinaryData). Every APVTS parameter is bound to the JS frontend with a JUCE 8
 * WebSliderRelay + parameter attachment.
 */
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

class VialReverbEditor final : public juce::AudioProcessorEditor {
public:
    explicit VialReverbEditor (VialReverbProcessor&);
    ~VialReverbEditor() override;

    void resized() override;

private:
    std::optional<juce::WebBrowserComponent::Resource> getResource (const juce::String& url) const;

    VialReverbProcessor& processorRef;

    std::vector<std::unique_ptr<juce::WebSliderRelay>> sliderRelays;
    std::vector<std::unique_ptr<juce::WebSliderParameterAttachment>> sliderAttachments;

    std::unique_ptr<juce::WebBrowserComponent> webView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VialReverbEditor)
};
