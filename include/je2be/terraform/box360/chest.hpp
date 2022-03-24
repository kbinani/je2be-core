#pragma once

namespace je2be::terraform::box360 {

class Chest {
  Chest() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor &cache, BlockPropertyAccessor const &accessor) {
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
          if (!BlockPropertyAccessor::IsChest(p)) {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          string type = blockJ->property("type");
          if (type == "left" || type == "right") {
            continue;
          }
          type = "single";

          auto f4 = Facing4FromJavaName(blockJ->property("facing"));
          auto facing = Pos2iFromFacing4(f4);
          map<string, string> props(blockJ->fProperties);

          Pos2i left = Pos2i(x, z) + Left90(facing);
          if (auto leftBlock = cache.blockAt(left.fX, y, left.fZ); leftBlock) {
            if (leftBlock->fId == blockJ->fId && leftBlock->property("facing") == blockJ->property("facing")) {
              type = "right";
            }
          }

          Pos2i right = Pos2i(x, z) + Right90(facing);
          if (auto rightBlock = cache.blockAt(right.fX, y, right.fZ); rightBlock) {
            if (rightBlock->fId == blockJ->fId && rightBlock->property("facing") == blockJ->property("facing")) {
              type = "left";
            }
          }

          props["type"] = type;
          auto replace = make_shared<mcfile::je::Block const>(blockJ->fName, props);
          mcfile::je::SetBlockOptions sbo;
          sbo.fRemoveTileEntity = false;
          out.setBlockAt(x, y, z, replace, sbo);
        }
      }
    }
  }
};

} // namespace je2be::terraform::box360
