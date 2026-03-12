#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace
{
constexpr auto kStateVersionProperty = "state_version";
constexpr auto kSelectedPresetIndexProperty = "selected_preset_index";
constexpr auto kSelectedPresetIdProperty = "selected_preset_id";
constexpr auto kSelectedPresetNameProperty = "selected_preset_name";
constexpr int kStateVersion = 2;

float computePeakLevel(const juce::AudioBuffer<float>& buffer, int numChannels)
{
    float peak = 0.0f;
    for (int channel = 0; channel < numChannels; ++channel)
    {
        const float* data = buffer.getReadPointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            peak = juce::jmax(peak, std::abs(data[sample]));
    }

    return peak;
}

int findPresetIndexFromState(const juce::ValueTree& state,
                             const std::vector<bloomverb::presets::BloomVerbPreset>& presets)
{
    if (presets.empty())
        return -1;

    const auto presetId = state.getProperty(kSelectedPresetIdProperty).toString();
    if (presetId.isNotEmpty())
    {
        for (size_t i = 0; i < presets.size(); ++i)
        {
            if (presetId == juce::String(presets[i].id))
                return static_cast<int>(i);
        }
    }

    const auto presetName = state.getProperty(kSelectedPresetNameProperty).toString();
    if (presetName.isNotEmpty())
    {
        for (size_t i = 0; i < presets.size(); ++i)
        {
            if (presetName == juce::String(presets[i].name))
                return static_cast<int>(i);
        }
    }

    const auto presetIndex = static_cast<int>(state.getProperty(kSelectedPresetIndexProperty, -1));
    if (presetIndex >= 0 && presetIndex < static_cast<int>(presets.size()))
        return presetIndex;

    return 0;
}
}

BloomVerbAudioProcessor::BloomVerbAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "BloomVerbState", bloomverb::params::createParameterLayout()),
      presetNames(bloomverb::presets::getFactoryPresetNames())
{
#if JucePlugin_Build_Standalone
    standaloneFormatManager.registerBasicFormats();
    standaloneReadAheadThread.startThread();
#endif
}

BloomVerbAudioProcessor::~BloomVerbAudioProcessor()
{
#if JucePlugin_Build_Standalone
    releaseStandalonePlaybackResources();
    standaloneReadAheadThread.stopThread(2000);
#endif
}

void BloomVerbAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());

#if JucePlugin_Build_Standalone
    const juce::SpinLock::ScopedLockType lock(standalonePlaybackLock);
    standalonePlaybackBuffer.setSize(juce::jmax(1, getTotalNumOutputChannels()),
                                     juce::jmax(1, samplesPerBlock),
                                     false,
                                     false,
                                     true);
    standaloneTransportSource.prepareToPlay(samplesPerBlock, sampleRate);
#endif
}

void BloomVerbAudioProcessor::releaseResources()
{
#if JucePlugin_Build_Standalone
    const juce::SpinLock::ScopedLockType lock(standalonePlaybackLock);
    standaloneTransportSource.releaseResources();
    standalonePlaybackBuffer.setSize(0, 0);
#endif
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

#if JucePlugin_Build_Standalone
    {
        const juce::SpinLock::ScopedTryLockType lock(standalonePlaybackLock);
        if (lock.isLocked() && standaloneReaderSource != nullptr)
        {
            if (standalonePlaybackBuffer.getNumChannels() != totalOutputChannels
                || standalonePlaybackBuffer.getNumSamples() < buffer.getNumSamples())
            {
                standalonePlaybackBuffer.setSize(juce::jmax(1, totalOutputChannels),
                                                buffer.getNumSamples(),
                                                false,
                                                false,
                                                true);
            }

            standalonePlaybackBuffer.clear();
            juce::AudioSourceChannelInfo fileInfo(&standalonePlaybackBuffer, 0, buffer.getNumSamples());
            standaloneTransportSource.getNextAudioBlock(fileInfo);

            for (int channel = 0; channel < totalOutputChannels; ++channel)
            {
                const int sourceChannel = juce::jmin(channel, standalonePlaybackBuffer.getNumChannels() - 1);
                buffer.copyFrom(channel, 0, standalonePlaybackBuffer, sourceChannel, 0, buffer.getNumSamples());
            }
        }
    }
#endif

    const auto runtimeParameters = readRuntimeParameters();
    const auto meterChannels = juce::jmax(1, juce::jmin(2, totalOutputChannels));
    updateMeterValue(inputMeterLevel, computePeakLevel(buffer, meterChannels));
    freezeVisualAmount.store(runtimeParameters.freeze ? 1.0f : 0.0f);

    engine.process(buffer, runtimeParameters);
    updateMeterValue(outputMeterLevel, computePeakLevel(buffer, juce::jmin(2, totalOutputChannels)));
}

