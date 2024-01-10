#pragma once

namespace je2be::terraform::box360 {

class Bed {
  Bed() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockPropertyAccessor const &accessor) {
    using namespace std;
    if (!accessor.fHasBed) {
      return;
    }
    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (p != BlockPropertyAccessor::BED) {
            continue;
          }
          auto block = out.blockAt(x, y, z);
          if (!block) {
            continue;
          }
          if (out.tileEntityAt(x, y, z)) {
            continue;
          }
          auto tile = Compound();
          tile->set(u8"x", Int(x));
          tile->set(u8"y", Int(y));
          tile->set(u8"z", Int(z));
          tile->set(u8"id", u8"minecraft:bed");
          out.fTileEntities[{x, y, z}] = tile;
        }
      }
    }
  }
};

} // namespace je2be::terraform::box360
