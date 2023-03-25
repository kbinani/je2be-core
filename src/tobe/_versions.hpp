#pragma once

#include "_version.hpp"

namespace je2be::tobe {

i32 constexpr kBlockDataVersion = 18040335;                                   // v1.19.70
Version constexpr kMinimumCompatibleClientVersion = Version(1, 19, 70, 0, 0); // v1.19.70
u8 constexpr kSubChunkBlockStorageVersion = 9;                                // v1.19.70
i32 constexpr kStorageVersion = 10;                                           // v1.19.70
i32 constexpr kNetworkVersion = 575;                                          // v1.19.70
char8_t const *const kInventoryVersion = u8"1.19.70";                         // v1.19.70
u8 constexpr kDragonFightVersion = 0;                                         // v1.19.70

// for lastOpenedWithVersion of level.dat
Version constexpr kSupportVersion = Version(1, 19, 70, 2, 0); // v1.19.70

char constexpr kChunkVersion = 0x28;          // v1.19.70
char constexpr kChunkVersionMaxLegacy = 0x16; // v1.17.41

} // namespace je2be::tobe
