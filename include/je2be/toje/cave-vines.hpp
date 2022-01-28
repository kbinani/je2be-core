#pragma once

namespace je2be::toje {

class CaveVines {
  CaveVines() = delete;

public:
  static void Do(mcfile::je::Chunk &out, ChunkCache<3, 3> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasCaveVines) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = mcfile::be::Chunk::kMinBlockY + 1; y <= mcfile::be::Chunk::kMaxBlockY; y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          auto lower = accessor.property(x, y - 1, z);
          if (BlockPropertyAccessor::IsCaveVines(p) && BlockPropertyAccessor::IsCaveVines(lower)) {
            auto blockJ = out.blockAt(x, y, z);
            if (!blockJ) {
              continue;
            }
            map<string, string> props(blockJ->fProperties);
            props.erase("age");
            auto plant = make_shared<mcfile::je::Block const>("minecraft:cave_vines_plant", props);
            out.setBlockAt(x, y, z, plant);
          }
        }
      }
    }
  }
};

} // namespace je2be::toje
