#include "tobe/_moving-piston.hpp"

#include "enums/_facing6.hpp"
#include "tobe/_block-data.hpp"
#include "tobe/_versions.hpp"

namespace je2be::tobe {

class MovingPiston::Impl {
  Impl() = delete;

public:
  static void PreprocessChunk(mcfile::je::CachedChunkLoader &loader, mcfile::je::Chunk &chunk) {
    using namespace mcfile;
    using namespace mcfile::je;
    using namespace std;

    SetBlockOptions withoutRemovingTileEntity;
    withoutRemovingTileEntity.fRemoveTileEntity = false;

    unordered_map<Pos3i, shared_ptr<CompoundTag>, Pos3iHasher> tileEntityReplacement;

    if (ChunkHasPiston(chunk)) {
      for (int by = chunk.minBlockY(); by <= chunk.maxBlockY(); by++) {
        for (int bx = chunk.minBlockX(); bx <= chunk.maxBlockX(); bx++) {
          for (int bz = chunk.minBlockZ(); bz <= chunk.maxBlockZ(); bz++) {
            Pos3i pos(bx, by, bz);
            auto block = loader.blockAt(pos);
            if (!block) {
              continue;
            }
            if (block->fName != u8"minecraft:sticky_piston" && block->fName != u8"minecraft:piston") {
              continue;
            }
            if (loader.tileEntityAt(pos)) {
              continue;
            }
            Facing6 f6 = Facing6FromJavaName(block->property(u8"facing", u8""));
            Pos3i vec = Pos3iFromFacing6(f6);
            Pos3i pistonHeadPos = pos + vec;
            int facingDirectionB = BedrockFacingDirectionBFromFacing6(f6);
            auto pistonHeadTileEntity = loader.tileEntityAt(pistonHeadPos);
            if (pistonHeadTileEntity) {
              auto pistonHead = PistonTileEntity::From(pistonHeadTileEntity, pistonHeadPos);
              if (!pistonHead) {
                continue;
              }
              if (!pistonHead->fSource || !pistonHead->fExtending || pistonHead->fFacing != facingDirectionB) {
                continue;
              }

              unordered_set<Pos3i, Pos3iHasher> attachedBlocks;
              LookupAttachedBlocks(loader, pistonHeadPos, true, facingDirectionB, attachedBlocks);

              auto progressJ = pistonHeadTileEntity->float32(u8"progress", 0);
              float lastProgressB = progressJ;
              float progressB = progressJ > 0 ? 0 : 0.5f;

              auto pistonArm = Compound();
              auto attachedBlocksTag = List<Tag::Type::Int>();
              for (auto attachedBlock : attachedBlocks) {
                Pos3i actual = attachedBlock - vec;
                attachedBlocksTag->push_back(Int(actual.fX));
                attachedBlocksTag->push_back(Int(actual.fY));
                attachedBlocksTag->push_back(Int(actual.fZ));
              }
              pistonArm->set(u8"AttachedBlocks", attachedBlocksTag);
              pistonArm->set(u8"BreakBlocks", List<Tag::Type::Int>());
              pistonArm->set(u8"LastProgress", Float(lastProgressB));
              pistonArm->set(u8"NewState", Byte(1));
              pistonArm->set(u8"Progress", Float(progressB));
              pistonArm->set(u8"State", Byte(1));
              pistonArm->set(u8"Sticky", Bool(block->fName == u8"minecraft:sticky_piston"));
              pistonArm->set(u8"id", u8"j2b:PistonArm");
              pistonArm->set(u8"isMovable", Bool(false));
              pistonArm->set(u8"x", Int(pos.fX));
              pistonArm->set(u8"y", Int(pos.fY));
              pistonArm->set(u8"z", Int(pos.fZ));

              tileEntityReplacement[pos] = pistonArm;
            } else {
              // Tile entity doesn't exist at piston_head position.
              // This means the piston is in static state.
              bool extended = block->property(u8"extended", u8"false") == u8"true";
              auto pistonArm = Compound();
              pistonArm->set(u8"AttachedBlocks", List<Tag::Type::Int>());
              pistonArm->set(u8"BreakBlocks", List<Tag::Type::Int>());
              pistonArm->set(u8"LastProgress", Float(extended ? 1 : 0));
              pistonArm->set(u8"NewState", Byte(extended ? 2 : 0));
              pistonArm->set(u8"Progress", Float(extended ? 1 : 0));
              pistonArm->set(u8"State", Byte(extended ? 2 : 0));
              pistonArm->set(u8"Sticky", Bool(block->fName == u8"minecraft:sticky_piston"));
              pistonArm->set(u8"id", u8"j2b:PistonArm");
              pistonArm->set(u8"isMovable", Bool(extended ? false : true));
              pistonArm->set(u8"x", Int(pos.fX));
              pistonArm->set(u8"y", Int(pos.fY));
              pistonArm->set(u8"z", Int(pos.fZ));

              tileEntityReplacement[pos] = pistonArm;
            }
          }
        }
      }
    }

    for (auto it : chunk.fTileEntities) {
      Pos3i pos = it.first;
      auto const &item = it.second;
      auto id = item->string(u8"id");
      if (!id) {
        continue;
      }
      if (id != u8"minecraft:piston") {
        continue;
      }
      auto extending = item->boolean(u8"extending");
      if (!extending) {
        continue;
      }
      auto source = item->boolean(u8"source");
      if (!source) {
        continue;
      }
      auto facing = item->int32(u8"facing");
      if (!facing) {
        continue;
      }
      if (*source) {
        if (*extending) {
          // extending = 1, source = 1
          auto block = chunk.blockAt(pos);
          if (!block) {
            continue;
          }

          map<u8string, u8string> props;
          props[u8"facing_direction"] = String::ToString(*facing);
          assert(block->fName == u8"minecraft:moving_piston");
          u8string name = block->property(u8"type") == u8"sticky" ? u8"j2b:sticky_piston_arm_collision" : u8"j2b:piston_arm_collision";
          auto newBlock = Block::FromNameAndProperties(name, chunk.getDataVersion(), props);
          chunk.setBlockAt(pos, newBlock, withoutRemovingTileEntity);

          tileEntityReplacement[pos] = nullptr;
        } else {
          // extending = 0, source = 1
          auto block = chunk.blockAt(pos);
          if (!block) {
            continue;
          }

          auto sticky = block->property(u8"type", u8"normal") == u8"sticky";
          mcfile::blocks::BlockId newId = sticky ? mcfile::blocks::minecraft::sticky_piston : mcfile::blocks::minecraft::piston;
          auto newBlock = block->withId(newId);
          chunk.setBlockAt(pos, newBlock, withoutRemovingTileEntity);

          unordered_set<Pos3i, Pos3iHasher> attachedBlocks;
          LookupAttachedBlocks(loader, pos, *extending, *facing, attachedBlocks);

          Pos3i vec = VectorOfFacing(*facing);

          auto pistonArm = Compound();
          auto attachedBlocksTag = List<Tag::Type::Int>();
          for (auto attachedBlock : attachedBlocks) {
            attachedBlocksTag->push_back(Int(attachedBlock.fX + vec.fX));
            attachedBlocksTag->push_back(Int(attachedBlock.fY + vec.fY));
            attachedBlocksTag->push_back(Int(attachedBlock.fZ + vec.fZ));
          }
          pistonArm->set(u8"AttachedBlocks", attachedBlocksTag);
          pistonArm->set(u8"BreakBlocks", List<Tag::Type::Int>());
          auto progressJ = item->float32(u8"progress", 0.0f);
          float progressB = progressJ > 0 ? 0 : 0.5f;
          float lastProgressB = progressJ > 0 ? 0.5f : 0;
          pistonArm->set(u8"LastProgress", Float(lastProgressB));
          pistonArm->set(u8"NewState", Byte(3));
          pistonArm->set(u8"Progress", Float(progressB));
          pistonArm->set(u8"State", Byte(3));
          pistonArm->set(u8"Sticky", Bool(sticky));
          pistonArm->set(u8"id", u8"j2b:PistonArm");
          pistonArm->set(u8"isMovable", Bool(false));
          pistonArm->set(u8"x", Int(pos.fX));
          pistonArm->set(u8"y", Int(pos.fY));
          pistonArm->set(u8"z", Int(pos.fZ));

          tileEntityReplacement[pos] = pistonArm;
        }
      } else {
        // extending = *, source = 0
        auto e = MovingBlockEntityFromPistonTileEntity(pos, *facing, item, loader, *extending, chunk.getDataVersion());
        tileEntityReplacement[pos] = e;
      }
    }

    for (auto it : tileEntityReplacement) {
      Pos3i pos = it.first;
      auto tileEntity = it.second;
      if (tileEntity) {
        chunk.fTileEntities[pos] = tileEntity;
      } else {
        chunk.fTileEntities.erase(pos);
      }
    }
  }

private:
  static CompoundTagPtr MovingBlockEntityFromPistonTileEntity(Pos3i pos, int facing, std::shared_ptr<CompoundTag const> const &item, mcfile::je::CachedChunkLoader &loader, bool expanding, int dataVersion) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::je;

