#pragma once

#include <je2be/nbt.hpp>

namespace je2be::lce {

class Context;

class Item {
  class Impl;
  Item() = delete;

public:
  static CompoundTagPtr Convert(CompoundTag const &in, Context const &ctx);
};

} // namespace je2be::lce
