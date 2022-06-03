#pragma once

namespace je2be::tobe {

int32_t constexpr kBlockDataVersion = 17959425;                               // TODO(1.19): preview 1.19.10.21
Version constexpr kMinimumCompatibleClientVersion = Version(1, 19, 10, 0, 0); // TODO(1.19): preview 1.19.10.21
char constexpr kSubChunkVersion = 0x27;
uint8_t constexpr kSubChunkBlockStorageVersion = 9;
int32_t constexpr kStorageVersion = 9;           // TODO(1.19): preview 1.19.10.21
int32_t constexpr kNetworkVersion = 532;         // TODO(1.19): preview 1.19.10.21
char const *const kInventoryVersion = "1.19.10"; // TODO(1.19): preview 1.19.10.21
uint8_t constexpr kDragonFightVersion = 0;

// for lastOpenedWithVersion of level.dat
Version constexpr kSupportVersion = Version(1, 19, 10, 21, 1); // TODO(1.19): preview 1.19.10.21

} // namespace je2be::tobe
