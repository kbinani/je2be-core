#pragma once

namespace je2be::toje {

class Door {
  Door() = delete;

public:
  static void Do(mcfile::je::Chunk &out, terraform::bedrock::BlockAccessorBedrock<3, 3> &cache, terraform::BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasDoor) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY(); y < accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto lowerP = accessor.property(x, y, z);
          auto upperP = accessor.property(x, y + 1, z);
          if (lowerP != terraform::BlockPropertyAccessor::DOOR || upperP != terraform::BlockPropertyAccessor::DOOR) {
            continue;
          }
          auto lowerB = cache.blockAt(x, y, z);
          auto upperB = cache.blockAt(x, y + 1, z);
          if (!lowerB || !upperB) {
            continue;
          }
          if (lowerB->fName != upperB->fName) {
            continue;
          }
          auto lowerJ = out.blockAt(x, y, z);
          auto upperJ = out.blockAt(x, y + 1, z);
          if (!lowerJ || !upperJ) {
            continue;
          }
          if (lowerJ->property(u8"half") != u8"lower" || upperJ->property(u8"half") != u8"upper") {
            continue;
          }
          u8string facing(lowerJ->property(u8"facing"));
          u8string open(lowerJ->property(u8"open"));
          u8string hinge(upperJ->property(u8"hinge"));
          if (facing.empty() || open.empty() || hinge.empty()) {
            continue;
          }

          // NOTE: Doors in villages usually need this repair.
          auto replaceUpper = upperJ->applying({
              {u8"facing", facing},
              {u8"open", open},
          });
          auto replaceLower = lowerJ->applying({{u8"hinge", hinge}});
          out.setBlockAt(x, y, z, replaceLower);
          out.setBlockAt(x, y + 1, z, replaceUpper);
        }
      }
    }
  }
};

} // namespace je2be::toje
