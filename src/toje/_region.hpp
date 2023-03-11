#pragma once

#include <je2be/pos2.hpp>

#include "_pos2i-set.hpp"

#include <atomic>

namespace je2be::toje {

class Context;

class Region {
  class Impl;
  Region() = delete;

public:
  static std::shared_ptr<Context> Convert(mcfile::Dimension d,
                                          Pos2iSet chunks,
                                          Pos2i region,
                                          unsigned int concurrency,
                                          mcfile::be::DbInterface *db,
                                          std::filesystem::path destination,
                                          Context const &parentContext,
                                          std::function<bool(void)> progress,
                                          std::atomic_uint64_t &numConvertedChunks,
                                          std::filesystem::path terrainTempDir);
};

} // namespace je2be::toje
