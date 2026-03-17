#pragma once

#include "IReverbAlgorithm.h"

namespace bloomverb
{
/** Type index to family mapping per plan:
 *  Plate (0) -> Plate
 *  Hall (1), Room (2) -> HallRoom
 *  Cloud (3), Bloom (4), Dream (5) -> CloudBloomDream
 *  Grain (6), Chamber (7) -> GrainSpecial
 */
inline ReverbFamily getFamilyForType(int type) noexcept
{
    switch (type)
    {
    case 0:
        return ReverbFamily::Plate;
    case 1:
    case 2:
        return ReverbFamily::HallRoom;
    case 3:
    case 4:
    case 5:
        return ReverbFamily::CloudBloomDream;
    case 6:
    case 7:
        return ReverbFamily::GrainSpecial;
    default:
        return ReverbFamily::HallRoom;
    }
}

inline int getVariantForType(int type) noexcept
{
    switch (type)
    {
    case 0:
        return 0; // Plate
    case 1:
        return 0; // Hall
    case 2:
        return 1; // Room
    case 3:
        return 0; // Cloud
    case 4:
        return 1; // Bloom
    case 5:
        return 2; // Dream
    case 6:
        return 0; // Grain
    case 7:
        return 1; // Chamber
    default:
        return 0;
    }
}
} // namespace bloomverb
