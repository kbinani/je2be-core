#pragma once

namespace je2be::tobe {
namespace {

int32_t constexpr kBlockDataVersion = 17879555;
Version constexpr kMinimumCompatibleClientVersion = Version(1, 16, 0, 0, 0);
char constexpr kSubChunkVersion = 0x16;
uint8_t constexpr kSubChunkBlockStorageVersion = 8;
int32_t constexpr kNetworkVersion = 408;
char const *const kInventoryVersion = "1.16.40";
uint8_t constexpr kDragonFightVersion = 0;

Version kSupportVersion = Version(1, 16, 40, 2, 0);

} // namespace
} // namespace je2be::tobe
