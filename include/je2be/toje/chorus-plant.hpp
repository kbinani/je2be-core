#pragma once

namespace je2be::toje {

class ChorusPlant {
  ChorusPlant() = delete;

public:
  static void Do(mcfile::je::Chunk &out, ChunkCache<3, 3> &cache, BlockPropertyAccessor const &accessor) {
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
          map<string, string> props(blockJ->fProperties);
          props["up"] = up ? "true" : "false";
          props["down"] = IsChorusPlantConnectable(down);
          props["north"] = IsChorusPlantConnectable(north);
          props["east"] = IsChorusPlantConnectable(east);
          props["south"] = IsChorusPlantConnectable(south);
          props["west"] = IsChorusPlantConnectable(west);
          auto replace = make_shared<mcfile::je::Block const>(blockJ->fName, props);
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }

private:
  static std::string IsChorusPlantConnectable(std::shared_ptr<mcfile::be::Block const> const &b) {
    if (!b) {
      return "false";
    }
    if (BlockPropertyAccessor::IsChorusPlant(*b)) {
      return "true";
    }
    if (b->fName == "minecraft:end_stone") {
      return "true";
    }
    return "false";
  }
};

} // namespace je2be::toje
