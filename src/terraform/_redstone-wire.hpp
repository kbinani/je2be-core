#pragma once

#include "_namespace.hpp"
#include "enums/_facing4.hpp"
#include "enums/_facing6.hpp"

namespace je2be::terraform {

class RedstoneWire {
  RedstoneWire() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasRedstoneWire) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (p != BlockPropertyAccessor::REDSTONE_WIRE) {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          map<u8string, optional<u8string>> props;

          // Looking for redstone connectable block in same Y.
          vector<pair<u8string, Pos2i>> const nesw({{u8"north", Pos2i(0, -1)}, {u8"east", Pos2i(1, 0)}, {u8"south", Pos2i(0, 1)}, {u8"west", Pos2i(-1, 0)}});
          for (auto d : nesw) {
            auto vec = d.second;
            auto block = cache.blockAt(x + vec.fX, y, z + vec.fZ);
            bool connect = false;
            if (block) {
              connect = IsRedstoneConnectable(*block, vec);
            }
            props[d.first] = connect ? u8"side" : u8"none";
          }

          // Check Y + 1, NESW blocks when the upper block is transparent against redstone wire.
          auto upperJ = cache.blockAt(x, y + 1, z);
          bool transparentUpper = true;
          if (upperJ) {
            transparentUpper = IsTransparentAgainstRedstoneWire(*upperJ);
          }
          if (transparentUpper) {
            for (auto d : nesw) {
              Pos2i vec = d.second;
              if (props[d.first] == u8"side") {
                continue;
              }
              auto block = cache.blockAt(x + vec.fX, y + 1, z + vec.fZ);
              if (block && block->fName == u8"minecraft:redstone_wire") {
                auto side = cache.blockAt(x + vec.fX, y, z + vec.fZ);
                if (side) {
                  if (side->fName.find(u8"_slab") != string::npos && side->fName.find(u8"double") == string::npos) {
                    props[d.first] = u8"side";
                  } else {
                    props[d.first] = u8"up";
                  }
                } else {
                  props[d.first] = u8"side";
                }
              }
            }
          }

          // Check Y - 1, NESW blocks when NESW block is transparent against redstone wire.
          for (auto d : nesw) {
            if (props[d.first] != u8"none") {
              continue;
            }
            Pos2i vec = d.second;
            auto sideJ = cache.blockAt(x + vec.fX, y, z + vec.fZ);
            bool transparentSide = true;
            if (sideJ) {
              transparentSide = IsTransparentAgainstRedstoneWire(*sideJ);
            }
            if (transparentSide) {
              auto lower = cache.blockAt(x + vec.fX, y - 1, z + vec.fZ);
              if (lower && lower->fId == mcfile::blocks::minecraft::redstone_wire) {
                props[d.first] = u8"side";
              }
            }
          }

          // Change "none" to "side", if there are 3 "none" and 1 "side"/"up" properties.
          int notNoneCount = 0;
          int noneCount = 0;
          u8string notNone;
          for (auto d : nesw) {
            if (props[d.first] == u8"none") {
              noneCount++;
            } else {
              notNoneCount++;
              notNone = d.first;
            }
          }
          if (notNoneCount == 1 && noneCount == 3) {
            if (notNone == u8"north") {
              props[u8"south"] = u8"side";
            } else if (notNone == u8"east") {
              props[u8"west"] = u8"side";
            } else if (notNone == u8"south") {
              props[u8"north"] = u8"side";
            } else if (notNone == u8"west") {
              props[u8"east"] = u8"side";
            }
          } else if (noneCount == 4) {
            for (auto it : nesw) {
              props[it.first] = u8"side";
            }
          }

          auto replace = blockJ->applying(props);
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }

  static bool IsRedstoneConnectable(mcfile::je::Block const &block, Pos2i direction) {
    using namespace std;
    using namespace mcfile::blocks::minecraft;
    auto id = block.fId;
    u8string_view name = Namespace::Remove(block.fName);
    if (id == lightning_rod || id == lectern || id == daylight_detector || id == detector_rail || id == redstone_torch || id == redstone_wall_torch || id == tripwire_hook || name.ends_with(u8"pressure_plate") || name.ends_with(u8"button") || id == lever || id == redstone_block || id == target || id == trapped_chest || id == redstone_wire || id == sculk_sensor || id == comparator) {
      return true;
    }
    if (id == repeater) {
      auto f4 = Facing4FromJavaName(block.property(u8"facing"));
      Pos2i d = Pos2iFromFacing4(f4);
      return (d.fX == -direction.fX && d.fZ == -direction.fZ) || (d.fX == direction.fX && d.fZ == direction.fZ);
    }
    if (id == observer) {
      auto f6 = Facing6FromJavaName(block.property(u8"facing"));
      Pos3i d3 = Pos3iFromFacing6(f6);
      return d3.fX == direction.fX && d3.fZ == direction.fZ;
    }
    return false;
  }

  static bool IsTransparentAgainstRedstoneWire(mcfile::je::Block const &block) {
    if (block.fName.ends_with(u8"slab")) {
      return block.property(u8"type", u8"") != u8"double";
    }
    return IsAlwaysTransparentAgainstRedstoneWire(block.fId);
  }

  static bool IsAlwaysTransparentAgainstRedstoneWire(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks::minecraft;
    switch (id) {
    case glowstone:
    case observer:
    case powder_snow:
    case redstone_block:
      // mcfile::blocks::IsTransparent(id) == false, but transparent against redstone wire
      return true;
    case barrier:
    case black_shulker_box:
    case blast_furnace:
    case blue_shulker_box:
    case brown_concrete:
    case brown_shulker_box:
    case cyan_shulker_box:
    case dropper:
    case furnace:
    case gray_shulker_box:
    case green_shulker_box:
    case jack_o_lantern:
    case light_blue_shulker_box:
    case light_gray_shulker_box:
    case lime_shulker_box:
    case magenta_shulker_box:
    case mangrove_roots:
    case muddy_mangrove_roots:
    case ochre_froglight:
    case orange_shulker_box:
    case pearlescent_froglight:
    case pink_shulker_box:
    case purple_shulker_box:
    case red_shulker_box:
    case redstone_lamp:
    case shulker_box:
    case slime_block:
    case smoker:
    case soul_sand:
    case spawner:
    case verdant_froglight:
    case water:
    case white_shulker_box:
    case yellow_shulker_box:
      // mcfile::blocks::IsTransparent(id) == true, but not transparent against redstone wire
      return false;
    }
    return mcfile::blocks::IsTransparent(id);
  }
};

} // namespace je2be::terraform
