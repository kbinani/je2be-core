#pragma once

#include <je2be/nbt.hpp>

#include "_props.hpp"

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

  static std::optional<std::u8string> GetCustomName(CompoundTag const &itemJ) {
    using namespace std;
    using namespace je2be::props;
    optional<u8string> name;
    if (auto components = itemJ.compoundTag(u8"components"); components) {
      if (auto n = components->string(u8"minecraft:custom_name"); n) {
        name = *n;
      } else {
        return nullopt;
      }
    } else if (auto tag = itemJ.compoundTag(u8"tag"); tag) {
      auto display = tag->compoundTag(u8"display");
      if (!display) {
        return nullopt;
      }
      if (auto n = display->string(u8"Name"); n) {
        name = *n;
      } else {
        return nullopt;
      }
    }
    if (!name) {
      return nullopt;
    }
    auto obj = ParseAsJson(*name);
    if (obj) {
      auto text = obj->find("text");
      if (text != obj->end() && text->is_string()) {
        return props::GetJsonStringValue(*text);
      } else {
        return nullopt;
      }
    } else {
      return strings::RemovePrefixAndSuffix(u8"\"", *name, u8"\"");
    }
  }
};

} // namespace je2be::java
