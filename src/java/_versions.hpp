#pragma once

#include "_version.hpp"

namespace je2be::java {

i32 constexpr kBlockDataVersion = 18108419;                                   // v1.20.81
Version constexpr kMinimumCompatibleClientVersion = Version(1, 20, 80, 0, 0); // v1.20.81
u8 constexpr kSubChunkBlockStorageVersion = 9;                                // v1.20.81
i32 constexpr kStorageVersion = 10;                                           // v1.20.81, "StorageVersion" in level.dat
i32 constexpr kNetworkVersion = 671;                                          // v1.20.81, "NetworkVersion" in level.dat
char8_t const *const kInventoryVersion = u8"1.20.81";                         // v1.20.81, "InventoryVersion" in level.dat
u8 constexpr kDragonFightVersion = 0;                                         // v1.20.81

Version constexpr kSupportVersion = Version(1, 20, 81, 1, 0); // v1.20.81, "lastOpenedWithVersion" in level.dat

uint8_t constexpr kChunkVersion = 0x80;          // v1.20.81
uint8_t constexpr kChunkVersionMaxLegacy = 0x16; // v1.17.41

} // namespace je2be::java
