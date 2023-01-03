#pragma once

#include <memory>
#include <string>

#include <minecraft-file.hpp>

#include <je2be/nbt.hpp>

namespace je2be::tobe {

class BlockData {
public:
  static CompoundTagPtr From(std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagConstPtr const &tile);
  static CompoundTagPtr Air();
  static CompoundTagPtr Make(std::string const &name);
  static int32_t GetFacingDirectionAFromFacing(mcfile::je::Block const &block);

private:
  class Impl;
  BlockData() = delete;
};

} // namespace je2be::tobe
