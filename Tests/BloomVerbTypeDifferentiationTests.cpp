#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>

#include <juce_audio_basics/juce_audio_basics.h>

#include "DSP/BloomVerbEngine.h"
#include "Parameters/BloomVerbParameters.h"
#include "Presets/BloomVerbPreset.h"
#include "Presets/BloomVerbPresetBank.h"

namespace
{
constexpr double kSampleRate = 48000.0;
constexpr int kBlockSize = 128;
constexpr int kNumChannels = 2;
constexpr float kTwoPi = 6.28318530717958647692f;
constexpr int kSignalSamples = static_cast<int>(kSampleRate * 0.24);
constexpr int kTailSamples = static_cast<int>(kSampleRate * 0.36);

struct RenderMetrics
{
    double signalEnergy = 0.0;
    double tailEnergy = 0.0;
    double totalEnergy = 0.0;
    double centroid = 0.0;
    double leftEnergy = 0.0;
    double rightEnergy = 0.0;
    double correlationNumerator = 0.0;
    std::uint64_t checksum = 1469598103934665603ull;
    float peak = 0.0f;
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

float deterministicNoise(std::uint32_t index)
{
    std::uint32_t x = index * 747796405u + 2891336453u;
    x ^= x >> 16;
    x *= 2246822519u;
    return static_cast<float>(x & 0x00ffffffu) / static_cast<float>(0x00ffffffu) * 2.0f - 1.0f;
}

float getPresetFloat(const bloomverb::presets::BloomVerbPreset& preset,
                     const std::string& key,
                     float fallback)
{
    if (const auto it = preset.parameterValuesById.find(key); it != preset.parameterValuesById.end())
        return it->second;

    return fallback;
}

bloomverb::RuntimeParameters makeDeterministicTypeParams(int type)
{
    bloomverb::RuntimeParameters params;
    params.type = type;
    params.size = 0.60f;
    params.decaySeconds = 4.8f;
    params.preDelayMs = 22.0f;
    params.diffusion = 0.76f;
    params.damping = 0.44f;
    params.early = 0.40f;
    params.width = 1.05f;
    params.mix = 0.82f;
    params.outputDb = 0.0f;
    params.motion = 0.0f;
    params.dynamic = 0.30f;
    params.harmonic = 0.0f;
    params.warp = 0.0f;
    params.swell = 0.08f;
    params.texture = 0.34f;
    params.tone = 0.0f;
    params.lowCutHz = 100.0f;
    params.highCutHz = 13500.0f;
    params.modRateHz = 0.0f;
    params.modDepth = 0.0f;
    params.duckAmount = 0.0f;
    params.bloomAmount = 0.0f;
    params.freeze = false;
    params.distance = 0.25f;
    params.transientPreserve = 0.0f;
    return params;
}

void fillInput(juce::AudioBuffer<float>& block, int absoluteStart)
{
    for (int sample = 0; sample < block.getNumSamples(); ++sample)
    {
        const int absoluteSample = absoluteStart + sample;
        const float t = static_cast<float>(absoluteSample / kSampleRate);
        const float impulse = (absoluteSample % 997) == 0 ? 0.45f : 0.0f;
        const float tone = 0.18f * std::sin(kTwoPi * 143.0f * t)
                         + 0.12f * std::sin(kTwoPi * 317.0f * t)
                         + 0.08f * std::sin(kTwoPi * 761.0f * t)
                         + impulse;
        block.setSample(0, sample, tone + deterministicNoise(static_cast<std::uint32_t>(absoluteSample)) * 0.003f);
        block.setSample(1, sample, tone * 0.93f - deterministicNoise(static_cast<std::uint32_t>(absoluteSample + 43)) * 0.003f);
    }
}

bloomverb::RuntimeParameters makeParamsFromPreset(const bloomverb::presets::BloomVerbPreset& preset)
{
    using IDs = bloomverb::params::IDs;

    bloomverb::RuntimeParameters params;
    params.type = static_cast<int>(std::round(getPresetFloat(preset, IDs::type, 1.0f)));
    params.size = getPresetFloat(preset, IDs::size, params.size);
    params.decaySeconds = getPresetFloat(preset, IDs::decaySeconds, params.decaySeconds);
    params.preDelayMs = getPresetFloat(preset, IDs::preDelayMs, params.preDelayMs);
    params.diffusion = getPresetFloat(preset, IDs::diffusion, params.diffusion);
    params.damping = getPresetFloat(preset, IDs::damping, params.damping);
    params.early = getPresetFloat(preset, IDs::early, params.early);
    params.width = getPresetFloat(preset, IDs::width, params.width);
    params.mix = getPresetFloat(preset, IDs::mix, params.mix);
    params.outputDb = getPresetFloat(preset, IDs::outputDb, params.outputDb);
    params.motion = getPresetFloat(preset, IDs::motion, params.motion);
    params.dynamic = getPresetFloat(preset, IDs::dynamic, params.dynamic);
    params.harmonic = getPresetFloat(preset, IDs::harmonic, params.harmonic);
    params.warp = getPresetFloat(preset, IDs::warp, params.warp);
    params.swell = getPresetFloat(preset, IDs::swell, params.swell);
    params.texture = getPresetFloat(preset, IDs::texture, params.texture);
    params.tone = getPresetFloat(preset, IDs::tone, params.tone);
    params.lowCutHz = getPresetFloat(preset, IDs::lowCutHz, params.lowCutHz);
    params.highCutHz = getPresetFloat(preset, IDs::highCutHz, params.highCutHz);
    params.modRateHz = getPresetFloat(preset, IDs::modRateHz, params.modRateHz);
    params.modDepth = getPresetFloat(preset, IDs::modDepth, params.modDepth);
    params.duckAmount = getPresetFloat(preset, IDs::duckAmount, params.duckAmount);
    params.bloomAmount = getPresetFloat(preset, IDs::bloomAmount, params.bloomAmount);
    params.freeze = getPresetFloat(preset, IDs::freeze, 0.0f) > 0.5f;
    params.distance = getPresetFloat(preset, IDs::distance, params.distance);
    params.transientPreserve = getPresetFloat(preset, IDs::transientPreserve, params.transientPreserve);
    return params;
}

RenderMetrics renderSignal(const bloomverb::RuntimeParameters& params)
{
    bloomverb::BloomVerbEngine engine;
    engine.prepare(kSampleRate, kBlockSize, kNumChannels);

    juce::AudioBuffer<float> block(kNumChannels, kBlockSize);
    RenderMetrics metrics;

    int processed = 0;
    while (processed < kSignalSamples)
    {
        const int currentBlockSize = juce::jmin(kBlockSize, kSignalSamples - processed);
        if (currentBlockSize != block.getNumSamples())
            block.setSize(kNumChannels, currentBlockSize, false, false, true);

        fillInput(block, processed);
        engine.process(block, params);

        for (int sample = 0; sample < currentBlockSize; ++sample)
        {
            const int absoluteSample = processed + sample;
            const float left = block.getSample(0, sample);
            const float right = block.getSample(1, sample);
            const double energy = static_cast<double>(left) * left + static_cast<double>(right) * right;
            metrics.peak = juce::jmax(metrics.peak, juce::jmax(std::abs(left), std::abs(right)));

            metrics.totalEnergy += energy;
            metrics.centroid += static_cast<double>(absoluteSample) * energy;
            metrics.leftEnergy += static_cast<double>(left) * left;
            metrics.rightEnergy += static_cast<double>(right) * right;
            metrics.correlationNumerator += static_cast<double>(left) * right;
            metrics.signalEnergy += energy;

            const auto leftBits = static_cast<std::uint32_t>(juce::ByteOrder::swapIfBigEndian(std::bit_cast<std::uint32_t>(left)));
            const auto rightBits = static_cast<std::uint32_t>(juce::ByteOrder::swapIfBigEndian(std::bit_cast<std::uint32_t>(right)));
            metrics.checksum ^= leftBits + 0x9e3779b97f4a7c15ull + (metrics.checksum << 6u) + (metrics.checksum >> 2u);
            metrics.checksum ^= rightBits + 0x9e3779b97f4a7c15ull + (metrics.checksum << 6u) + (metrics.checksum >> 2u);
        }

        processed += currentBlockSize;
    }

    processed = 0;
    while (processed < kTailSamples)
    {
        const int currentBlockSize = juce::jmin(kBlockSize, kTailSamples - processed);
        if (currentBlockSize != block.getNumSamples())
            block.setSize(kNumChannels, currentBlockSize, false, false, true);

        block.clear();
        engine.process(block, params);

        for (int sample = 0; sample < currentBlockSize; ++sample)
        {
            const int absoluteSample = kSignalSamples + processed + sample;
            const float left = block.getSample(0, sample);
            const float right = block.getSample(1, sample);
            const double energy = static_cast<double>(left) * left + static_cast<double>(right) * right;
            metrics.peak = juce::jmax(metrics.peak, juce::jmax(std::abs(left), std::abs(right)));
            metrics.totalEnergy += energy;
            metrics.tailEnergy += energy;
            metrics.centroid += static_cast<double>(absoluteSample) * energy;
            metrics.leftEnergy += static_cast<double>(left) * left;
            metrics.rightEnergy += static_cast<double>(right) * right;
            metrics.correlationNumerator += static_cast<double>(left) * right;

            const auto leftBits = static_cast<std::uint32_t>(juce::ByteOrder::swapIfBigEndian(std::bit_cast<std::uint32_t>(left)));
            const auto rightBits = static_cast<std::uint32_t>(juce::ByteOrder::swapIfBigEndian(std::bit_cast<std::uint32_t>(right)));
            metrics.checksum ^= leftBits + 0x9e3779b97f4a7c15ull + (metrics.checksum << 6u) + (metrics.checksum >> 2u);
            metrics.checksum ^= rightBits + 0x9e3779b97f4a7c15ull + (metrics.checksum << 6u) + (metrics.checksum >> 2u);
        }

        processed += currentBlockSize;
    }

    metrics.centroid /= juce::jmax(1.0e-12, metrics.totalEnergy);
    return metrics;
}

double computeSignalTailRatio(const RenderMetrics& metrics)
{
    return metrics.signalEnergy / juce::jmax(1.0e-12, metrics.tailEnergy);
}

double computeStereoCorrelation(const RenderMetrics& metrics)
{
    return metrics.correlationNumerator / std::sqrt(juce::jmax(1.0e-12, metrics.leftEnergy * metrics.rightEnergy));
}
} // namespace

int main()
{
    bool ok = true;

    const auto plate = renderSignal(makeDeterministicTypeParams(0));
    const auto hall = renderSignal(makeDeterministicTypeParams(1));
    const auto room = renderSignal(makeDeterministicTypeParams(2));

    const double plateRatio = computeSignalTailRatio(plate);
    const double hallRatio = computeSignalTailRatio(hall);
    const double roomRatio = computeSignalTailRatio(room);

    const double plateCentroidSeconds = plate.centroid / kSampleRate;
    const double hallCentroidSeconds = hall.centroid / kSampleRate;
    const double roomCentroidSeconds = room.centroid / kSampleRate;

    const double hallCorrelation = computeStereoCorrelation(hall);
    const double roomCorrelation = computeStereoCorrelation(room);

    ok &= expect(plate.checksum != hall.checksum, "Plate and Hall deterministic renders should differ");
    ok &= expect(plate.checksum != room.checksum, "Plate and Room deterministic renders should differ");
    ok &= expect(hall.checksum != room.checksum, "Hall and Room deterministic renders should differ");

    ok &= expect(roomRatio > hallRatio * 1.02, "Room should keep less tail energy than Hall");
    ok &= expect(plateRatio > hallRatio * 1.005, "Plate should keep less tail energy than Hall");
    ok &= expect(hallCentroidSeconds > roomCentroidSeconds + 0.005, "Hall centroid should trail Room");
    ok &= expect(hallCentroidSeconds > plateCentroidSeconds + 0.005, "Hall centroid should trail Plate");
    ok &= expect(hallCorrelation < roomCorrelation - 0.01, "Hall should decorrelate more than Room");

    const auto cloud = renderSignal(makeDeterministicTypeParams(3));
    const auto grain = renderSignal(makeDeterministicTypeParams(6));
    const auto chamber = renderSignal(makeDeterministicTypeParams(7));

    ok &= expect(cloud.checksum != hall.checksum, "Cloud and Hall (different families) should differ");
    ok &= expect(cloud.checksum != plate.checksum, "Cloud and Plate (different families) should differ");
    ok &= expect(grain.checksum != hall.checksum, "Grain and Hall (different families) should differ");
    ok &= expect(grain.checksum != cloud.checksum, "Grain and Cloud (different families) should differ");
    ok &= expect(chamber.checksum != grain.checksum, "Chamber and Grain (same family, different variants) should differ");

    std::array<std::uint64_t, 8> typeChecksums;
    for (int t = 0; t < 8; ++t)
        typeChecksums[static_cast<size_t>(t)] = renderSignal(makeDeterministicTypeParams(t)).checksum;
    for (int a = 0; a < 8; ++a)
        for (int b = a + 1; b < 8; ++b)
            ok &= expect(typeChecksums[static_cast<size_t>(a)] != typeChecksums[static_cast<size_t>(b)],
                        "All 8 types must produce distinct deterministic output");

    bloomverb::RuntimeParameters sizeParams = makeDeterministicTypeParams(1);
    sizeParams.size = 0.15f;
    const auto hallSmall = renderSignal(sizeParams);
    sizeParams.size = 0.95f;
    const auto hallLarge = renderSignal(sizeParams);
    ok &= expect(hallSmall.checksum != hallLarge.checksum, "Size control must affect Hall output");

    bloomverb::RuntimeParameters decayParams = makeDeterministicTypeParams(2);
    decayParams.decaySeconds = 0.2f;
    const auto roomShort = renderSignal(decayParams);
    decayParams.decaySeconds = 12.0f;
    const auto roomLong = renderSignal(decayParams);
    ok &= expect(roomShort.checksum != roomLong.checksum, "Decay control must affect Room output");

    bloomverb::RuntimeParameters mixParams = makeDeterministicTypeParams(4);
    mixParams.mix = 0.0f;
    const auto bloomDry = renderSignal(mixParams);
    mixParams.mix = 1.0f;
    const auto bloomWet = renderSignal(mixParams);
    ok &= expect(bloomDry.checksum != bloomWet.checksum, "Mix control must affect Bloom output");

    const auto& presets = bloomverb::presets::getFactoryPresets();
    ok &= expect(presets.size() >= 3, "Expected at least three factory presets");
    if (!ok)
        return 1;

    const auto presetPlate = renderSignal(makeParamsFromPreset(presets[0]));
    const auto presetRoom = renderSignal(makeParamsFromPreset(presets[1]));
    const auto presetHall = renderSignal(makeParamsFromPreset(presets[2]));

    ok &= expect(presetPlate.checksum != presetRoom.checksum, "Small Plate and Tight Room presets should render differently");
    ok &= expect(presetPlate.checksum != presetHall.checksum, "Small Plate and Clean Hall presets should render differently");
    ok &= expect(presetRoom.checksum != presetHall.checksum, "Tight Room and Clean Hall presets should render differently");

    if (!ok)
        return 1;

    std::cout << "BloomVerbTypeDifferentiationTests passed.\n";
    return 0;
}