    auto e = Compound();
    e->set(u8"id", u8"j2b:MovingBlock");
    e->set(u8"isMovable", Bool(true));
    e->set(u8"expanding", Bool(expanding));

    auto blockState = item->compoundTag(u8"blockState");
    if (!blockState) {
      return nullptr;
    }
    auto block = Block::FromCompoundTag(*blockState, dataVersion);
    auto movingBlock = BlockData::From(block, nullptr, {});
    if (!movingBlock) {
      return nullptr;
    }
    e->set(u8"movingBlock", movingBlock);

    auto movingBlockExtra = Compound();
    movingBlockExtra->set(u8"name", u8"minecraft:air");
    movingBlockExtra->set(u8"states", Compound());
    movingBlockExtra->set(u8"version", Int(kBlockDataVersion));
    e->set(u8"movingBlockExtra", movingBlockExtra);

    auto pistonPos = LookupPistonPos(loader, pos, facing);
    if (pistonPos) {
      e->set(u8"pistonPosX", Int(pistonPos->fX));
      e->set(u8"pistonPosY", Int(pistonPos->fY));
      e->set(u8"pistonPosZ", Int(pistonPos->fZ));
    }

    e->set(u8"x", Int(pos.fX));
    e->set(u8"y", Int(pos.fY));
    e->set(u8"z", Int(pos.fZ));

