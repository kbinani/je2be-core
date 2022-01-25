#pragma once

namespace je2be::toje {

class Snowy {
  Snowy() = delete;

public:
  static void Do(mcfile::je::Chunk &out, ChunkCache<3, 3> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasSnowy) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = mcfile::be::Chunk::kMinChunkY * 16; y < mcfile::be::Chunk::kMaxChunkY * 16 - 1; y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (!BlockPropertyAccessor::IsSnowy(p)) {
            continue;
          }
          auto upper = cache.blockAt(x, y + 1, z);
          if (!upper) {
            continue;
          }
          if (upper->fName != "minecraft:snow") {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          map<string, string> props(blockJ->fProperties);
          props["snowy"] = "true";
          auto replace = make_shared<mcfile::je::Block const>(blockJ->fName, props);
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }
};

} // namespace je2be::toje
