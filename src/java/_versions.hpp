#pragma once

#include "_version.hpp"

namespace je2be::java {

i32 constexpr kBlockDataVersion = 18168865;                                    // v1.21.114
Version constexpr kMinimumCompatibleClientVersion = Version(1, 21, 110, 0, 0); // v1.21.114
u8 constexpr kSubChunkBlockStorageVersion = 9;                                 // v1.21.114
i32 constexpr kStorageVersion = 10;                                            // v1.21.114, "StorageVersion" in level.dat
i32 constexpr kNetworkVersion = 844;                                           // v1.21.114, "NetworkVersion" in level.dat
char8_t const *const kInventoryVersion = u8"1.21.114";                         // v1.21.114, "InventoryVersion" in level.dat
u8 constexpr kDragonFightVersion = 1;                                          // v1.21.114

Version constexpr kSupportVersion = Version(1, 21, 114, 1, 0); // v1.21.114, "lastOpenedWithVersion" in level.dat

uint8_t constexpr kChunkVersion = 0x29;          // v1.21.114
uint8_t constexpr kChunkVersionMaxLegacy = 0x16; // v1.17.41

} // namespace je2be::java
