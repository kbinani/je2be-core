#pragma once

namespace je2be::tobe {

int32_t constexpr kBlockDataVersion = 17959425;                               // v1.19.50
Version constexpr kMinimumCompatibleClientVersion = Version(1, 19, 50, 0, 0); // v1.19.50
uint8_t constexpr kSubChunkBlockStorageVersion = 9;                           // v1.19.50
int32_t constexpr kStorageVersion = 10;                                       // v1.19.50
int32_t constexpr kNetworkVersion = 560;                                      // v1.19.50
char const *const kInventoryVersion = "1.19.50";                              // v1.19.50
uint8_t constexpr kDragonFightVersion = 0;                                    // v1.19.50

// for lastOpenedWithVersion of level.dat
Version constexpr kSupportVersion = Version(1, 19, 50, 2, 0); // v1.19.50

char constexpr kChunkVersion = 0x28;          // v1.19.50
char constexpr kChunkVersionMaxLegacy = 0x16; // v1.17.41

} // namespace je2be::tobe
