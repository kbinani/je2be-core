#pragma once

#include <minecraft-file.hpp>

#include "_pos3.hpp"
#include "enums/_facing6.hpp"
#include "terraform/_block-accessor.hpp"
#include "terraform/_block-property-accessor.hpp"

namespace je2be::terraform::box360 {

class NetherPortal {
  NetherPortal() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &cache, terraform::BlockPropertyAccessor const &accessor) {
    using namespace std;
    using namespace mcfile::blocks;

    if (!accessor.fHasNetherPortal) {
      return;
    }
    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY(); y < accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (p != BlockPropertyAccessor::NETHER_PORTAL) {
            continue;
          }
          Pos3i center(x, y, z);
          enum Status {
            ReachedToObsidian,
            PortalBlock,
            Illegal,
          };
          map<Facing6, Status> status;
          Facing6Enumerate([&status](Facing6 f) {
            status[f] = PortalBlock;
            return true;
          });
          for (int i = 1; i <= 21; i++) {
            Facing6Enumerate([&](Facing6 f) {
              Pos3i direction = Pos3iFromFacing6(f);
              Pos3i pos = center + direction * i;

              if (status[f] != PortalBlock) {
                return true;
              }
              auto block = cache.blockAt(pos.fX, pos.fY, pos.fZ);
              if (!block) {
                status[f] = Illegal;
                return true;
              }
              switch (block->fId) {
              case minecraft::obsidian:
                status[f] = ReachedToObsidian;
                break;
              case minecraft::nether_portal:
                break;
              default:
                status[f] = Illegal;
                break;
              }
              return true;
            });
            if (all_of(status.begin(), status.end(), [](auto it) { return it.second != PortalBlock; })) {
              break;
            }
          }
          if (status[Facing6::Up] != ReachedToObsidian || status[Facing6::Down] != ReachedToObsidian) {
            out.setBlockAt(center, make_shared<mcfile::je::Block const>(u8"minecraft:air"));
          } else {
            if (status[Facing6::North] == ReachedToObsidian && status[Facing6::South] == ReachedToObsidian) {
              if (auto block = out.blockAt(center); block) {
                out.setBlockAt(center, block->applying({{u8"axis", u8"z"}}));
              }
            } else if (status[Facing6::East] == ReachedToObsidian && status[Facing6::West] == ReachedToObsidian) {
              if (auto block = out.blockAt(center); block) {
                out.setBlockAt(center, block->applying({{u8"axis", u8"x"}}));
              }
            } else {
              out.setBlockAt(center, make_shared<mcfile::je::Block const>(u8"minecraft:air"));
            }
          }
        }
      }
    }
  }
};

} // namespace je2be::terraform::box360
