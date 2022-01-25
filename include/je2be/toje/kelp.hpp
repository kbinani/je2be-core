#pragma once

namespace je2be::toje {

class Kelp {
  Kelp() = delete;

public:
  static void Do(mcfile::je::Chunk &out, ChunkCache<3, 3> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasKelp) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = mcfile::be::Chunk::kMinBlockY; y < mcfile::be::Chunk::kMaxBlockY; y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          auto upper = accessor.property(x, y + 1, z);
          if (BlockPropertyAccessor::IsKelp(p) && BlockPropertyAccessor::IsKelp(upper)) {
            auto kelpPlant = make_shared<mcfile::je::Block const>("minecraft:kelp_plant");
            out.setBlockAt(x, y, z, kelpPlant);
          }
        }
      }
    }
  }
};

} // namespace je2be::toje
