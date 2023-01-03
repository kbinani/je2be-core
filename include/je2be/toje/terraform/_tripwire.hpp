#pragma once

namespace je2be::toje {

class Tripwire {
  Tripwire() = delete;

public:
  static void Do(mcfile::je::Chunk &out, terraform::bedrock::BlockAccessorBedrock<3, 3> &cache, terraform::BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasTripwire) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    vector<pair<string, Pos2i>> const nesw({{"north", Pos2i(0, -1)}, {"east", Pos2i(1, 0)}, {"south", Pos2i(0, 1)}, {"west", Pos2i(-1, 0)}});

    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (p != terraform::BlockPropertyAccessor::TRIPWIRE) {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          map<string, optional<string>> props;
          for (auto it : nesw) {
            Pos2i vec = it.second;
            auto block = cache.blockAt(x + vec.fX, y, z + vec.fZ);
            bool connect = false;
            if (block) {
              if (terraform::BlockPropertyAccessor::IsTripwire(*block)) {
                connect = true;
              } else if (block->fName == "minecraft:tripwire_hook") {
                Facing4 f4 = Facing4FromBedrockDirection(block->fStates->int32("direction", 0));
                Pos2i direction = Pos2iFromFacing4(f4);
                connect = direction.fX == -vec.fX && direction.fZ == -vec.fZ;
              } else {
                connect = false;
              }
            }
            props[it.first] = connect ? "true" : "false";
          }
          auto replace = blockJ->applying(props);
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }
};

} // namespace je2be::toje
