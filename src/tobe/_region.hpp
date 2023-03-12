#pragma once

#include <je2be/integers.hpp>

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
class EntityStore;

class Region {
  class Impl;
  Region() = delete;

public:
  static std::shared_ptr<WorldData> Convert(
      mcfile::Dimension dim,
      std::shared_ptr<mcfile::je::Region> const &region,
      Options const &options,
      std::shared_ptr<EntityStore> const &entityStore,
      LevelData const &levelData,
      DbInterface &db,
      Progress *progress,
      std::atomic_uint32_t &done,
      u64 const numTotalChunks,
      std::atomic_bool &abortSignal,
      std::atomic_uint64_t &numConvertedChunks);
};

} // namespace je2be::tobe
