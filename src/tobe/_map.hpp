#pragma once

#include <je2be/nbt.hpp>

#include "db/_db-interface.hpp"

namespace je2be::tobe {

class Options;

class Map {
  class Impl;
  Map() = delete;

public:
  static i64 UUID(i32 javaMapId, u8 scale);

  static bool Convert(i32 javaMapId, CompoundTag const &item, std::filesystem::path const &input, Options const &opt, DbInterface &db);
};

} // namespace je2be::tobe
