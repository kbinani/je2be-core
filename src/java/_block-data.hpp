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
  static bool IsUpdate121Block(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks::minecraft;
    switch (id) {
    case tuff_stairs:
    case tuff_slab:
    case tuff_wall:
    case chiseled_tuff:
    case polished_tuff:
    case polished_tuff_stairs:
    case polished_tuff_slab:
    case polished_tuff_wall:
    case tuff_bricks:
    case tuff_brick_stairs:
    case tuff_brick_slab:
    case tuff_brick_wall:
    case chiseled_tuff_bricks:
    case trial_spawner:
    case crafter:
    case chiseled_copper:
    case exposed_chiseled_copper:
    case weathered_chiseled_copper:
    case oxidized_chiseled_copper:
    case waxed_chiseled_copper:
    case waxed_exposed_chiseled_copper:
    case waxed_weathered_chiseled_copper:
    case waxed_oxidized_chiseled_copper:
    case copper_grate:
    case exposed_copper_grate:
    case weathered_copper_grate:
    case oxidized_copper_grate:
    case waxed_copper_grate:
    case waxed_exposed_copper_grate:
    case waxed_weathered_copper_grate:
    case waxed_oxidized_copper_grate:
    case copper_bulb:
    case exposed_copper_bulb:
    case weathered_copper_bulb:
    case oxidized_copper_bulb:
    case waxed_copper_bulb:
    case waxed_exposed_copper_bulb:
    case waxed_weathered_copper_bulb:
    case waxed_oxidized_copper_bulb:
    case copper_door:
    case exposed_copper_door:
    case weathered_copper_door:
    case oxidized_copper_door:
    case waxed_copper_door:
    case waxed_exposed_copper_door:
    case waxed_weathered_copper_door:
    case waxed_oxidized_copper_door:
    case copper_trapdoor:
    case exposed_copper_trapdoor:
    case weathered_copper_trapdoor:
    case oxidized_copper_trapdoor:
    case waxed_copper_trapdoor:
    case waxed_exposed_copper_trapdoor:
    case waxed_weathered_copper_trapdoor:
    case waxed_oxidized_copper_trapdoor:
    case vault:
    case heavy_core:
      return true;
    }
    return false;
  }

private:
  class Impl;
  BlockData() = delete;
};

} // namespace je2be::java
