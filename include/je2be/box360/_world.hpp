#pragma once

#include <je2be/status.hpp>

#include <minecraft-file.hpp>

namespace je2be::box360 {

class Context;
class Options;
class Progress;

class World {
  class Impl;
  World() = delete;

public:
  static Status Convert(std::filesystem::path const &levelRootDirectory,
                        std::filesystem::path const &outputDirectory,
                        mcfile::Dimension dimension,
                        unsigned int concurrency,
                        Context const &ctx,
                        Options const &options,
                        Progress *progress,
                        int progressChunksOffset);
};

} // namespace je2be::box360
