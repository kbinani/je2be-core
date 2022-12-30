#pragma once

#include <je2be/terraform/block-property-accessor.hpp>
#include <je2be/terraform/box360/block-accessor-box360.hpp>

namespace je2be::terraform::box360 {

class AttachedStem {
  AttachedStem() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessorBox360<3, 3> &cache, BlockPropertyAccessor const &accessor) {
    using namespace mcfile::blocks;
    if (accessor.fHasMelonStem) {
      Impl(out, cache, accessor, minecraft::melon, minecraft::attached_melon_stem, IsMelonStem);
    }
    if (accessor.fHasPumpkinStem) {
      Impl(out, cache, accessor, minecraft::pumpkin, minecraft::attached_pumpkin_stem, IsPumpkinStem);
    }
  }

  static void Impl(mcfile::je::Chunk &out,
                   BlockAccessorBox360<3, 3> &cache,
                   BlockPropertyAccessor const &accessor,
                   mcfile::blocks::BlockId cropBlockId,
                   mcfile::blocks::BlockId attachedStemBlockId,
                   std::function<bool(BlockPropertyAccessor::DataType)> pred) {
    using namespace std;

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    vector<pair<string, Pos2i>> const nesw({{"north", Pos2i(0, -1)}, {"east", Pos2i(1, 0)}, {"south", Pos2i(0, 1)}, {"west", Pos2i(-1, 0)}});
    string attachedStemBlockName = mcfile::blocks::Name(attachedStemBlockId);

    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (!pred(p)) {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          for (auto it : nesw) {
            auto targetPos = Pos2i(x, z) + it.second;
            auto target = cache.blockAt(targetPos.fX, y, targetPos.fZ);
            if (!target) {
              continue;
            }
            if (target->fId != cropBlockId) {
              continue;
            }
            auto replace = make_shared<mcfile::je::Block const>(attachedStemBlockName, map<string, string>{{"facing", it.first}});
            out.setBlockAt(x, y, z, replace);
            break;
          }
        }
      }
    }
  }

  static bool IsMelonStem(BlockPropertyAccessor::DataType p) {
    return p == BlockPropertyAccessor::MELON_STEM;
  }

  static bool IsPumpkinStem(BlockPropertyAccessor::DataType p) {
    return p == BlockPropertyAccessor::PUMPKIN_STEM;
  }
};

} // namespace je2be::terraform::box360
