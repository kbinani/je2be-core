#pragma once

#include "_version.hpp"

namespace je2be::tobe {

i32 constexpr kBlockDataVersion = 17959425;                                   // v1.19.50
Version constexpr kMinimumCompatibleClientVersion = Version(1, 19, 50, 0, 0); // v1.19.50
u8 constexpr kSubChunkBlockStorageVersion = 9;                                // v1.19.50
i32 constexpr kStorageVersion = 10;                                           // v1.19.50
i32 constexpr kNetworkVersion = 560;                                          // v1.19.50
char const *const kInventoryVersion = "1.19.50";                              // v1.19.50
u8 constexpr kDragonFightVersion = 0;                                         // v1.19.50

// for lastOpenedWithVersion of level.dat
Version constexpr kSupportVersion = Version(1, 19, 50, 2, 0); // v1.19.50

char constexpr kChunkVersion = 0x28;          // v1.19.50
char constexpr kChunkVersionMaxLegacy = 0x16; // v1.17.41

} // namespace je2be::tobe
