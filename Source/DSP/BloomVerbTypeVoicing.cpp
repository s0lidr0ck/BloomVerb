#include "BloomVerbTypeVoicing.h"

namespace bloomverb
{
TypeVoicingProfile getTypeVoicingProfile(int type) noexcept
{
    switch (type)
    {
        case 0: // Plate
            return { 0.85f, -0.10f, 0.70f, 0.70f, 0.85f, 0.80f, -0.10f,  0.35f, -0.05f,  0.12f, -0.15f };
        case 1: // Hall
            return { 1.00f,  0.00f, 1.00f, 1.00f, 1.00f, 1.00f,  0.00f,  0.00f,  0.00f,  0.00f,  0.00f };
        case 2: // Room
            return { 0.75f, -0.05f, 0.80f, 1.35f, 0.80f, 0.70f, -0.15f,  0.45f, -0.08f, -0.05f, -0.10f };
        case 3: // Cloud
            return { 1.35f,  0.10f, 1.30f, 0.65f, 1.40f, 1.20f,  0.10f, -0.35f,  0.12f,  0.05f,  0.30f };
        case 4: // Bloom
            return { 1.20f,  0.02f, 1.15f, 0.80f, 1.25f, 1.00f,  0.05f, -0.15f,  0.08f,  0.06f,  0.18f };
        case 5: // Grain
            return { 1.00f, -0.05f, 0.95f, 0.90f, 1.10f, 0.90f,  0.35f,  0.10f,  0.05f,  0.25f,  0.10f };
        case 6: // Chamber
            return { 0.90f, -0.02f, 0.85f, 1.20f, 0.95f, 0.90f,  0.00f,  0.25f, -0.02f, -0.02f, -0.05f };
        case 7: // Dream
            return { 1.25f,  0.15f, 1.20f, 0.75f, 1.35f, 1.50f,  0.15f, -0.40f,  0.15f,  0.10f,  0.35f };
        default:
            break;
    }

    return {};
}
} // namespace bloomverb
