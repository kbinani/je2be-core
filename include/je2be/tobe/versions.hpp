#pragma once

namespace je2be::tobe {

int32_t constexpr kBlockDataVersion = 17959425;
Version constexpr kMinimumCompatibleClientVersion = Version(1, 18, 0, 0, 0);
char constexpr kSubChunkVersion = 0x27;
uint8_t constexpr kSubChunkBlockStorageVersion = 9;
int32_t constexpr kStorageVersion = 8;
int32_t constexpr kNetworkVersion = 475;
char const *const kInventoryVersion = "1.18.0";
uint8_t constexpr kDragonFightVersion = 0;

Version constexpr kSupportVersion = Version(1, 18, 0, 2, 0);

} // namespace je2be::tobe
