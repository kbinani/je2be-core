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
          Status north = PortalBlock;
          Status east = PortalBlock;
          Status south = PortalBlock;
          Status west = PortalBlock;
          for (int i = 1; i < 23; i++) {
            if (north == PortalBlock && accessor.property(x, y, z - i) != BlockPropertyAccessor::NETHER_PORTAL) {
              if (auto block = out.blockAt(x, y, z - i); block) {
                if (block->fId == mcfile::blocks::minecraft::obsidian) {
                  north = ReachedToObsidian;
                }
              } else {
                north = Illegal;
              }
            }
            if (east == PortalBlock && accessor.property(x + i, y, z) != BlockPropertyAccessor::NETHER_PORTAL) {
              if (auto block = out.blockAt(x + i, y, z); block) {
                if (block->fId == mcfile::blocks::minecraft::obsidian) {
                  east = ReachedToObsidian;
                }
              } else {
                east = Illegal;
              }
            }
            if (south == PortalBlock && accessor.property(x, y, z + i) != BlockPropertyAccessor::NETHER_PORTAL) {
              if (auto block = out.blockAt(x, y, z + i); block) {
                if (block->fId == mcfile::blocks::minecraft::obsidian) {
                  south = ReachedToObsidian;
                }
              } else {
                south = Illegal;
              }
            }
            if (west == PortalBlock && accessor.property(x - i, y, z) != BlockPropertyAccessor::NETHER_PORTAL) {
              if (auto block = out.blockAt(x - i, y, z); block) {
                if (block->fId == mcfile::blocks::minecraft::obsidian) {
                  west = ReachedToObsidian;
                }
              } else {
                west = Illegal;
              }
            }
            if (north != PortalBlock && east != PortalBlock && south != PortalBlock && west != PortalBlock) {
              break;
            }
          }

          if (north == ReachedToObsidian && south == ReachedToObsidian) {
            if (auto block = out.blockAt(x, y, z); block) {
              out.setBlockAt({x, y, z}, block->applying({{"axis", "z"}}));
            }
          } else if (east == ReachedToObsidian && west == ReachedToObsidian) {
            if (auto block = out.blockAt(x, y, z); block) {
              out.setBlockAt({x, y, z}, block->applying({{"axis", "x"}}));
            }
          } else {
            continue;
          }
        }
      }
    }
  }
};

} // namespace je2be::terraform::box360
