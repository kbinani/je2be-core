#pragma once

namespace je2be::toje {

class Piston {
  Piston() = delete;

public:
  static void Do(mcfile::je::Chunk &chunkJ, ChunkCache<3, 3> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasPiston) {
      return;
    }

    int cx = chunkJ.fChunkX;
    int cz = chunkJ.fChunkZ;
    auto chunkB = cache.at(cx, cz);
    if (!chunkB) {
      return;
    }

    mcfile::je::SetBlockOptions sbo;
    sbo.fRemoveTileEntity = false;

    for (int y = mcfile::be::Chunk::kMinBlockY; y <= mcfile::be::Chunk::kMaxBlockY; y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (!BlockPropertyAccessor::IsPiston(p)) {
            continue;
          }
          auto blockB = cache.blockAt(x, y, z);
          if (!blockB) {
            continue;
          }
          auto blockJ = chunkJ.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          map<string, string> props(blockJ->fProperties);
          auto blockEntity = chunkB->blockEntityAt(x, y, z);
          if (blockB->fName == "minecraft:piston" || blockB->fName == "minecraft:sticky_piston") {
            if (blockEntity) {
              auto state = blockEntity->byte("State");
              props["extended"] = ToString(state == 2);
              auto replace = make_shared<mcfile::je::Block const>(blockJ->fName, props);
              chunkJ.setBlockAt(x, y, z, replace, sbo);
            }
          } else if (blockB->fName == "minecraft:movingBlock" && blockEntity) {
            auto piston = PistonBodyFromPistonPos(*blockEntity, cache);
            if (piston) {
              props.clear();

              Facing6 pistonFacing = Facing6FromBedrockFacingDirectionB(piston->fBlock->fStates->int32("facing_direction", 0));
              props["facing"] = JavaNameFromFacing6(pistonFacing);

              bool sticky = piston->fBlock->fName == "minecraft:sticky_piston";
              props["type"] = sticky ? "sticky" : "normal";

              auto replace = make_shared<mcfile::je::Block const>("minecraft:moving_piston", props);
              chunkJ.setBlockAt(x, y, z, replace, sbo);

              //TODO: tile entity
            }
          } else if (blockB->fName == "minecraft:pistonArmCollision" || blockB->fName == "minecraft:stickyPistonArmCollision") {
            Facing6 f6 = Facing6FromBedrockFacingDirectionB(blockB->fStates->int32("facing_direction", 0));
            Pos3i direction = Pos3iFromFacing6(f6);
            Pos3i pistonPos = Pos3i(x, y, z) - direction;
            auto pistonBlockEntity = cache.blockEntityAt(pistonPos);
            if (pistonBlockEntity) {
              auto state = pistonBlockEntity->byte("State");
              if (state == 1) {
                // state = 1 means the piston is extending state.
                // Block name shold be renamed to "moving_piston".
                props.clear();
                props["facing"] = JavaNameFromFacing6(f6);
                bool sticky = blockB->fName == "minecraft:stickyPistonArmCollision";
                props["type"] = sticky ? "sticky" : "normal";
                auto replace = make_shared<mcfile::je::Block const>("minecraft:moving_piston", props);
                chunkJ.setBlockAt(x, y, z, replace, sbo);
              }
            }
          }
        }
      }
    }
  }

  struct PistonBody {
    std::shared_ptr<mcfile::be::Block const> fBlock;
    std::shared_ptr<mcfile::nbt::CompoundTag const> fBlockEntity; //TODO: no more needed?
  };

  static std::optional<PistonBody> PistonBodyFromPistonPos(mcfile::nbt::CompoundTag const &blockEntity, ChunkCache<3, 3> &loader) {
    auto x = blockEntity.int32("pistonPosX");
    auto y = blockEntity.int32("pistonPosY");
    auto z = blockEntity.int32("pistonPosZ");
    if (!x || !y || !z) {
      return std::nullopt;
    }
    auto block = loader.blockAt(*x, *y, *z);
    if (!block) {
      return std::nullopt;
    }
    auto be = loader.blockEntityAt(Pos3i(*x, *y, *z));
    if (!be) {
      return std::nullopt;
    }
    PistonBody pb;
    pb.fBlock = block;
    pb.fBlockEntity = be;
    return pb;
  }

  static std::string ToString(bool b) {
    return b ? "true" : "false";
  }
};

} // namespace je2be::toje