    return e;
  }

  struct PistonTileEntity {
    std::u8string fName;
    int fFacing;
    bool fExtending;
    bool fSource;
    Pos3i fPos;

    PistonTileEntity(std::u8string const &name, int facing, bool extending, bool source, Pos3i pos)
        : fName(name), fFacing(facing), fExtending(extending), fSource(source), fPos(pos) {}

    static std::optional<PistonTileEntity> From(std::shared_ptr<CompoundTag const> const &tag, Pos3i pos) {
      if (!tag) {
        return std::nullopt;
      }
      auto id = tag->string(u8"id");
      if (!id) {
        return std::nullopt;
      }
      if (*id != u8"minecraft:piston") {
        return std::nullopt;
      }
      auto state = tag->compoundTag(u8"blockState");
      if (!state) {
        return std::nullopt;
      }
      auto name = state->string(u8"Name");
      if (!name) {
        return std::nullopt;
      }
      auto facing = tag->int32(u8"facing");
      if (!facing) {
        return std::nullopt;
      }
      auto extending = tag->boolean(u8"extending");
      if (!extending) {
        return std::nullopt;
      }
      auto source = tag->boolean(u8"source");
      if (!source) {
        return std::nullopt;
      }
      return PistonTileEntity(*name, *facing, *extending, *source, pos);
    }
  };

