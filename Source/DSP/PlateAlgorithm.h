#pragma once

#include "IReverbAlgorithm.h"
#include "FDNCore.h"

namespace bloomverb
{
/** Plate family: metallic, dense plate-style network. */
class PlateAlgorithm : public IReverbAlgorithm
{
public:
    void prepare(double sampleRate) override;
    void reset() override;
    void processSample(float diffuseL,
                       float diffuseR,
                       const AlgorithmProcessContext& ctx,
                       float& wetL,
                       float& wetR) override;

private:
    std::unique_ptr<FDNCore> fdnCore;
};
} // namespace bloomverb
