#pragma once

#include <je2be/status.hpp>

namespace je2be::lce {

class Options;
class Progress;
class ChunkDecompressor;

class Converter {
  class Impl;
  Converter() = delete;

public:
  static Status Run(std::vector<uint8_t> const &inputSavegame,
                    std::filesystem::path const &outputDirectory,
                    unsigned int concurrency,
                    ChunkDecompressor const &chunkDecompressor,
                    Options const &options,
                    Progress *progress = nullptr);
};

} // namespace je2be::lce
