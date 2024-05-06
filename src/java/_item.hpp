#pragma once

#include <je2be/nbt.hpp>

namespace je2be::java {

struct Context;

class Item {
  class Impl;

public:
  static CompoundTagPtr From(CompoundTagPtr const &item, Context &ctx, int sourceDataVersion);
  static i8 GetSkullTypeFromBlockName(std::u8string_view const &name);
  static CompoundTagPtr Empty();

  static std::optional<i32> Count(CompoundTag const &c) {
    if (auto count = c.int32(u8"count"); count) {
      // java 1.20.5 or later
      return *count;
    } else if (auto legacyCount = c.byte(u8"Count"); legacyCount) {
      return *legacyCount;
    } else {
      return std::nullopt;
    }
  }

  static i32 Count(CompoundTag const &c, i32 def) {
    if (auto count = Count(c); count) {
      return *count;
    } else {
      return def;
    }
  }
};

} // namespace je2be::java
