#pragma once

#include <je2be/lce/chunk-decompressor.hpp>

namespace je2be::ps3 {

class ChunkDecompressor : public je2be::lce::ChunkDecompressor {
public:
  Status decompress(std::vector<uint8_t> &buffer) const override {
    if (buffer.size() < 9) {
      return JE2BE_ERROR;
    }
    std::vector<uint8_t> decoded;
    if (!mcfile::Compression::DecompressDeflate<decltype(decoded)>(buffer.data() + 8, buffer.size() - 8, decoded)) {
      return JE2BE_ERROR;
    }
    decoded.swap(buffer);
    return Status::Ok();
  }
};

} // namespace je2be::ps3
