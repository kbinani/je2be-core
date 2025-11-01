#pragma once

#include "_version.hpp"

namespace je2be::java {

i32 constexpr kBlockDataVersion = 18168865;                                    // v1.21.120
Version constexpr kMinimumCompatibleClientVersion = Version(1, 21, 120, 0, 0); // v1.21.120
u8 constexpr kSubChunkBlockStorageVersion = 9;                                 // v1.21.120
i32 constexpr kStorageVersion = 10;                                            // v1.21.120, "StorageVersion" in level.dat
i32 constexpr kNetworkVersion = 859;                                           // v1.21.120, "NetworkVersion" in level.dat
char8_t const *const kInventoryVersion = u8"1.21.120";                         // v1.21.120, "InventoryVersion" in level.dat
u8 constexpr kDragonFightVersion = 0;                                          // v1.21.120

Version constexpr kSupportVersion = Version(1, 21, 120, 4, 0); // v1.21.120, "lastOpenedWithVersion" in level.dat

uint8_t constexpr kChunkVersion = 0x2a;          // v1.21.120
uint8_t constexpr kChunkVersionMaxLegacy = 0x16; // v1.17.41

} // namespace je2be::java
