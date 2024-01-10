#pragma once

#include <je2be/lce/chunk-decompressor.hpp>

#include "lce/_savegame.hpp"

namespace je2be::box360 {

class ChunkDecompressor : public je2be::lce::ChunkDecompressor {
public:
  Status decompress(std::vector<uint8_t> &buffer) const override {
    if (!je2be::lce::Savegame::DecompressRawChunk(buffer)) {
      return JE2BE_ERROR;
    }
    return Status::Ok();
  }
};

} // namespace je2be::box360
