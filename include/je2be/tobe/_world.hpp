#pragma once

#include <minecraft-file.hpp>

namespace je2be {
class DbInterface;
}

namespace je2be::tobe {

class World {
  class Impl;
  World() = delete;

public:
  static bool PutWorldEntities(mcfile::Dimension d, DbInterface &db, std::filesystem::path temp, unsigned int concurrency);
};

} // namespace je2be::tobe
