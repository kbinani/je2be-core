#pragma once

#include <je2be/pos2.hpp>

namespace leveldb {
class DB;
}

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
                                          leveldb::DB *db,
                                          std::filesystem::path destination,
                                          Context const &parentContext,
                                          std::function<bool(void)> progress);
};

} // namespace je2be::toje