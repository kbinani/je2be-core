#pragma once

#include <minecraft-file.hpp>

namespace je2be::tobe {

class MovingPiston {
private:
  MovingPiston() = delete;
  class Impl;

public:
  static void PreprocessChunk(mcfile::je::CachedChunkLoader &loader, mcfile::je::Chunk &chunk);
};

} // namespace je2be::tobe
