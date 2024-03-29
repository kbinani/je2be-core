#pragma once

namespace je2be::bedrock {

class TwistingVines {
  TwistingVines() = delete;

public:
  static void Do(mcfile::je::Chunk &out, terraform::bedrock::BlockAccessorBedrock<3, 3> &cache, terraform::BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasTwistingVines) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY(); y < accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          auto upper = accessor.property(x, y + 1, z);
          if (p == terraform::BlockPropertyAccessor::TWISTING_VINES && upper == terraform::BlockPropertyAccessor::TWISTING_VINES) {
            auto plant = mcfile::je::Block::FromId(mcfile::blocks::minecraft::twisting_vines_plant, out.getDataVersion());
            out.setBlockAt(x, y, z, plant);
          }
        }
      }
    }
  }
};

} // namespace je2be::bedrock
