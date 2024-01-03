#pragma once

#include <je2be/integers.hpp>
#include <je2be/status.hpp>

#include <minecraft-file.hpp>

namespace je2be::lce {

class Context;
class Options;
class Progress;

class World {
  class Impl;
  World() = delete;

public:
  enum {
    kLengthOverworldRegions = 3,
    kLengthNetherRegions = 3,
    kLengthEndRegions = 1,

    // chunks * (convert + per-chunk-terraform + concat + per-region-terraform)
    kProgressWeightOverworld = 32 * 32 * (kLengthOverworldRegions * 2 * kLengthOverworldRegions * 2) * 4,
    kProgressWeightNether = 32 * 32 * (kLengthNetherRegions * 2 * kLengthNetherRegions * 2) * 4,
    kProgressWeightEnd = 32 * 32 * (kLengthEndRegions * 2 * kLengthEndRegions * 2) * 4,

    kProgressWeightTotal = kProgressWeightOverworld + kProgressWeightNether + kProgressWeightEnd,
  };

  static int LengthRegions(mcfile::Dimension d) {
    switch (d) {
    case mcfile::Dimension::Overworld:
      return kLengthOverworldRegions;
    case mcfile::Dimension::Nether:
      return kLengthNetherRegions;
    case mcfile::Dimension::End:
      return kLengthEndRegions;
    default:
      return 0;
    }
  }

  static int ProgressWeight(mcfile::Dimension d) {
    switch (d) {
    case mcfile::Dimension::Overworld:
      return kProgressWeightOverworld;
    case mcfile::Dimension::Nether:
      return kProgressWeightNether;
    case mcfile::Dimension::End:
      return kProgressWeightEnd;
    default:
      return 0;
    }
  }

  static Status Convert(std::filesystem::path const &levelRootDirectory,
                        std::filesystem::path const &outputDirectory,
                        mcfile::Dimension dimension,
                        unsigned int concurrency,
                        Context const &ctx,
                        Options const &options,
                        Progress *progress,
                        u64 progressChunksOffset);
};

} // namespace je2be::lce
