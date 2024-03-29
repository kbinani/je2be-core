#pragma once

#include "enums/_chunk-conversion-mode.hpp"

#include <minecraft-file.hpp>

namespace je2be::bedrock {

class SubChunk {
  SubChunk() = delete;
  class Impl;

public:
  static std::shared_ptr<mcfile::je::ChunkSection> Convert(mcfile::be::SubChunk const &sectionB, mcfile::Dimension dim, ChunkConversionMode mode, int dataVersion);
};

} // namespace je2be::bedrock
