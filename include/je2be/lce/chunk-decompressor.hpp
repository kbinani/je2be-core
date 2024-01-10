#pragma once

#include <je2be/status.hpp>

namespace je2be::lce {

class ChunkDecompressor {
public:
  virtual ~ChunkDecompressor() {}
  virtual Status decompress(std::vector<uint8_t> &buffer) const = 0;
};

} // namespace je2be::lce