  static void LookupAttachedBlocks(mcfile::je::CachedChunkLoader &loader, Pos3i center, bool extendingExpected, int facingExpected, std::unordered_set<Pos3i, Pos3iHasher> &attachedBlocks) {
    using namespace std;
    using namespace mcfile;

    attachedBlocks.clear();

    static int constexpr kMaxMovableBlocksByAPiston = 12;

    unordered_map<Pos3i, optional<PistonTileEntity>, Pos3iHasher> testedBlocks;
    unordered_set<Pos3i, Pos3iHasher> testBlocks;

    Pos3i const startPos = center + VectorOfFacing(facingExpected);
    optional<PistonTileEntity> startBlock = PistonTileEntity::From(loader.tileEntityAt(startPos), startPos);
    if (!startBlock) {
      return;
    }
    attachedBlocks.insert(startPos);
    testedBlocks.insert(make_pair(startPos, *startBlock));
    testBlocks.insert(startPos);

    while (!testBlocks.empty() && attachedBlocks.size() <= kMaxMovableBlocksByAPiston) {
      vector<pair<Pos3i, PistonTileEntity>> testing;
      for (Pos3i pos : testBlocks) {
        auto base = testedBlocks.find(pos);
        if (base == testedBlocks.end()) {
          continue;
        }
        if (!base->second) {
          continue;
        }
        for (int facing = 0; facing < 6; facing++) {
          Pos3i p = pos + VectorOfFacing(facing);
          if (testedBlocks.find(p) != testedBlocks.end()) {
            continue;
          }
          testing.push_back(make_pair(p, *base->second));
        }
      }
      testBlocks.clear();

      sort(testing.begin(), testing.end(), CompareByChunkDistance(center.fX, center.fZ));

      for (auto it : testing) {
        Pos3i pos = it.first;
        PistonTileEntity base = it.second;
        auto target = PistonTileEntity::From(loader.tileEntityAt(pos), pos);
        testedBlocks.insert(make_pair(pos, target));
        if (!target) {
          continue;
        }
        if (!IsBaseStickyAgainstTarget(base, *target)) {
          continue;
        }
        testBlocks.insert(pos);
        attachedBlocks.insert(pos);
      }
    }
  }

  static std::optional<Pos3i> LookupPistonPos(mcfile::je::CachedChunkLoader &loader, Pos3i center, int facing) {
    using namespace std;
    using namespace mcfile;

    Pos3i startPos = center;
    optional<PistonTileEntity> startBlock = PistonTileEntity::From(loader.tileEntityAt(center), center);
    if (!startBlock) {
      return nullopt;
    }

    static int constexpr kMaxMovableBlocksByAPiston = 12;

    unordered_map<Pos3i, optional<PistonTileEntity>, Pos3iHasher> testedBlocks;
    unordered_set<Pos3i, Pos3iHasher> attachedBlocks;

    testedBlocks.insert(make_pair(startPos, startBlock));

    unordered_set<Pos3i, Pos3iHasher> testBlocks;
    testBlocks.insert(startPos);
    attachedBlocks.insert(startPos);

    int maxSize = kMaxMovableBlocksByAPiston * 2 + 1;
    int maxTestedBlocks = maxSize * maxSize * maxSize;
    while (!testBlocks.empty() && attachedBlocks.size() <= kMaxMovableBlocksByAPiston && testedBlocks.size() < maxTestedBlocks) {
      for (Pos3i pos : testBlocks) {
        Pos3i testPos = pos - VectorOfFacing(facing);
        if (testedBlocks.find(testPos) != testedBlocks.end()) {
          continue;
        }
        auto test = PistonTileEntity::From(loader.tileEntityAt(testPos), testPos);
        if (!test) {
          continue;
        }
        if (test->fFacing != facing) {
          continue;
        }
        if (!test->fSource) {
          continue;
        }
        if (test->fName == u8"minecraft:piston" || test->fName == u8"minecraft:sticky_piston") {
          return testPos;
        } else if (test->fName == u8"minecraft:piston_head") {
          return testPos - VectorOfFacing(facing);
        }
      }

      vector<pair<Pos3i, PistonTileEntity>> expanding;
      for (Pos3i pos : testBlocks) {
        auto found = testedBlocks.find(pos);
        if (found == testedBlocks.end()) {
          continue;
        }
        if (!found->second) {
          continue;
        }
        PistonTileEntity base = *found->second;
        for (int f6 = 0; f6 < 6; f6++) {
          Pos3i testPos = pos + VectorOfFacing(f6);
          if (testedBlocks.find(testPos) != testedBlocks.end()) {
            continue;
          }
          expanding.push_back(make_pair(testPos, base));
        }
      }
      testBlocks.clear();

      sort(expanding.begin(), expanding.end(), CompareByChunkDistance(center.fX, center.fZ));

      for (auto it : expanding) {
        Pos3i pos = it.first;
        PistonTileEntity base = it.second;
        auto target = PistonTileEntity::From(loader.tileEntityAt(pos), pos);
        testedBlocks.insert(make_pair(pos, target));
        if (!target) {
          continue;
        }
        if (IsBaseStickyAgainstTarget(*target, base)) {
          testBlocks.insert(pos);
          attachedBlocks.insert(pos);
        }
      }
    }
    return nullopt;
  }

