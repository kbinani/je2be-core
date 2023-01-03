#pragma once

#include <je2be/pos2.hpp>

#include <minecraft-file.hpp>

namespace je2be {
class DbInterface;
}

namespace je2be::tobe {

class World {
  class Impl;
  World() = delete;

public:
  static bool PutWorldEntities(
      mcfile::Dimension d,
      DbInterface &db,
      std::unordered_map<Pos2i, std::vector<std::filesystem::path>, Pos2iHasher> const &files,
      unsigned int concurrency);
};

} // namespace je2be::tobe
