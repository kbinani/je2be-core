#pragma once

#include <je2be/integers.hpp>
#include <je2be/pos2.hpp>
#include <je2be/status.hpp>

#include <minecraft-file.hpp>

#include <atomic>

namespace je2be {
class DbInterface;
}

namespace je2be::java {

class EntityStore;
class Progress;

class World {
  class Impl;
  World() = delete;

public:
  static Status PutWorldEntities(
      mcfile::Dimension d,
      DbInterface &db,
      std::shared_ptr<EntityStore> const &entityStore,
      unsigned int concurrency,
      Progress *progress,
      std::atomic<u64> &done,
      u64 total);
};

} // namespace je2be::java
