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
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          map<string, string> props(blockJ->fProperties);
          string name = blockJ->fName;
          if (growth < 7) {
            props.erase("facing");
          } else {
            auto vec = VecFromFacingDirection(d);
            auto cropPos = Pos2i(x, z) + vec;
            auto crop = cache.blockAt(cropPos.fX, y, cropPos.fZ);
            if (crop && crop->fName == "minecraft:pumpkin") {
              props["facing"] = FacingFromFacingDirection(d);
              props.erase("age");
              name = "minecraft:attached_pumpkin_stem";
            } else {
              props.erase("facing");
            }
          }
          auto replace = make_shared<mcfile::je::Block const>(name, props);
          out.setBlockAt(x, y, z, replace);
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
    case 3:
      return Pos2i(0, 1); // south
    default:
      return Pos2i(0, 0); // up, down
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
