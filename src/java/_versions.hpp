#pragma once

#include "_version.hpp"

namespace je2be::java {

i32 constexpr kBlockDataVersion = 18163713;                                   // v1.21.50
Version constexpr kMinimumCompatibleClientVersion = Version(1, 21, 50, 0, 0); // v1.21.50
u8 constexpr kSubChunkBlockStorageVersion = 9;                                // v1.21.50
i32 constexpr kStorageVersion = 10;                                           // v1.21.50, "StorageVersion" in level.dat
i32 constexpr kNetworkVersion = 766;                                          // v1.21.50, "NetworkVersion" in level.dat
char8_t const *const kInventoryVersion = u8"1.21.50";                         // v1.21.50, "InventoryVersion" in level.dat
u8 constexpr kDragonFightVersion = 0;                                         // v1.21.50

Version constexpr kSupportVersion = Version(1, 21, 0, 3, 0); // v1.21.0, "lastOpenedWithVersion" in level.dat

uint8_t constexpr kChunkVersion = 0x29;          // v1.21.50
uint8_t constexpr kChunkVersionMaxLegacy = 0x16; // v1.17.41

} // namespace je2be::java
