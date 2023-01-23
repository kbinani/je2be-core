#pragma once

#include "terraform/_block-property-accessor.hpp"
#include "terraform/bedrock/_block-accessor-bedrock.hpp"

namespace je2be::terraform::bedrock {

class AttachedStem {
  AttachedStem() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessorBedrock<3, 3> &cache, BlockPropertyAccessor const &accessor) {
    if (accessor.fHasPumpkinStem) {
      DoImpl(out, cache, accessor, "minecraft:pumpkin", "minecraft:attached_pumpkin_stem", IsPumpkinStem);
    }
    if (accessor.fHasMelonStem) {
      DoImpl(out, cache, accessor, "minecraft:melon_block", "minecraft:attached_melon_stem", IsMelonStem);
    }
  }

  static bool IsPumpkinStem(BlockPropertyAccessor::DataType p) {
    return p == BlockPropertyAccessor::PUMPKIN_STEM;
  }

  static bool IsMelonStem(BlockPropertyAccessor::DataType p) {
    return p == BlockPropertyAccessor::MELON_STEM;
  }

  static void DoImpl(mcfile::je::Chunk &out,
                     BlockAccessorBedrock<3, 3> &cache, BlockPropertyAccessor const &accessor,
                     std::string const &cropFullNameBE,
                     std::string const &stemFullNameJE,
                     std::function<bool(BlockPropertyAccessor::DataType)> pred) {
    using namespace std;

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (!pred(p)) {
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
          map<string, optional<string>> props;
          auto name = blockJ->fName;
          if (growth < 7) {
            props["facing"] = nullopt;
          } else {
            auto vec = VecFromFacingDirection(d);
            auto cropPos = Pos2i(x, z) + vec;
            auto crop = cache.blockAt(cropPos.fX, y, cropPos.fZ);
            if (crop && crop->fName == cropFullNameBE) {
              props["facing"] = FacingFromFacingDirection(d);
              props["age"] = nullopt;
              name = stemFullNameJE;
            } else {
              props["facing"] = nullopt;
            }
          }
          auto replace = blockJ->renamed(name)->applying(props);
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }

  static Pos2i VecFromFacingDirection(i32 d) {
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

  static std::string FacingFromFacingDirection(i32 d) {
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

} // namespace je2be::terraform::bedrock
