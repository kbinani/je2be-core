#pragma once

namespace je2be::toje {

class Beacon {
  Beacon() = delete;

public:
  static void Do(mcfile::je::Chunk &out, terraform::bedrock::BlockAccessorBedrock<3, 3> &cache, terraform::BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasBeacon) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY() + 1; y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (p != terraform::BlockPropertyAccessor::BEACON) {
            continue;
          }
          auto tileEntity = out.fTileEntities[Pos3i(x, y, z)];
          if (!tileEntity) [[unlikely]] {
            continue;
          }
          int level = BeaconLevel(x, y, z, cache);
          tileEntity->set(u8"Levels", Int(level));
        }
      }
    }
  }

  static int BeaconLevel(int x, int y, int z, terraform::bedrock::BlockAccessorBedrock<3, 3> &cache) {
    for (int i = 1; i <= 4; i++) {
      int x0 = x - i;
      int z0 = z - i;
      int by = y - i;
      for (int bx = x0; bx <= x0 + 2 * i; bx++) {
        for (int bz = z0; bz <= z0 + 2 * i; bz++) {
          auto b = cache.blockAt(bx, by, bz);
          if (!IsBeaconBaseBlock(b)) {
            return i - 1;
          }
        }
      }
    }
    return 4;
  }

  static bool IsBeaconBaseBlock(std::shared_ptr<mcfile::be::Block const> const &b) {
    if (!b) {
      return false;
    }
    auto name = b->fName;
    return name == u8"minecraft:iron_block" || name == u8"minecraft:gold_block" || name == u8"minecraft:diamond_block" || name == u8"minecraft:emerald_block" || name == u8"minecraft:netherite_block";
  }
};

} // namespace je2be::toje
