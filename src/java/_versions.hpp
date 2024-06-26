#pragma once

#include "_version.hpp"

namespace je2be::java {

i32 constexpr kBlockDataVersion = 18153475;                                  // v1.21.0
Version constexpr kMinimumCompatibleClientVersion = Version(1, 21, 0, 0, 0); // v1.21.0
u8 constexpr kSubChunkBlockStorageVersion = 9;                               // v1.21.0
i32 constexpr kStorageVersion = 10;                                          // v1.21.0, "StorageVersion" in level.dat
i32 constexpr kNetworkVersion = 685;                                         // v1.21.0, "NetworkVersion" in level.dat
char8_t const *const kInventoryVersion = u8"1.21.0";                         // v1.21.0, "InventoryVersion" in level.dat
u8 constexpr kDragonFightVersion = 0;                                        // v1.21.0

Version constexpr kSupportVersion = Version(1, 21, 0, 3, 0); // v1.21.0, "lastOpenedWithVersion" in level.dat

uint8_t constexpr kChunkVersion = 0x28;          // v1.21.0
uint8_t constexpr kChunkVersionMaxLegacy = 0x16; // v1.17.41

} // namespace je2be::java
