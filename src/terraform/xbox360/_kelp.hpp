#pragma once

namespace je2be::terraform::box360 {

class Kelp {
  Kelp() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockPropertyAccessor const &accessor) {
    if (!accessor.fHasKelp) {
      return;
    }
    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY(); y < accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          auto upper = accessor.property(x, y + 1, z);
          if (p == BlockPropertyAccessor::KELP && upper != BlockPropertyAccessor::KELP) {
            auto kelpPlant = mcfile::je::Block::FromId(mcfile::blocks::minecraft::kelp, out.getDataVersion());
            out.setBlockAt(x, y, z, kelpPlant);
          }
        }
      }
    }
  }
};

} // namespace je2be::terraform::box360
