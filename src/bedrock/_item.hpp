#pragma once

#include <je2be/nbt.hpp>

namespace je2be::bedrock {

class Context;

class Item {
public:
  struct Options {
    bool fOfferItem = false;
  };
  using Converter = std::function<std::u8string(std::u8string const &, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &)>;

private:
  class Impl;
  Item() = delete;

public:
  static CompoundTagPtr From(CompoundTag const &tagB, Context &ctx, int dataVersion, Options const &opt);
};

} // namespace je2be::bedrock
