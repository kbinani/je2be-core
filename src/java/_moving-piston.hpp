#pragma once

#include <minecraft-file.hpp>

namespace je2be {

struct DataVersion;

namespace java {

class MovingPiston {
private:
  MovingPiston() = delete;
  class Impl;

public:
  static void PreprocessChunk(mcfile::je::CachedChunkLoader &loader, mcfile::je::Chunk &chunk, DataVersion const &dataVersion);
};

} // namespace java
} // namespace je2be
