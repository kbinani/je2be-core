#pragma once

#include <je2be/pos2.hpp>

#include <minecraft-file.hpp>

namespace je2be {
class DbInterface;
}

namespace je2be::tobe {

class EntityStore;

class World {
  class Impl;
  World() = delete;

public:
  static bool PutWorldEntities(
      mcfile::Dimension d,
      DbInterface &db,
      std::shared_ptr<EntityStore> const &entityStore,
      unsigned int concurrency);
};

} // namespace je2be::tobe
