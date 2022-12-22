#pragma once

#include <minecraft-file.hpp>

namespace je2be {

class DbInterface;

}

namespace je2be::tobe {

class LevelData;
class Progress;
class Options;

class World {
  class Impl;
  World() = delete;

public:
  [[nodiscard]] static bool Convert(
      mcfile::je::World const &w,
      mcfile::Dimension dim,
      DbInterface &db,
      LevelData &ld,
      unsigned int concurrency,
      Progress *progress,
      uint32_t &done,
      double const numTotalChunks,
      Options const &options);
};

} // namespace je2be::tobe
