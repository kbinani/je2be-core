#pragma once

#include "terraform/_block-accessor.hpp"
#include "terraform/_block-property-accessor.hpp"

namespace je2be::terraform {

class ChorusPlant {
  ChorusPlant() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasChorusPlant) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (p != BlockPropertyAccessor::CHORUS_PLANT) {
            continue;
          }
          auto blockB = cache.blockAt(x, y, z);
          if (!blockB) {
            continue;
          }
          if (blockB->fName == u8"minecraft:chorus_flower") {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          auto up = accessor.property(x, y + 1, z) == BlockPropertyAccessor::CHORUS_PLANT;
          auto down = cache.blockAt(x, y - 1, z);
          auto north = cache.blockAt(x, y, z - 1);
          auto east = cache.blockAt(x + 1, y, z);
          auto south = cache.blockAt(x, y, z + 1);
          auto west = cache.blockAt(x - 1, y, z);

          auto replace = blockJ->applying({
              {u8"up", up ? u8"true" : u8"false"},
              {u8"down", IsChorusPlantConnectable(down)},
              {u8"north", IsChorusPlantConnectable(north)},
              {u8"east", IsChorusPlantConnectable(east)},
              {u8"south", IsChorusPlantConnectable(south)},
              {u8"west", IsChorusPlantConnectable(west)},
          });
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }

private:
  static std::u8string IsChorusPlantConnectable(std::shared_ptr<mcfile::je::Block const> const &b) {
    if (!b) {
      return u8"false";
    }
    if (BlockPropertyAccessor::IsChorusPlant(*b)) {
      return u8"true";
    }
    if (b->fId == mcfile::blocks::minecraft::end_stone) {
      return u8"true";
    }
    return u8"false";
  }
};

} // namespace je2be::terraform
