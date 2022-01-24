#pragma once

namespace je2be::toje {

class WeepingVines {
  WeepingVines() = delete;

public:
  static void Do(mcfile::je::Chunk &out, ChunkCache<3, 3> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = mcfile::be::Chunk::kMinChunkY * 16 + 1; y < mcfile::be::Chunk::kMaxChunkY * 16; y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          auto lower = accessor.property(x, y - 1, z);
          if (BlockPropertyAccessor::IsWeepingVines(p) && BlockPropertyAccessor::IsWeepingVines(lower)) {
            auto plant = make_shared<mcfile::je::Block const>("minecraft:weeping_vines_plant");
            out.setBlockAt(x, y, z, plant);
          }
        }
      }
    }
  }
};

} // namespace je2be::toje
