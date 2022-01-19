#pragma once

namespace je2be::toje {

class Region {
public:
  std::unordered_set<Pos2i, Pos2iHasher> fChunks;

  static bool Convert(std::unordered_set<Pos2i, Pos2iHasher> const &chunks, int rx, int rz, leveldb::DB &db) {
    //TODO:
    return true;
  }
};

} // namespace je2be::toje
