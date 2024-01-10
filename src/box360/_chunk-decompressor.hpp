#pragma once

#include <je2be/lce/chunk-decompressor.hpp>

#include "lce/_lzx-decoder.hpp"

namespace je2be::box360 {

class ChunkDecompressor : public je2be::lce::ChunkDecompressor {
public:
  Status decompress(std::vector<uint8_t> &buffer) const override {
    if (buffer.size() < 4) {
      return JE2BE_ERROR;
    }
    // u32 decompressedSize = mcfile::U32FromBE(Mem::Read<u32>(buffer, 0));
    buffer.erase(buffer.begin(), buffer.begin() + 4);
    size_t decodedSize = lce::detail::LzxDecoder::Decode(buffer);
    if (decodedSize == 0) {
      return JE2BE_ERROR;
    } else {
      return Status::Ok();
    }
  }
};

} // namespace je2be::box360
