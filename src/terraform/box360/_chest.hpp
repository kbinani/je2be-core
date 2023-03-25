#pragma once

#include "enums/_facing4.hpp"

namespace je2be::terraform::box360 {

class Chest {
  Chest() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;
    if (!accessor.fHasChest) {
      return;
    }
    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (p != BlockPropertyAccessor::CHEST) {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          auto type = blockJ->property(u8"type");
          if (type == u8"left" || type == u8"right") {
            continue;
          }
          type = u8"single";

          auto f4 = Facing4FromJavaName(blockJ->property(u8"facing"));
          auto facing = Pos2iFromFacing4(f4);
          map<u8string, optional<u8string>> props;

          Pos2i left = Pos2i(x, z) + Left90(facing);
          if (auto leftBlock = cache.blockAt(left.fX, y, left.fZ); leftBlock) {
            if (leftBlock->fId == blockJ->fId && leftBlock->property(u8"facing") == blockJ->property(u8"facing")) {
              type = u8"right";
            }
          }

          Pos2i right = Pos2i(x, z) + Right90(facing);
          if (auto rightBlock = cache.blockAt(right.fX, y, right.fZ); rightBlock) {
            if (rightBlock->fId == blockJ->fId && rightBlock->property(u8"facing") == blockJ->property(u8"facing")) {
              type = u8"left";
            }
          }

          props[u8"type"] = type;
          auto replace = blockJ->applying(props);
          mcfile::je::SetBlockOptions sbo;
          sbo.fRemoveTileEntity = false;
          out.setBlockAt(x, y, z, replace, sbo);
        }
      }
    }
  }
};

} // namespace je2be::terraform::box360