double BloomVerbAudioProcessor::getTailLengthSeconds() const
{
    return bloomverb::params::getValue(apvts, bloomverb::params::IDs::decaySeconds);
}

const juce::StringArray& BloomVerbAudioProcessor::getPresetNames() const noexcept
{
    return presetNames;
}

int BloomVerbAudioProcessor::getNumPrograms()
{
    return juce::jmax(1, presetNames.size());
}

int BloomVerbAudioProcessor::getCurrentProgram()
{
    return juce::jmax(0, getCurrentPresetIndex());
}

void BloomVerbAudioProcessor::setCurrentProgram(int index)
{
    applyPresetByIndex(index);
}

const juce::String BloomVerbAudioProcessor::getProgramName(int index)
{
    if (index >= 0 && index < presetNames.size())
        return presetNames[index];

    return {};
}

int BloomVerbAudioProcessor::getCurrentPresetIndex() const noexcept
{
    const auto presetCount = presetNames.size();
    if (presetCount <= 0)
        return -1;

    return juce::jlimit(0, presetCount - 1, currentPresetIndex.load());
}

void BloomVerbAudioProcessor::applyPresetByIndex(int presetIndex)
{
    const auto& presets = bloomverb::presets::getFactoryPresets();
    const auto presetCount = static_cast<int>(presets.size());
    if (presetCount <= 0 || presetIndex < 0 || presetIndex >= presetCount)
        return;

    const auto applyNow = [this, presetIndex, &presets]
    {
        applyPresetInternal(presets[static_cast<size_t>(presetIndex)]);
        currentPresetIndex.store(presetIndex);
    };

    if (juce::MessageManager::existsAndIsCurrentThread())
    {
        applyNow();
        return;
    }

    if (auto* messageManager = juce::MessageManager::getInstanceWithoutCreating(); messageManager != nullptr)
    {
        messageManager->callAsync([this, presetIndex]
        {
            applyPresetByIndex(presetIndex);
        });
        return;
    }

    applyNow();
}

void BloomVerbAudioProcessor::applyNextPreset()
{
    const auto presetCount = presetNames.size();
    if (presetCount <= 0)
        return;

    const auto current = getCurrentPresetIndex();
    const auto nextIndex = (current + 1) % presetCount;
    applyPresetByIndex(nextIndex);
}

void BloomVerbAudioProcessor::applyPreviousPreset()
{
    const auto presetCount = presetNames.size();
    if (presetCount <= 0)
        return;

    const auto current = getCurrentPresetIndex();
    const auto previousIndex = (current + presetCount - 1) % presetCount;
    applyPresetByIndex(previousIndex);
}

void BloomVerbAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty(kStateVersionProperty, kStateVersion, nullptr);
    state.setProperty(kSelectedPresetIndexProperty, getCurrentPresetIndex(), nullptr);

    const auto presetIndex = getCurrentPresetIndex();
    const auto& presets = bloomverb::presets::getFactoryPresets();
    if (presetIndex >= 0 && presetIndex < static_cast<int>(presets.size()))
    {
        state.setProperty(kSelectedPresetIdProperty, juce::String(presets[static_cast<size_t>(presetIndex)].id), nullptr);
        state.setProperty(kSelectedPresetNameProperty, juce::String(presets[static_cast<size_t>(presetIndex)].name), nullptr);
    }

    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void BloomVerbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (const auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (xml->hasTagName(apvts.state.getType()))
        {
            auto restoredState = juce::ValueTree::fromXml(*xml);
            apvts.replaceState(restoredState);
            currentPresetIndex.store(findPresetIndexFromState(restoredState, bloomverb::presets::getFactoryPresets()));
        }
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

void BloomVerbAudioProcessor::applyPresetInternal(const bloomverb::presets::BloomVerbPreset& preset)
{
    const auto setParameterValue = [this](const std::string& paramID, float value)
    {
        if (auto* parameter = apvts.getParameter(juce::StringRef(paramID.c_str())); parameter != nullptr)
        {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
            parameter->endChangeGesture();
        }
    };

    for (const auto& [parameterId, value] : preset.parameterValuesById)
        setParameterValue(parameterId, value);
}

void BloomVerbAudioProcessor::updateMeterValue(std::atomic<float>& meter, float target) const
{
    const float current = meter.load();
    const float release = 0.18f;
    const float next = (target > current) ? target : current + (target - current) * release;
    meter.store(juce::jlimit(0.0f, 1.0f, next));
}

#if JucePlugin_Build_Standalone
bool BloomVerbAudioProcessor::loadStandalonePlaybackFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;

    std::unique_ptr<juce::AudioFormatReader> reader(standaloneFormatManager.createReaderFor(file));
    if (reader == nullptr)
        return false;

    const auto sourceSampleRate = reader->sampleRate;
    const auto sourceNumChannels = static_cast<int>(reader->numChannels);
    auto replacementSource = std::make_unique<juce::AudioFormatReaderSource>(reader.release(), true);
    replacementSource->setLooping(standaloneLoopingEnabled.load());

    const juce::SpinLock::ScopedLockType lock(standalonePlaybackLock);
    standaloneTransportSource.stop();
    standaloneTransportSource.setSource(nullptr);
    standaloneReaderSource.reset();

    standaloneReaderSource = std::move(replacementSource);
    standaloneTransportSource.setSource(standaloneReaderSource.get(),
                                        32768,
                                        &standaloneReadAheadThread,
                                        sourceSampleRate,
                                        sourceNumChannels);
    standaloneTransportSource.setPosition(0.0);
    standaloneTransportSource.start();

    const double lengthSeconds = standaloneReaderSource->getTotalLength() / juce::jmax(1.0, sourceSampleRate);
    standaloneLoadedFileLabel = "Loaded: " + file.getFileName() + "  |  " + juce::String(lengthSeconds, 1) + " s";
    return true;
}

void BloomVerbAudioProcessor::setStandalonePlaybackActive(bool shouldPlay)
{
    const juce::SpinLock::ScopedLockType lock(standalonePlaybackLock);
    if (standaloneReaderSource == nullptr)
        return;

    if (shouldPlay)
        standaloneTransportSource.start();
    else
        standaloneTransportSource.stop();
}

bool BloomVerbAudioProcessor::isStandalonePlaybackActive() const noexcept
{
    const juce::SpinLock::ScopedLockType lock(const_cast<juce::SpinLock&>(standalonePlaybackLock));
    return standaloneTransportSource.isPlaying();
}

bool BloomVerbAudioProcessor::hasStandalonePlaybackFile() const noexcept
{
    const juce::SpinLock::ScopedLockType lock(const_cast<juce::SpinLock&>(standalonePlaybackLock));
    return standaloneReaderSource != nullptr;
}

void BloomVerbAudioProcessor::setStandalonePlaybackLooping(bool shouldLoop)
{
    standaloneLoopingEnabled.store(shouldLoop);
    const juce::SpinLock::ScopedLockType lock(standalonePlaybackLock);
    if (standaloneReaderSource != nullptr)
        standaloneReaderSource->setLooping(shouldLoop);
}

bool BloomVerbAudioProcessor::isStandalonePlaybackLooping() const noexcept
{
    return standaloneLoopingEnabled.load();
}

juce::String BloomVerbAudioProcessor::getStandalonePlaybackFileLabel() const
{
    const juce::SpinLock::ScopedLockType lock(const_cast<juce::SpinLock&>(standalonePlaybackLock));
    return standaloneLoadedFileLabel;
}

juce::String BloomVerbAudioProcessor::getStandalonePlaybackStatusText() const
{
    const juce::SpinLock::ScopedLockType lock(const_cast<juce::SpinLock&>(standalonePlaybackLock));
    if (standaloneReaderSource == nullptr)
        return "Load an audio file to audition BloomVerb";

    juce::String status = standaloneTransportSource.isPlaying() ? "Playing" : "Stopped";
    status << "  |  " << (standaloneLoopingEnabled.load() ? "Loop on" : "Loop off");
    return status;
}

void BloomVerbAudioProcessor::releaseStandalonePlaybackResources()
{
    const juce::SpinLock::ScopedLockType lock(standalonePlaybackLock);
    standaloneTransportSource.stop();
    standaloneTransportSource.setSource(nullptr);
    standaloneReaderSource.reset();
    standalonePlaybackBuffer.setSize(0, 0);
}
#endif

juce::AudioProcessorEditor* BloomVerbAudioProcessor::createEditor()
{
    return new BloomVerbAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BloomVerbAudioProcessor();
}
