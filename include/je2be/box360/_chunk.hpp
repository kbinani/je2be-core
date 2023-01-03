#pragma once

#include <je2be/status.hpp>

#include <minecraft-file.hpp>

namespace je2be::box360 {

class Context;
class Options;

class Chunk {
  class Impl;
  Chunk() = delete;

public:
  enum {
    kTargetDataVersion = mcfile::je::Chunk::kDataVersion,
  };
  static std::string TargetVersionString() {
    return "1.19.3";
  }

  static Status Convert(mcfile::Dimension dimension,
                        std::filesystem::path const &region,
                        int cx,
                        int cz,
                        std::shared_ptr<mcfile::je::WritableChunk> &result,
                        Context const &ctx,
                        Options const &options);
};

} // namespace je2be::box360