  static std::function<bool(std::pair<Pos3i, PistonTileEntity> a, std::pair<Pos3i, PistonTileEntity> b)> CompareByChunkDistance(int bx, int bz) {
    using namespace mcfile;
    int cx = Coordinate::ChunkFromBlock(bx);
    int cz = Coordinate::ChunkFromBlock(bz);
    return [cx, cz](auto a, auto b) {
      int const acx = Coordinate::ChunkFromBlock(a.first.fX);
      int const acz = Coordinate::ChunkFromBlock(a.first.fZ);
      int const bcx = Coordinate::ChunkFromBlock(b.first.fX);
      int const bcz = Coordinate::ChunkFromBlock(b.first.fZ);
      int const aCost = abs(acx - cx) + abs(acz - cz);
      int const bCost = abs(bcx - cx) + abs(bcz - cz);
      return aCost < bCost;
    };
  }

  static bool IsBaseStickyAgainstTarget(PistonTileEntity const &base, PistonTileEntity const &target) {
    static std::u8string const slimeBlock = u8"minecraft:slime_block";
    static std::u8string const honeyBlock = u8"minecraft:honey_block";

    if (base.fFacing != target.fFacing) {
      return false;
    }

    if (target.fSource) {
      return false;
    }

    if (base.fName == slimeBlock) {
      return target.fName != honeyBlock;
    } else if (base.fName == honeyBlock) {
      return target.fName != slimeBlock;
    } else {
      return false;
    }
  }

  static Pos3i VectorOfFacing(int facing) {
    if (facing == 1) {
      // up
      return Pos3i(0, 1, 0);
    } else if (facing == 2) {
      // north
      return Pos3i(0, 0, -1);
    } else if (facing == 3) {
      // south
      return Pos3i(0, 0, 1);
    } else if (facing == 4) {
      // west
      return Pos3i(-1, 0, 0);
    } else if (facing == 5) {
      // east
      return Pos3i(1, 0, 0);
    } else {
      // 0, down
      return Pos3i(0, -1, 0);
    }
  }

  static bool ChunkHasPiston(mcfile::je::Chunk const &chunk) {
    for (auto const &section : chunk.fSections) {
      if (!section) {
        continue;
      }
      bool has = false;
      section->eachBlockPalette([&has](std::shared_ptr<mcfile::je::Block const> const &b, size_t) {
        if (b->fName == u8"minecraft:sticky_piston" || b->fName == u8"minecraft:piston" || b->fName == u8"minecraft:moving_piston") {
          has = true;
          return false;
        }
        return true;
      });
      if (has) {
        return true;
      }
    }
    return false;
  }
};

void MovingPiston::PreprocessChunk(mcfile::je::CachedChunkLoader &loader, mcfile::je::Chunk &chunk) {
  return Impl::PreprocessChunk(loader, chunk);
}

} // namespace je2be::tobe
