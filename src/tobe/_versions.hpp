#pragma once

#include "_version.hpp"

namespace je2be::tobe {

i32 constexpr kBlockDataVersion = 18090528;                                  // v1.20.15
Version constexpr kMinimumCompatibleClientVersion = Version(1, 20, 0, 0, 0); // v1.20.0
u8 constexpr kSubChunkBlockStorageVersion = 9;                               // v1.20.0
i32 constexpr kStorageVersion = 10;                                          // v1.20.0
i32 constexpr kNetworkVersion = 589;                                         // v1.20.0
char8_t const *const kInventoryVersion = u8"1.20.0";                         // v1.20.0
u8 constexpr kDragonFightVersion = 0;                                        // v1.20.0

// for lastOpenedWithVersion of level.dat
Version constexpr kSupportVersion = Version(1, 20, 0, 1, 0); // v1.20.0

char constexpr kChunkVersion = 0x28;          // v1.20.0
char constexpr kChunkVersionMaxLegacy = 0x16; // v1.17.41

} // namespace je2be::tobe
