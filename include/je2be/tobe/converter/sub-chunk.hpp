#pragma once

#include <minecraft-file.hpp>

namespace je2be::tobe {

class ChunkData;
class ChunkDataPackage;
class WorldData;

class SubChunk {
  class Impl;
  SubChunk() = delete;

public:
  [[nodiscard]] static bool Convert(mcfile::je::Chunk const &chunk,
                                    mcfile::Dimension dim,
                                    int chunkY,
                                    ChunkData &cd,
                                    ChunkDataPackage &cdp,
                                    WorldData &wd);
};

} // namespace je2be::tobe
