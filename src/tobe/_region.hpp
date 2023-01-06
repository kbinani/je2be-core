#pragma once

#include <minecraft-file.hpp>

#include <atomic>

namespace je2be {
class DbInterface;
}

namespace je2be::tobe {

class LevelData;
class Progress;
class WorldData;
class Options;

class Region {
  class Impl;
  Region() = delete;

public:
  static std::shared_ptr<WorldData> Convert(
      mcfile::Dimension dim,
      std::shared_ptr<mcfile::je::Region> const &region,
      Options const &options,
      std::filesystem::path const &worldTempDir,
      LevelData const &levelData,
      DbInterface &db,
      Progress *progress,
      std::atomic_uint32_t &done,
      double const numTotalChunks,
      std::atomic_bool &abortSignal,
      std::atomic_uint64_t &numConvertedChunks);
};

} // namespace je2be::tobe
