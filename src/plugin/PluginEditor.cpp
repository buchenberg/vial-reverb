#include "PluginEditor.h"
#include "BinaryData.h"

using vfx::kParams;

namespace {
juce::String mimeForExtension (const juce::String& path) {
    if (path.endsWith (".html")) return "text/html";
    if (path.endsWith (".js") || path.endsWith (".mjs")) return "text/javascript";
    if (path.endsWith (".css")) return "text/css";
    if (path.endsWith (".json") || path.endsWith (".map")) return "application/json";
    if (path.endsWith (".svg")) return "image/svg+xml";
    if (path.endsWith (".png")) return "image/png";
    if (path.endsWith (".woff2")) return "font/woff2";
    return "application/octet-stream";
}
} // namespace

VialReverbEditor::VialReverbEditor (VialReverbProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p) {
    using namespace juce;

    auto options = WebBrowserComponent::Options{}
                       .withBackend (WebBrowserComponent::Options::Backend::webview2)
                       .withWinWebView2Options (
                           WebBrowserComponent::Options::WinWebView2{}
                               .withUserDataFolder (juce::File::getSpecialLocation (juce::File::tempDirectory)
                                                        .getChildFile ("VialReverbWebView")))
                       .withNativeIntegrationEnabled()
                       .withResourceProvider ([this] (const auto& url) { return getResource (url); });

    // Create a slider relay per parameter (all params are sliders now).
    for (const auto& spec : kParams) {
        auto relay = std::make_unique<WebSliderRelay> (spec.id);
        options = options.withOptionsFrom (*relay);
        sliderRelays.push_back (std::move (relay));
    }

    webView = std::make_unique<WebBrowserComponent> (std::move (options));
    addAndMakeVisible (*webView);

    // Attach each relay to its APVTS parameter.
    size_t sliderIdx = 0;
    for (const auto& spec : kParams) {
        auto* param = processorRef.apvts.getParameter (spec.id);
        if (param == nullptr)
            continue;

        sliderAttachments.push_back (std::make_unique<WebSliderParameterAttachment> (
            *param, *sliderRelays[sliderIdx++], processorRef.apvts.undoManager));
    }

    webView->goToURL (WebBrowserComponent::getResourceProviderRoot());

    setResizable (true, true);
    setResizeLimits (500, 300, 1200, 800);
    setSize (720, 420);
}

VialReverbEditor::~VialReverbEditor() = default;

void VialReverbEditor::resized() {
    if (webView != nullptr)
        webView->setBounds (getLocalBounds());
}

std::optional<juce::WebBrowserComponent::Resource>
VialReverbEditor::getResource (const juce::String& url) const {
    auto path = url;
    if (path.startsWith ("/"))
        path = path.substring (1);
    if (path.isEmpty())
        path = "index.html";

    juce::String filename = path.fromLastOccurrenceOf ("/", false, false);
    if (filename.isEmpty())
        filename = "index.html";

    for (int i = 0; i < BinaryData::namedResourceListSize; ++i) {
        const char* resourceName = BinaryData::namedResourceList[i];
        if (juce::String (BinaryData::getNamedResourceOriginalFilename (resourceName)) == filename) {
            int dataSize = 0;
            const char* data = BinaryData::getNamedResource (resourceName, dataSize);
            std::vector<std::byte> bytes (reinterpret_cast<const std::byte*> (data),
                                          reinterpret_cast<const std::byte*> (data) + dataSize);
            return juce::WebBrowserComponent::Resource { std::move (bytes), mimeForExtension (filename) };
        }
    }
    return std::nullopt;
}
