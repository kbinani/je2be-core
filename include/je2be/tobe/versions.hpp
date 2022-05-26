#pragma once

namespace je2be::tobe {

int32_t constexpr kBlockDataVersion = 17959425;                               // preview 1.19.10.20
Version constexpr kMinimumCompatibleClientVersion = Version(1, 19, 10, 0, 0); // preview 1.19.10.20
char constexpr kSubChunkVersion = 0x27;
uint8_t constexpr kSubChunkBlockStorageVersion = 9;
int32_t constexpr kStorageVersion = 9;           // preview 1.19.10.20
int32_t constexpr kNetworkVersion = 530;         // preview 1.19.10.20
char const *const kInventoryVersion = "1.19.10"; // preview 1.19.10.20
uint8_t constexpr kDragonFightVersion = 0;

Version constexpr kSupportVersion = Version(1, 19, 10, 20, 1); // preview 1.19.10.20

} // namespace je2be::tobe
