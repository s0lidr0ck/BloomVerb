#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>

#include "Parameters/BloomVerbParameters.h"

namespace
{
class ParameterContractTestProcessor final : public juce::AudioProcessor
{
public:
    ParameterContractTestProcessor()
        : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          apvts(*this, nullptr, "BloomVerbState", bloomverb::params::createParameterLayout())
    {
    }

    using AudioProcessor::processBlock;

    const juce::String getName() const override { return "ParameterContractTestProcessor"; }
    void prepareToPlay(double, int) override {}
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override { return true; }
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock& destData) override
    {
        if (auto state = apvts.copyState(); auto xml = state.createXml())
            copyXmlToBinary(*xml, destData);
    }
    void setStateInformation(const void* data, int sizeInBytes) override
    {
        if (const auto xml = getXmlFromBinary(data, sizeInBytes))
        {
            if (xml->hasTagName(apvts.state.getType()))
                apvts.replaceState(juce::ValueTree::fromXml(*xml));
        }
    }

    juce::AudioProcessorValueTreeState apvts;
};

bool expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }
    return true;
}

bool checkParameterPresence(ParameterContractTestProcessor& processor)
{
    using namespace bloomverb::params;

    const std::vector<juce::String> expectedIDs = {
        IDs::type, IDs::size, IDs::decaySeconds, IDs::preDelayMs, IDs::diffusion, IDs::damping, IDs::early,
        IDs::width, IDs::mix, IDs::outputDb, IDs::motion, IDs::dynamic, IDs::harmonic, IDs::warp, IDs::swell,
        IDs::texture, IDs::tone, IDs::lowCutHz, IDs::highCutHz, IDs::modRateHz, IDs::modDepth, IDs::duckAmount,
        IDs::bloomAmount, IDs::freeze, IDs::distance, IDs::transientPreserve
    };

    bool ok = true;
    ok &= expect(static_cast<int>(processor.apvts.processor.getParameters().size()) == static_cast<int>(expectedIDs.size()),
                 "Unexpected parameter count");

    for (const auto& id : expectedIDs)
        ok &= expect(processor.apvts.getParameter(id) != nullptr, "Missing parameter ID: " + id.toStdString());

    return ok;
}

bool checkStateRoundTrip(ParameterContractTestProcessor& a, ParameterContractTestProcessor& b)
{
    using namespace bloomverb::params;
    bool ok = true;

    auto* mix = a.apvts.getParameter(IDs::mix);
    auto* decay = a.apvts.getParameter(IDs::decaySeconds);
    auto* type = a.apvts.getParameter(IDs::type);
    auto* freeze = a.apvts.getParameter(IDs::freeze);
    auto* output = a.apvts.getParameter(IDs::outputDb);

    ok &= expect(mix != nullptr && decay != nullptr && type != nullptr && freeze != nullptr && output != nullptr,
                 "Expected core parameters not found");
    if (!ok)
        return false;

    mix->setValueNotifyingHost(0.77f);
    decay->setValueNotifyingHost(0.56f);
    type->setValueNotifyingHost(0.87f);
    freeze->setValueNotifyingHost(1.0f);
    output->setValueNotifyingHost(0.65f);

    const auto* aMix = a.apvts.getRawParameterValue(IDs::mix);
    const auto* aDecay = a.apvts.getRawParameterValue(IDs::decaySeconds);
    const auto* aType = a.apvts.getRawParameterValue(IDs::type);
    const auto* aFreeze = a.apvts.getRawParameterValue(IDs::freeze);
    const auto* aOutput = a.apvts.getRawParameterValue(IDs::outputDb);
    ok &= expect(aMix != nullptr && aDecay != nullptr && aType != nullptr && aFreeze != nullptr && aOutput != nullptr,
                 "Source parameter pointers missing");
    if (!ok)
        return false;

    juce::MemoryBlock state;
    a.getStateInformation(state);
    b.setStateInformation(state.getData(), static_cast<int>(state.getSize()));

    const auto* bMix = b.apvts.getRawParameterValue(IDs::mix);
    const auto* bDecay = b.apvts.getRawParameterValue(IDs::decaySeconds);
    const auto* bType = b.apvts.getRawParameterValue(IDs::type);
    const auto* bFreeze = b.apvts.getRawParameterValue(IDs::freeze);
    const auto* bOutput = b.apvts.getRawParameterValue(IDs::outputDb);

    ok &= expect(bMix != nullptr && bDecay != nullptr && bType != nullptr && bFreeze != nullptr && bOutput != nullptr,
                 "Round-trip parameter pointers missing");
    if (!ok)
        return false;

    ok &= expect(std::abs(bMix->load() - aMix->load()) < 0.0001f, "Mix round-trip mismatch");
    ok &= expect(std::abs(bDecay->load() - aDecay->load()) < 0.0001f, "Decay round-trip mismatch");
    ok &= expect(std::abs(bType->load() - aType->load()) < 0.0001f, "Type round-trip mismatch");
    ok &= expect(std::abs(bFreeze->load() - aFreeze->load()) < 0.0001f, "Freeze round-trip mismatch");
    ok &= expect(std::abs(bOutput->load() - aOutput->load()) < 0.0001f, "Output round-trip mismatch");

    return ok;
}
} // namespace

int main()
{
    ParameterContractTestProcessor processorA;
    ParameterContractTestProcessor processorB;

    bool ok = true;
    ok &= checkParameterPresence(processorA);
    ok &= checkStateRoundTrip(processorA, processorB);

    if (!ok)
        return 1;

    std::cout << "BloomVerbStateTests passed.\n";
    return 0;
}
