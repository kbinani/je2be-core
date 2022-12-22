#pragma once

#include <je2be/enums/chunk-conversion-mode.hpp>

#include <minecraft-file.hpp>

namespace je2be::toje {

class SubChunk {
  SubChunk() = delete;
  class Impl;

public:
  static std::shared_ptr<mcfile::je::ChunkSection> Convert(mcfile::be::SubChunk const &sectionB, mcfile::Dimension dim, ChunkConversionMode mode);
};

} // namespace je2be::toje
