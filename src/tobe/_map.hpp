#pragma once

#include <je2be/nbt.hpp>

#include "db/_db-interface.hpp"

namespace je2be::tobe {

class Options;

class Map {
  class Impl;
  Map() = delete;

public:
  static int64_t UUID(int32_t javaMapId, uint8_t scale);

  static bool Convert(int32_t javaMapId, CompoundTag const &item, std::filesystem::path const &input, Options const &opt, DbInterface &db);
};

} // namespace je2be::tobe
