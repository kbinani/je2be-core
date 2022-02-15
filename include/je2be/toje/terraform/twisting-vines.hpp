#pragma once

namespace je2be::toje {

class TwistingVines {
  TwistingVines() = delete;

public:
  static void Do(mcfile::je::Chunk &out, ChunkCache<3, 3> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasTwistingVines) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY(); y < accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          auto upper = accessor.property(x, y + 1, z);
          if (BlockPropertyAccessor::IsTwistingVines(p) && BlockPropertyAccessor::IsTwistingVines(upper)) {
            auto plant = make_shared<mcfile::je::Block const>("minecraft:twisting_vines_plant");
            out.setBlockAt(x, y, z, plant);
          }
        }
      }
    }
  }
};

} // namespace je2be::toje