#pragma once

#include <je2be/status.hpp>

#include <minecraft-file.hpp>

#include "terraform/bedrock/_block-accessor-bedrock.hpp"

namespace je2be::bedrock {

class Context;

class Chunk {
  Chunk() = delete;
  class Impl;

public:
  static Status Convert(mcfile::Dimension d,
                        int cx,
                        int cz,
                        mcfile::be::Chunk const &b,
                        terraform::bedrock::BlockAccessorBedrock<3, 3> &cache,
                        Context &ctx,
                        std::shared_ptr<mcfile::je::WritableChunk> &out);
};

} // namespace je2be::bedrock
