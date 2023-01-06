#pragma once

#include <je2be/pos2.hpp>

#include <atomic>

namespace je2be::toje {

class Context;

class Region {
  class Impl;
  Region() = delete;

public:
  static std::shared_ptr<Context> Convert(mcfile::Dimension d,
                                          std::unordered_set<Pos2i, Pos2iHasher> chunks,
                                          int rx,
                                          int rz,
                                          mcfile::be::DbInterface *db,
                                          std::filesystem::path destination,
                                          Context const &parentContext,
                                          std::function<bool(void)> progress,
                                          std::atomic_uint64_t &numConvertedChunks);
};

} // namespace je2be::toje
