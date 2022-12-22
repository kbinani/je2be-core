#pragma once

#include <je2be/nbt.hpp>

namespace je2be::tobe {

struct Context;

class Item {
  class Impl;

public:
  static CompoundTagPtr From(CompoundTagPtr const &item, Context const &ctx);
  static int8_t GetSkullTypeFromBlockName(std::string_view const &name);
  static CompoundTagPtr Empty();
};

} // namespace je2be::tobe
