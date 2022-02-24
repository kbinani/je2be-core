#pragma once

namespace je2be::tobe {

class MovingPiston {
private:
  MovingPiston() = delete;

public:
  static void PreprocessChunk(mcfile::je::CachedChunkLoader &loader, mcfile::je::Chunk &chunk) {
    using namespace mcfile;
    using namespace mcfile::je;
    using namespace std;
    using namespace je2be::nbt;

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
            if (block->fName != "minecraft:sticky_piston" && block->fName != "minecraft:piston") {
              continue;
            }
            if (loader.tileEntityAt(pos)) {
              continue;
            }
            Facing6 f6 = Facing6FromJavaName(block->property("facing", ""));
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

              auto pistonArm = make_shared<CompoundTag>();
              auto attachedBlocksTag = make_shared<ListTag>(Tag::Type::Int);
              for (auto attachedBlock : attachedBlocks) {
                Pos3i actual = attachedBlock - vec;
                attachedBlocksTag->push_back(Int(actual.fX));
                attachedBlocksTag->push_back(Int(actual.fY));
                attachedBlocksTag->push_back(Int(actual.fZ));
              }
              pistonArm->set("AttachedBlocks", attachedBlocksTag);
              pistonArm->set("BreakBlocks", make_shared<ListTag>(Tag::Type::Int));
              pistonArm->set("LastProgress", Float(0));
              pistonArm->set("NewState", Byte(1));
              pistonArm->set("Progress", Float(0.5));
              pistonArm->set("State", Byte(1));
              pistonArm->set("Sticky", Bool(block->fName == "minecraft:sticky_piston"));
              pistonArm->set("id", String("j2b:PistonArm"));
              pistonArm->set("isMovable", Bool(false));
              pistonArm->set("x", Int(pos.fX));
              pistonArm->set("y", Int(pos.fY));
              pistonArm->set("z", Int(pos.fZ));

              tileEntityReplacement[pos] = pistonArm;
            } else {
              // Tile entity doesn't exist at piston_head position.
              // This means the piston is in static state.
              bool extended = block->property("extended", "false") == "true";
              auto pistonArm = make_shared<CompoundTag>();
              pistonArm->set("AttachedBlocks", make_shared<ListTag>(Tag::Type::Int));
              pistonArm->set("BreakBlocks", make_shared<ListTag>(Tag::Type::Int));
              pistonArm->set("LastProgress", Float(extended ? 1 : 0));
              pistonArm->set("NewState", Byte(extended ? 2 : 0));
              pistonArm->set("Progress", Float(extended ? 1 : 0));
              pistonArm->set("State", Byte(extended ? 2 : 0));
              pistonArm->set("Sticky", Bool(block->fName == "minecraft:sticky_piston"));
              pistonArm->set("id", String("j2b:PistonArm"));
              pistonArm->set("isMovable", Bool(false));
              pistonArm->set("x", Int(pos.fX));
              pistonArm->set("y", Int(pos.fY));
              pistonArm->set("z", Int(pos.fZ));

              tileEntityReplacement[pos] = pistonArm;
            }
          }
        }
      }
    }

    for (auto it : chunk.fTileEntities) {
      Pos3i pos = it.first;
      auto const &item = it.second;
      auto id = item->string("id");
      if (!id) {
        continue;
      }
      if (id != "minecraft:piston") {
        continue;
      }
      auto extending = item->boolean("extending");
      if (!extending) {
        continue;
      }
      auto source = item->boolean("source");
      if (!source) {
        continue;
      }
      auto facing = item->int32("facing");
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

          map<string, string> props;
          props["facing_direction"] = to_string(*facing);
          string name = block->fName == "minecraft:sticky_piston" ? "j2b:stickyPistonArmCollision" : "j2b:pistonArmCollision";
          auto newBlock = make_shared<Block>(name, props);
          chunk.setBlockAt(pos, newBlock, withoutRemovingTileEntity);

          tileEntityReplacement[pos] = nullptr;
        } else {
          // extending = 0, source = 1
          auto block = chunk.blockAt(pos);
          if (!block) {
            continue;
          }

          auto sticky = block->property("type", "normal") == "sticky";
          string name = sticky ? "minecraft:sticky_piston" : "minecraft:piston";
          auto newBlock = make_shared<Block>(name, block->fProperties);
          chunk.setBlockAt(pos, newBlock, withoutRemovingTileEntity);

          unordered_set<Pos3i, Pos3iHasher> attachedBlocks;
          LookupAttachedBlocks(loader, pos, *extending, *facing, attachedBlocks);

          auto pistonArm = make_shared<CompoundTag>();
          auto attachedBlocksTag = make_shared<ListTag>(Tag::Type::Int);
          for (auto attachedBlock : attachedBlocks) {
            attachedBlocksTag->push_back(Int(attachedBlock.fX));
            attachedBlocksTag->push_back(Int(attachedBlock.fY));
            attachedBlocksTag->push_back(Int(attachedBlock.fZ));
          }
          pistonArm->set("AttachedBlocks", attachedBlocksTag);
          pistonArm->set("BreakBlocks", make_shared<ListTag>(Tag::Type::Int));
          pistonArm->set("LastProgress", Float(0.5));
          pistonArm->set("NewState", Byte(3));
          pistonArm->set("Progress", Float(0));
          pistonArm->set("State", Byte(3));
          pistonArm->set("Sticky", Bool(sticky));
          pistonArm->set("id", String("j2b:PistonArm"));
          pistonArm->set("isMovable", Bool(false));
          pistonArm->set("x", Int(pos.fX));
          pistonArm->set("y", Int(pos.fY));
          pistonArm->set("z", Int(pos.fZ));

          tileEntityReplacement[pos] = pistonArm;
        }
      } else {
        // extending = *, source = 0
        auto e = MovingBlockEntityFromPistonTileEntity(pos, *facing, item, loader);
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
  static std::shared_ptr<CompoundTag> MovingBlockEntityFromPistonTileEntity(Pos3i pos, int facing, std::shared_ptr<CompoundTag const> const &item, mcfile::je::CachedChunkLoader &loader) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::je;
    using namespace je2be::nbt;

    auto e = make_shared<CompoundTag>();
    e->set("id", String("j2b:MovingBlock"));
    e->set("isMovable", Bool(true));

    auto blockState = item->compoundTag("blockState");
    if (!blockState) {
      return nullptr;
    }
    auto name = blockState->string("Name");
    if (!name) {
      return nullptr;
    }
    auto properties = blockState->compoundTag("Properties");
    map<string, string> props;
    if (properties) {
      for (auto p : properties->fValue) {
        string key = p.first;
        StringTag const *s = p.second->asString();
        if (s == nullptr) {
          continue;
        }
        props[key] = s->fValue;
      }
    }
    auto block = make_shared<Block const>(*name, props);
    auto movingBlock = BlockData::From(block);
    if (!movingBlock) {
      return nullptr;
    }
    e->set("movingBlock", movingBlock);

    auto movingBlockExtra = make_shared<CompoundTag>();
    movingBlockExtra->set("name", String("minecraft:air"));
    movingBlockExtra->set("states", make_shared<CompoundTag>());
    movingBlockExtra->set("version", Int(kBlockDataVersion));
    e->set("movingBlockExtra", movingBlockExtra);

    auto pistonPos = LookupPistonPos(loader, pos, facing);
    if (pistonPos) {
      e->set("pistonPosX", Int(pistonPos->fX));
      e->set("pistonPosY", Int(pistonPos->fY));
      e->set("pistonPosZ", Int(pistonPos->fZ));
    }

    e->set("x", Int(pos.fX));
    e->set("y", Int(pos.fY));
    e->set("z", Int(pos.fZ));

    return e;
  }

  struct PistonTileEntity {
    std::string fName;
    int fFacing;
    bool fExtending;
    bool fSource;
    Pos3i fPos;

    PistonTileEntity(std::string name, int facing, bool extending, bool source, Pos3i pos)
        : fName(name), fFacing(facing), fExtending(extending), fSource(source), fPos(pos) {}

    static std::optional<PistonTileEntity> From(std::shared_ptr<CompoundTag const> const &tag, Pos3i pos) {
      if (!tag) {
        return std::nullopt;
      }
      auto id = tag->string("id");
      if (!id) {
        return std::nullopt;
      }
      if (*id != "minecraft:piston") {
        return std::nullopt;
      }
      auto state = tag->compoundTag("blockState");
      if (!state) {
        return std::nullopt;
      }
      auto name = state->string("Name");
      if (!name) {
        return std::nullopt;
      }
      auto facing = tag->int32("facing");
      if (!facing) {
        return std::nullopt;
      }
      auto extending = tag->boolean("extending");
      if (!extending) {
        return std::nullopt;
      }
      auto source = tag->boolean("source");
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

    static std::string const slimeBlock = "minecraft:slime_block";
    static std::string const honeyBlock = "minecraft:honey_block";
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
        if (test->fName == "minecraft:piston" || test->fName == "minecraft:sticky_piston") {
          return testPos;
        } else if (test->fName == "minecraft:piston_head") {
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
        for (int facing = 0; facing < 6; facing++) {
          Pos3i testPos = pos + VectorOfFacing(facing);
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
    static std::string const slimeBlock = "minecraft:slime_block";
    static std::string const honeyBlock = "minecraft:honey_block";

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
      section->eachBlockPalette([&has](mcfile::je::Block const &b) {
        if (b.fName == "minecraft:sticky_piston" || b.fName == "minecraft:piston") {
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

} // namespace je2be::tobe
