#pragma once

#include <je2be/status.hpp>

#include <minecraft-file.hpp>

namespace je2be::java {

class ChunkData;
class ChunkDataPackage;
class WorldData;

class SubChunk {
  class Impl;
  SubChunk() = delete;

public:
  [[nodiscard]] static Status Convert(
      mcfile::je::Chunk const &chunk,
      mcfile::Dimension dim,
      int chunkY,
      ChunkData &cd,
      ChunkDataPackage &cdp,
      WorldData &wd);
};

} // namespace je2be::java
