#pragma once

#include <je2be/status.hpp>

#include <minecraft-file.hpp>

namespace je2be::lce {

class Context;
class Options;

class Chunk {
  class Impl;
  Chunk() = delete;

public:
  enum {
    kTargetDataVersion = 2586, // 1.16.5
  };
  static std::u8string TargetVersionString() {
    return u8"1.16.5";
  }

  static Status Convert(mcfile::Dimension dimension,
                        std::filesystem::path const &region,
                        int cx,
                        int cz,
                        std::shared_ptr<mcfile::je::WritableChunk> &result,
                        Context const &ctx,
                        Options const &options);

  static std::shared_ptr<mcfile::je::WritableChunk> CreateEmptyChunk(mcfile::Dimension dim, int cx, int cz, bool newSeaLevel);
};

} // namespace je2be::lce
