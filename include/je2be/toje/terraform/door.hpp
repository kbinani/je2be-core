#pragma once

namespace je2be::toje {

class Door {
  Door() = delete;

public:
  static void Do(mcfile::je::Chunk &out, ChunkCache<3, 3> &cache, BlockPropertyAccessor const &accessor) {
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
          if (!BlockPropertyAccessor::IsDoor(lowerP) || !BlockPropertyAccessor::IsDoor(upperP)) {
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
          if (lowerJ->property("half") != "lower" || upperJ->property("half") != "upper") {
            continue;
          }
          auto facing = lowerJ->property("facing");
          auto open = lowerJ->property("open");
          auto hinge = upperJ->property("hinge");
          if (facing.empty() || open.empty() || hinge.empty()) {
            continue;
          }
          map<string, string> propsUpper(upperJ->fProperties);
          map<string, string> propsLower(lowerJ->fProperties);

          //NOTE: Doors in villages usually need this repair.
          propsLower["hinge"] = hinge;

          propsUpper["facing"] = facing;
          propsUpper["open"] = open;

          auto replaceUpper = make_shared<mcfile::je::Block const>(upperJ->fName, propsUpper);
          auto replaceLower = make_shared<mcfile::je::Block const>(lowerJ->fName, propsLower);
          out.setBlockAt(x, y, z, replaceLower);
          out.setBlockAt(x, y + 1, z, replaceUpper);
        }
      }
    }
  }
};

} // namespace je2be::toje
