#pragma once

namespace je2be::terraform {

class ChorusPlant {
  ChorusPlant() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor &cache, BlockPropertyAccessor const &accessor) {
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
          if (!BlockPropertyAccessor::IsChorusPlant(p)) {
            continue;
          }
          auto blockB = cache.blockAt(x, y, z);
          if (!blockB) {
            continue;
          }
          if (blockB->fName == "minecraft:chorus_flower") {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          auto up = BlockPropertyAccessor::IsChorusPlant(accessor.property(x, y + 1, z));
          auto down = cache.blockAt(x, y - 1, z);
          auto north = cache.blockAt(x, y, z - 1);
          auto east = cache.blockAt(x + 1, y, z);
          auto south = cache.blockAt(x, y, z + 1);
          auto west = cache.blockAt(x - 1, y, z);

          auto replace = blockJ->applying({
              {"up", up ? "true" : "false"},
              {"down", IsChorusPlantConnectable(down)},
              {"north", IsChorusPlantConnectable(north)},
              {"east", IsChorusPlantConnectable(east)},
              {"south", IsChorusPlantConnectable(south)},
              {"west", IsChorusPlantConnectable(west)},
          });
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }

private:
  static std::string IsChorusPlantConnectable(std::shared_ptr<mcfile::je::Block const> const &b) {
    if (!b) {
      return "false";
    }
    if (BlockPropertyAccessor::IsChorusPlant(*b)) {
      return "true";
    }
    if (b->fId == mcfile::blocks::minecraft::end_stone) {
      return "true";
    }
    return "false";
  }
};

} // namespace je2be::terraform
