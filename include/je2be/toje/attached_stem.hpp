#pragma once

namespace je2be::toje {

class AttachedStem {
  AttachedStem() = delete;

public:
  static void Do(mcfile::je::Chunk &out, ChunkCache<3, 3> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasPumpkinStem) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = mcfile::be::Chunk::kMinChunkY * 16; y < mcfile::be::Chunk::kMaxChunkY * 16; y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (!BlockPropertyAccessor::IsPumpkinStem(p)) {
            continue;
          }
          auto stem = cache.blockAt(x, y, z);
          if (!stem) {
            continue;
          }
          auto d = stem->fStates->int32("facing_direction", 0);
          auto growth = stem->fStates->int32("growth", 0);
          if (growth < 7) {
            continue;
          }
          auto vec = VecFromFacingDirection(d);
          auto cropPos = Pos2i(x, z) + vec;
          auto crop = cache.blockAt(cropPos.fX, y, cropPos.fZ);
          if (crop && crop->fName == "minecraft:pumpkin") {
            map<string, string> props;
            props["facing"] = FacingFromFacingDirection(d);
            auto attachedStem = make_shared<mcfile::je::Block const>("minecraft:attached_pumpkin_stem", props);
            out.setBlockAt(x, y, z, attachedStem);
          }
        }
      }
    }
  }

  static Pos2i VecFromFacingDirection(int32_t d) {
    switch (d) {
    case 5:
      return Pos2i(1, 0); // east
    case 4:
      return Pos2i(-1, 0); // west
    case 2:
      return Pos2i(0, -1); // north
    default:
    case 3:
      return Pos2i(0, 1); // south
    }
  }

  static std::string FacingFromFacingDirection(int32_t d) {
    switch (d) {
    case 5:
      return "east";
    case 4:
      return "west";
    case 2:
      return "north";
    default:
    case 3:
      return "south";
    }
  }
};

} // namespace je2be::toje
