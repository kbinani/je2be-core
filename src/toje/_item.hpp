#pragma once

#include <je2be/nbt.hpp>

namespace je2be::toje {

class Context;

class Item {
  class Impl;
  Item() = delete;
  using Converter = std::function<std::u8string(std::u8string const &, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx)>;

public:
  static CompoundTagPtr From(CompoundTag const &tagB, Context &ctx);
};

} // namespace je2be::toje
