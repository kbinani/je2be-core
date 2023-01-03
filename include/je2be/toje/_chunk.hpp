#pragma once

#include <minecraft-file.hpp>

#include <je2be/terraform/bedrock/_block-accessor-bedrock.hpp>

namespace je2be::toje {

class Context;

class Chunk {
  Chunk() = delete;
  class Impl;

public:
  static std::shared_ptr<mcfile::je::WritableChunk> Convert(mcfile::Dimension d, int cx, int cz, mcfile::be::Chunk const &b, terraform::bedrock::BlockAccessorBedrock<3, 3> &cache, Context &ctx);
};

} // namespace je2be::toje
