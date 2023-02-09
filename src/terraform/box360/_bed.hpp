#pragma once

namespace je2be::terraform::box360 {

class Bed {
  Bed() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockPropertyAccessor const &accessor) {
    if (!accessor.fHasBed) {
      return;
    }
    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY(); y < accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (p != BlockPropertyAccessor::BED) {
            continue;
          }
          auto found = out.fTileEntities.find({x, y, z});
          if (found != out.fTileEntities.end()) {
            continue;
          }
          auto tag = Compound();
          tag->set("id", String("minecraft:bed"));
          tag->set("x", Int(x));
          tag->set("y", Int(y));
          tag->set("z", Int(z));
          out.fTileEntities[{x, y, z}] = tag;
        }
      }
    }
  }
};

} // namespace je2be::terraform::box360
