#pragma once

#include <je2be/nbt.hpp>

namespace je2be::tobe {

struct Context;

class Item {
  class Impl;

public:
  static CompoundTagPtr From(CompoundTagPtr const &item, Context &ctx, int sourceDataVersion);
  static i8 GetSkullTypeFromBlockName(std::u8string_view const &name);
  static CompoundTagPtr Empty();
};

} // namespace je2be::tobe
