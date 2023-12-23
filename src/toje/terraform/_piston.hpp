#pragma once

#include "terraform/_block-property-accessor.hpp"

namespace je2be::toje {

class Piston {
  Piston() = delete;

public:
  static void Do(mcfile::je::Chunk &chunkJ, terraform::bedrock::BlockAccessorBedrock<3, 3> &cache, terraform::BlockPropertyAccessor const &accessor) {
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

    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (p != terraform::BlockPropertyAccessor::PISTON) {
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
          auto blockEntity = chunkB->blockEntityAt(x, y, z);
          if ((blockB->fName == u8"minecraft:piston" || blockB->fName == u8"minecraft:sticky_piston") && blockEntity) {
            // Block
            auto state = blockEntity->byte(u8"State");
            int facingDirectionB = blockB->fStates->int32(u8"facing_direction", 0);
            Facing6 f6 = Facing6FromBedrockFacingDirectionB(facingDirectionB);
            bool sticky = blockB->fName == u8"minecraft:sticky_piston";

            if (state == 3) {
              auto replace = mcfile::je::Block::FromId(mcfile::blocks::minecraft::moving_piston, chunkJ.getDataVersion())->applying({{u8"facing", JavaNameFromFacing6(f6)}, {u8"type", sticky ? u8"sticky" : u8"normal"}});
              chunkJ.setBlockAt(x, y, z, replace, sbo);
            } else {
              auto replace = blockJ->applying({{u8"extended", ToString(state == 1 || state == 2)}});
              chunkJ.setBlockAt(x, y, z, replace, sbo);
            }

            // Tile entity
            if (state == 3) {
              bool extending = blockEntity->boolean(u8"expanding", false);
              auto progress = blockEntity->float32(u8"LastProgress", 0.0f);

              auto tileEntityJ = Compound();
              tileEntityJ->set(u8"id", u8"minecraft:piston");
              tileEntityJ->set(u8"keepPacked", Bool(false));
              tileEntityJ->set(u8"x", Int(x));
              tileEntityJ->set(u8"y", Int(y));
              tileEntityJ->set(u8"z", Int(z));
              tileEntityJ->set(u8"extending", Bool(extending));
              tileEntityJ->set(u8"progress", Float(progress));
              tileEntityJ->set(u8"facing", Int(facingDirectionB));
              tileEntityJ->set(u8"source", Bool(true));

              map<u8string, u8string> blockStateProps;
              blockStateProps[u8"extended"] = ToString(extending);
              blockStateProps[u8"facing"] = JavaNameFromFacing6(f6);
              auto blockState = mcfile::je::Block::FromNameAndProperties(blockB->fName, chunkJ.getDataVersion(), blockStateProps);

              tileEntityJ->set(u8"blockState", blockState->toCompoundTag());

              chunkJ.fTileEntities[Pos3i(x, y, z)] = tileEntityJ;
            }
          } else if ((blockB->fName == u8"minecraft:movingBlock" || blockB->fName == u8"minecraft:moving_block") && blockEntity) {
            auto piston = PistonBodyFromPistonPos(*blockEntity, cache);
            if (piston) {
              // Block
              int facingDirectionB = piston->fBlock->fStates->int32(u8"facing_direction", 0);
              Facing6 pistonFacing = Facing6FromBedrockFacingDirectionB(facingDirectionB);

              auto replace = mcfile::je::Block::FromId(mcfile::blocks::minecraft::moving_piston, chunkJ.getDataVersion())->applying({
                  {u8"facing", JavaNameFromFacing6(pistonFacing)}, {u8"type", u8"normal"}, // Always "normal"
              });
              chunkJ.setBlockAt(x, y, z, replace, sbo);

              // Tile entity
              auto state = piston->fBlockEntity->byte(u8"State");
              auto lastProgress = piston->fBlockEntity->float32(u8"LastProgress", 0.0f);
              bool extending = state == 1; // state should be 3 or 1 here.
              auto tileEntityJ = Compound();
              tileEntityJ->set(u8"id", u8"minecraft:piston");
              tileEntityJ->set(u8"keepPacked", Bool(false));
              tileEntityJ->set(u8"x", Int(x));
              tileEntityJ->set(u8"y", Int(y));
              tileEntityJ->set(u8"z", Int(z));
              tileEntityJ->set(u8"extending", Bool(extending));
              tileEntityJ->set(u8"progress", Float(lastProgress)); //TODO(1.21) state == 3 ? 0 : 0.5));
              tileEntityJ->set(u8"facing", Int(facingDirectionB));
              tileEntityJ->set(u8"source", Bool(false));

              auto movingBlockJ = MovingBlock(*blockEntity, chunkJ.getDataVersion());
              if (movingBlockJ) {
                tileEntityJ->set(u8"blockState", movingBlockJ->toCompoundTag());
              }

              chunkJ.fTileEntities[Pos3i(x, y, z)] = tileEntityJ;
            }
          } else if (blockB->fName == u8"minecraft:pistonArmCollision" || blockB->fName == u8"minecraft:piston_arm_collision" || blockB->fName == u8"minecraft:stickyPistonArmCollision" || blockB->fName == u8"minecraft:sticky_piston_arm_collision") {
            int facingDirectionB = blockB->fStates->int32(u8"facing_direction", 0);
            Facing6 f6 = Facing6FromBedrockFacingDirectionB(facingDirectionB);
            Pos3i direction = Pos3iFromFacing6(f6);
            Pos3i pistonPos = Pos3i(x, y, z) - direction;
            auto pistonBlockEntity = cache.blockEntityAt(pistonPos);
            if (pistonBlockEntity) {
              auto state = pistonBlockEntity->byte(u8"State");
              if (state == 1) {
                // Block
                // state = 1 means the piston is extending state.
                // Block name shold be renamed to "moving_piston".
                bool sticky = (blockB->fName == u8"minecraft:stickyPistonArmCollision") || (blockB->fName == u8"minecraft:sticky_piston_arm_collision");
                auto replace = mcfile::je::Block::FromId(mcfile::blocks::minecraft::moving_piston, chunkJ.getDataVersion())->applying({{u8"facing", JavaNameFromFacing6(f6)}, {u8"type", sticky ? u8"sticky" : u8"normal"}});
                chunkJ.setBlockAt(x, y, z, replace, sbo);

                // Tile Entity
                auto lastProgress = pistonBlockEntity->float32(u8"LastProgress", 0.0f);
                auto tileEntityJ = Compound();
                tileEntityJ->set(u8"id", u8"minecraft:piston");
                tileEntityJ->set(u8"keepPacked", Bool(false));
                tileEntityJ->set(u8"x", Int(x));
                tileEntityJ->set(u8"y", Int(y));
                tileEntityJ->set(u8"z", Int(z));
                tileEntityJ->set(u8"progress", Float(lastProgress));
                tileEntityJ->set(u8"facing", Int(facingDirectionB));
                tileEntityJ->set(u8"source", Bool(true));
                tileEntityJ->set(u8"extending", Bool(true));

                map<u8string, u8string> pistonHeadProps;
                pistonHeadProps[u8"facing"] = JavaNameFromFacing6(f6);
                pistonHeadProps[u8"short"] = u8"false";
                pistonHeadProps[u8"type"] = sticky ? u8"sticky" : u8"normal";
                auto pistonHead = mcfile::je::Block::FromIdAndProperties(mcfile::blocks::minecraft::piston_head, chunkJ.getDataVersion(), pistonHeadProps);
                tileEntityJ->set(u8"blockState", pistonHead->toCompoundTag());

                chunkJ.fTileEntities[Pos3i(x, y, z)] = tileEntityJ;
              }
            }
          }
        }
      }
    }
  }

  static std::shared_ptr<mcfile::je::Block const> MovingBlock(CompoundTag const &blockEntity, int dataVersion) {
    auto movingBlock = blockEntity.compoundTag(u8"movingBlock");
    if (!movingBlock) {
      return nullptr;
    }
    auto movingBlockB = mcfile::be::Block::FromCompound(*movingBlock);
    if (!movingBlockB) {
      return nullptr;
    }
    return BlockData::From(*movingBlockB, dataVersion);
  }

  struct PistonBody {
    std::shared_ptr<mcfile::be::Block const> fBlock;
    std::shared_ptr<CompoundTag const> fBlockEntity;
  };

  static std::optional<PistonBody> PistonBodyFromPistonPos(CompoundTag const &blockEntity, terraform::bedrock::BlockAccessorBedrock<3, 3> &loader) {
    auto x = blockEntity.int32(u8"pistonPosX");
    auto y = blockEntity.int32(u8"pistonPosY");
    auto z = blockEntity.int32(u8"pistonPosZ");
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

  static std::u8string ToString(bool b) {
    return b ? u8"true" : u8"false";
  }
};

} // namespace je2be::toje
