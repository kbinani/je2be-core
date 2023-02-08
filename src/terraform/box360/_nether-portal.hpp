#pragma once

namespace je2be::terraform::box360 {

class NetherPortal {
  NetherPortal() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &cache, terraform::BlockPropertyAccessor const &accessor) {
    using namespace std;

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
          for (int i = 1; i < 21; i++) {
            Facing6Enumerate([&](Facing6 f) {
              Pos3i direction = Pos3iFromFacing6(f);
              Pos3i p(x + direction.fX * i, y + direction.fY * i, z + direction.fZ * i);
              if (status[f] == PortalBlock && accessor.property(p.fX, p.fY, p.fZ) != BlockPropertyAccessor::NETHER_PORTAL) {
                if (auto block = out.blockAt(p.fX, p.fY, p.fZ); block) {
                  if (block->fId == mcfile::blocks::minecraft::obsidian) {
                    status[f] = ReachedToObsidian;
                  }
                } else {
                  status[f] = Illegal;
                }
              }
              return true;
            });
            if (all_of(status.begin(), status.end(), [](auto it) { return it.second != PortalBlock; })) {
              break;
            }
          }
          if (status[Facing6::Up] != ReachedToObsidian || status[Facing6::Down] != ReachedToObsidian) {
            out.setBlockAt({x, y, z}, make_shared<mcfile::je::Block const>("minecraft:air"));
          } else {
            if (status[Facing6::North] == ReachedToObsidian && status[Facing6::South] == ReachedToObsidian) {
              if (auto block = out.blockAt(x, y, z); block) {
                out.setBlockAt({x, y, z}, block->applying({{"axis", "z"}}));
              }
            } else if (status[Facing6::East] == ReachedToObsidian && status[Facing6::West] == ReachedToObsidian) {
              if (auto block = out.blockAt(x, y, z); block) {
                out.setBlockAt({x, y, z}, block->applying({{"axis", "x"}}));
              }
            } else {
              out.setBlockAt({x, y, z}, make_shared<mcfile::je::Block const>("minecraft:air"));
            }
          }
        }
      }
    }
  }
};

} // namespace je2be::terraform::box360
