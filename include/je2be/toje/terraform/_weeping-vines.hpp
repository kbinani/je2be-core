#pragma once

namespace je2be::toje {

class WeepingVines {
  WeepingVines() = delete;

public:
  static void Do(mcfile::je::Chunk &out, terraform::bedrock::BlockAccessorBedrock<3, 3> &cache, terraform::BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasWeepingVines) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY() + 1; y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          auto lower = accessor.property(x, y - 1, z);
          if (p == terraform::BlockPropertyAccessor::WEEPING_VINES && lower == terraform::BlockPropertyAccessor::WEEPING_VINES) {
            auto plant = make_shared<mcfile::je::Block const>("minecraft:weeping_vines_plant");
            out.setBlockAt(x, y, z, plant);
          }
        }
      }
    }
  }
};

} // namespace je2be::toje
