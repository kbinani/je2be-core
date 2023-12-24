#pragma once

namespace je2be::toje {

class DoublePlant {
  DoublePlant() = delete;

public:
  static void Do(mcfile::je::Chunk &out, terraform::BlockPropertyAccessor const &accessor) {
    if (!accessor.fHasDoublePlantUpperSunflower) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY() + 1; y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto upperP = accessor.property(x, y, z);
          if (upperP != terraform::BlockPropertyAccessor::DOUBLE_PLANT_UPPER_SUNFLOWER) {
            continue;
          }
          auto upperB = out.blockAt(x, y, z);
          auto lowerB = out.blockAt(x, y - 1, z);
          if (!lowerB || !upperB) {
            continue;
          }
          if (lowerB->fId == upperB->fId) {
            continue;
          }
          if (lowerB->property(u8"half") != u8"lower") {
            continue;
          }
          auto replaceUpper = lowerB->applying({{u8"half", u8"upper"}});
          out.setBlockAt(x, y, z, replaceUpper);
        }
      }
    }
  }
};

} // namespace je2be::toje
