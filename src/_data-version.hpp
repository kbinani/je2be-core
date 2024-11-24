#pragma once

#include "enums/_chunk-conversion-mode.hpp"

namespace je2be {

constexpr int kJavaDataVersion = 4179;          // 1.21.4pre1 //TODO(1.21.4)
constexpr int kJavaDataVersionMaxLegacy = 2730; // 1.17.1

constexpr int kJavaDataVersionComponentIntroduced = 3819; // 24w09a

struct DataVersion {
  int32_t const fSource;
  int32_t const fTarget;

  static int32_t TargetVersionByMode(ChunkConversionMode mode) {
    switch (mode) {
    case ChunkConversionMode::Legacy:
      return kJavaDataVersionMaxLegacy;
    case ChunkConversionMode::CavesAndCliffs2:
    default:
      return kJavaDataVersion;
    }
  }

  DataVersion(int32_t source, ChunkConversionMode mode) : fSource(source), fTarget(TargetVersionByMode(mode)) {}
  DataVersion(int32_t source, int32_t target) : fSource(source), fTarget(target) {}
};

} // namespace je2be
