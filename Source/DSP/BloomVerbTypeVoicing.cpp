#include "BloomVerbTypeVoicing.h"

namespace bloomverb
{
namespace
{
TypeDescriptor makeDescriptor(TypeVoicingProfile voicing,
                              TankLineArray baseDelaySeconds,
                              TankLineArray inputWeightsLeft,
                              TankLineArray inputWeightsRight,
                              TankLineArray outputWeightsLeft,
                              TankLineArray outputWeightsRight)
{
    return { voicing, { baseDelaySeconds, inputWeightsLeft, inputWeightsRight, outputWeightsLeft, outputWeightsRight } };
}

const std::array<TypeDescriptor, 8> kTypeDescriptors {{
    // Plate
    makeDescriptor(
        { 0.78f, -0.18f, -0.12f, 0.55f, 1.10f, 0.92f, 0.88f, 0.68f, -0.10f,
          0.35f, 0.45f, 0.60f, 0.72f, -0.08f, 0.08f, -0.12f, 1.35f, 1.18f, 0.92f, 1.18f },
        { 0.0187f, 0.0219f, 0.0258f, 0.0294f, 0.0338f, 0.0379f, 0.0426f, 0.0488f },
        { 0.92f, 0.50f, 0.68f, -0.38f, 0.62f, -0.22f, 0.44f, -0.56f },
        { -0.18f, 0.74f, -0.28f, 0.52f, -0.44f, 0.42f, -0.16f, 0.58f },
        { 0.46f, -0.18f, 0.38f, -0.15f, 0.32f, -0.12f, 0.24f, -0.16f },
        { -0.14f, 0.42f, -0.16f, 0.31f, -0.13f, 0.24f, -0.10f, 0.28f }),
    // Hall
    makeDescriptor(
        { 1.08f,  0.10f, 0.00f, 1.10f, 0.82f, 1.10f, 1.00f, 0.85f,  0.00f,
          0.00f, 1.10f, 1.25f, 1.40f, 0.04f, 0.00f, 0.00f, 1.00f, 0.98f, 1.03f, 0.92f },
        { 0.0311f, 0.0377f, 0.0419f, 0.0483f, 0.0557f, 0.0613f, 0.0719f, 0.0837f },
        { 0.85f, 0.35f, 0.65f, -0.45f, 0.55f, -0.30f, 0.40f, -0.70f },
        { -0.30f, 0.80f, -0.45f, 0.60f, -0.65f, 0.55f, -0.25f, 0.72f },
        { 0.40f, -0.22f, 0.36f, -0.18f, 0.30f, -0.15f, 0.26f, -0.20f },
        { -0.20f, 0.38f, -0.18f, 0.35f, -0.16f, 0.28f, -0.14f, 0.34f }),
    // Room
    makeDescriptor(
        { 0.62f, -0.26f, -0.08f, 0.42f, 1.42f, 0.82f, 0.76f, 0.55f, -0.18f,
          0.45f, 0.34f, 0.50f, 0.68f, -0.10f, -0.08f, -0.14f, 1.25f, 0.84f, 0.86f, 1.10f },
        { 0.0108f, 0.0135f, 0.0168f, 0.0201f, 0.0239f, 0.0278f, 0.0317f, 0.0364f },
        { 1.00f, 0.42f, 0.54f, -0.26f, 0.46f, -0.18f, 0.30f, -0.42f },
        { -0.08f, 0.58f, -0.20f, 0.38f, -0.32f, 0.28f, -0.10f, 0.40f },
        { 0.34f, -0.16f, 0.28f, -0.12f, 0.22f, -0.10f, 0.18f, -0.12f },
        { -0.10f, 0.30f, -0.12f, 0.24f, -0.10f, 0.18f, -0.08f, 0.20f }),
    // Cloud
    makeDescriptor(
        { 1.42f,  0.22f, 0.12f, 1.34f, 0.42f, 1.34f, 1.35f, 0.92f,  0.10f,
         -0.35f, 1.45f, 1.80f, 2.25f, 0.14f, 0.02f, 0.24f, 0.78f, 0.80f, 1.14f, 0.72f },
        { 0.0470f, 0.0548f, 0.0645f, 0.0769f, 0.0897f, 0.1045f, 0.1194f, 0.1380f },
        { 0.72f, 0.22f, 0.60f, -0.52f, 0.48f, -0.38f, 0.34f, -0.78f },
        { -0.36f, 0.72f, -0.52f, 0.68f, -0.72f, 0.62f, -0.30f, 0.82f },
        { 0.32f, -0.20f, 0.30f, -0.16f, 0.28f, -0.12f, 0.26f, -0.18f },
        { -0.24f, 0.34f, -0.18f, 0.38f, -0.15f, 0.32f, -0.12f, 0.40f }),
    // Bloom
    makeDescriptor(
        { 1.22f,  0.14f, 0.03f, 1.12f, 0.62f, 1.22f, 1.18f, 0.82f,  0.06f,
         -0.18f, 1.18f, 1.45f, 1.82f, 0.10f, 0.03f, 0.16f, 0.92f, 0.86f, 1.08f, 0.84f },
        { 0.0395f, 0.0468f, 0.0554f, 0.0658f, 0.0772f, 0.0901f, 0.1048f, 0.1219f },
        { 0.76f, 0.28f, 0.62f, -0.46f, 0.52f, -0.34f, 0.38f, -0.74f },
        { -0.28f, 0.76f, -0.48f, 0.63f, -0.70f, 0.58f, -0.24f, 0.76f },
        { 0.35f, -0.20f, 0.33f, -0.16f, 0.30f, -0.13f, 0.26f, -0.18f },
        { -0.21f, 0.36f, -0.17f, 0.35f, -0.14f, 0.29f, -0.12f, 0.36f }),
    // Grain
    makeDescriptor(
        { 0.96f, -0.04f, -0.04f, 0.90f, 0.95f, 1.00f, 1.00f, 0.75f,  0.24f,
          0.12f, 0.85f, 1.10f, 1.45f, 0.04f, 0.14f, 0.08f, 1.08f, 0.90f, 0.98f, 1.32f },
        { 0.0242f, 0.0287f, 0.0339f, 0.0416f, 0.0491f, 0.0578f, 0.0672f, 0.0796f },
        { 0.88f, 0.22f, 0.72f, -0.54f, 0.44f, -0.36f, 0.28f, -0.66f },
        { -0.34f, 0.70f, -0.56f, 0.54f, -0.60f, 0.48f, -0.22f, 0.68f },
        { 0.38f, -0.25f, 0.34f, -0.20f, 0.29f, -0.16f, 0.24f, -0.19f },
        { -0.22f, 0.35f, -0.20f, 0.33f, -0.18f, 0.29f, -0.14f, 0.31f }),
    // Chamber
    makeDescriptor(
        { 0.88f, -0.10f, -0.02f, 0.84f, 1.08f, 0.94f, 0.92f, 0.64f,  0.00f,
          0.25f, 0.72f, 0.95f, 1.20f, -0.04f, -0.04f, -0.04f, 1.12f, 0.96f, 0.95f, 0.92f },
        { 0.0164f, 0.0201f, 0.0249f, 0.0307f, 0.0369f, 0.0442f, 0.0518f, 0.0610f },
        { 0.90f, 0.40f, 0.60f, -0.34f, 0.50f, -0.24f, 0.36f, -0.52f },
        { -0.20f, 0.72f, -0.30f, 0.48f, -0.46f, 0.40f, -0.16f, 0.54f },
        { 0.42f, -0.18f, 0.35f, -0.15f, 0.28f, -0.13f, 0.22f, -0.15f },
        { -0.16f, 0.38f, -0.14f, 0.30f, -0.13f, 0.24f, -0.10f, 0.26f }),
    // Dream
    makeDescriptor(
        { 1.36f,  0.26f, 0.18f, 1.24f, 0.34f, 1.40f, 1.28f, 1.00f,  0.12f,
         -0.40f, 1.60f, 2.10f, 2.60f, 0.16f, 0.05f, 0.28f, 0.82f, 0.72f, 1.18f, 0.66f },
        { 0.0538f, 0.0628f, 0.0746f, 0.0889f, 0.1048f, 0.1231f, 0.1441f, 0.1680f },
        { 0.70f, 0.18f, 0.58f, -0.58f, 0.42f, -0.40f, 0.30f, -0.82f },
        { -0.40f, 0.68f, -0.56f, 0.72f, -0.76f, 0.66f, -0.32f, 0.86f },
        { 0.30f, -0.22f, 0.28f, -0.18f, 0.26f, -0.15f, 0.24f, -0.20f },
        { -0.28f, 0.32f, -0.20f, 0.40f, -0.16f, 0.35f, -0.14f, 0.43f })
}};
} // namespace

const TypeDescriptor& getTypeDescriptor(int type) noexcept
{
    if (type >= 0 && static_cast<size_t>(type) < kTypeDescriptors.size())
        return kTypeDescriptors[static_cast<size_t>(type)];

    return kTypeDescriptors[1];
}

const TypeVoicingProfile& getTypeVoicingProfile(int type) noexcept
{
    return getTypeDescriptor(type).voicing;
}

const TankConstellation& getTankConstellation(int type) noexcept
{
    return getTypeDescriptor(type).tank;
}
} // namespace bloomverb
