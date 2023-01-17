#pragma once

#include <je2be/nbt.hpp>
#include <je2be/status.hpp>

#include "toje/_context.hpp"

#include <atomic>

namespace je2be::toje {

class World {
  class Impl;
  World() = delete;

public:
  static Status Convert(mcfile::Dimension d,
                        std::vector<std::pair<Pos2i, Context::ChunksInRegion>> const &regions,
                        mcfile::be::DbInterface &db,
                        std::filesystem::path root,
                        unsigned concurrency,
                        Context const &parentContext,
                        std::shared_ptr<Context> &resultContext,
                        std::function<bool(void)> progress,
                        std::atomic_uint64_t &numConvertedChunks,
                        std::filesystem::path terrainTempDir);
};

} // namespace je2be::toje
