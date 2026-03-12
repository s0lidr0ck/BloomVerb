#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

BloomVerbAudioProcessor::BloomVerbAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "BloomVerbState", bloomverb::params::createParameterLayout())
{
}

void BloomVerbAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
}

void BloomVerbAudioProcessor::releaseResources()
{
    engine.reset();
}

bool BloomVerbAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void BloomVerbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const auto totalInputChannels = getTotalNumInputChannels();
    const auto totalOutputChannels = getTotalNumOutputChannels();

    for (auto channel = totalInputChannels; channel < totalOutputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    engine.process(buffer, readRuntimeParameters());
}

double BloomVerbAudioProcessor::getTailLengthSeconds() const
{
    return bloomverb::params::getValue(apvts, bloomverb::params::IDs::decaySeconds);
}

void BloomVerbAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (const auto state = apvts.copyState(); auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void BloomVerbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (const auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

bloomverb::RuntimeParameters BloomVerbAudioProcessor::readRuntimeParameters() const
{
    using namespace bloomverb::params;
    bloomverb::RuntimeParameters p;

    p.type = static_cast<int>(std::round(getValue(apvts, IDs::type)));
    p.size = getValue(apvts, IDs::size);
    p.decaySeconds = getValue(apvts, IDs::decaySeconds);
    p.preDelayMs = getValue(apvts, IDs::preDelayMs);
    p.diffusion = getValue(apvts, IDs::diffusion);
    p.damping = getValue(apvts, IDs::damping);
    p.early = getValue(apvts, IDs::early);
    p.width = getValue(apvts, IDs::width);
    p.mix = getValue(apvts, IDs::mix);
    p.outputDb = getValue(apvts, IDs::outputDb);
    p.motion = getValue(apvts, IDs::motion);
    p.dynamic = getValue(apvts, IDs::dynamic);
    p.harmonic = getValue(apvts, IDs::harmonic);
    p.warp = getValue(apvts, IDs::warp);
    p.swell = getValue(apvts, IDs::swell);
    p.texture = getValue(apvts, IDs::texture);
    p.tone = getValue(apvts, IDs::tone);
    p.lowCutHz = getValue(apvts, IDs::lowCutHz);
    p.highCutHz = getValue(apvts, IDs::highCutHz);
    p.modRateHz = getValue(apvts, IDs::modRateHz);
    p.modDepth = getValue(apvts, IDs::modDepth);
    p.duckAmount = getValue(apvts, IDs::duckAmount);
    p.bloomAmount = getValue(apvts, IDs::bloomAmount);
    p.freeze = getBoolValue(apvts, IDs::freeze);
    p.distance = getValue(apvts, IDs::distance);
    p.transientPreserve = getValue(apvts, IDs::transientPreserve);

    return p;
}

juce::AudioProcessorEditor* BloomVerbAudioProcessor::createEditor()
{
    return new BloomVerbAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BloomVerbAudioProcessor();
}
