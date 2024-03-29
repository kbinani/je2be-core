#pragma once

#include "terraform/_block-property-accessor.hpp"

namespace je2be::bedrock {

class Campfire {
  Campfire() = delete;

public:
  static void Do(mcfile::je::Chunk &out, terraform::bedrock::BlockAccessorBedrock<3, 3> &cache, terraform::BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasCampfire) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY() + 1; y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (p != terraform::BlockPropertyAccessor::CAMPFIRE) {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          map<u8string, optional<u8string>> props;
          props[u8"signal_fire"] = u8"false";
          auto lower = cache.blockAt(x, y - 1, z);
          if (lower) {
            if (lower->fName == u8"minecraft:hay_block") {
              props[u8"signal_fire"] = u8"true";
            }
          }
          auto replace = blockJ->applying(props);
          mcfile::je::SetBlockOptions o;
          o.fRemoveTileEntity = false;
          out.setBlockAt(x, y, z, replace, o);
        }
      }
    }
  }
};

} // namespace je2be::bedrock
