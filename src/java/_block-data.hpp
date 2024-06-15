#pragma once

#include <memory>
#include <string>

#include <minecraft-file.hpp>

#include <je2be/nbt.hpp>

namespace je2be::java {

class BlockData {
public:
  struct Options {
    bool fItem = false;
  };
  static CompoundTagPtr From(std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagConstPtr const &tile, Options const &options);
  static CompoundTagPtr Air();
  static CompoundTagPtr Make(std::u8string const &name);
  static i32 GetFacingDirectionAFromFacing(mcfile::je::Block const &block);

private:
  class Impl;
  BlockData() = delete;
};

} // namespace je2be::java
