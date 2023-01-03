#pragma once

namespace je2be::terraform {

class Snowy {
  Snowy() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasSnowy) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY(); y < accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (p != BlockPropertyAccessor::SNOWY) {
            continue;
          }
          auto upper = cache.blockAt(x, y + 1, z);
          if (!upper) {
            continue;
          }
          if (upper->fId != mcfile::blocks::minecraft::snow) {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          auto replace = blockJ->applying({{"snowy", "true"}});
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }
};

} // namespace je2be::terraform
