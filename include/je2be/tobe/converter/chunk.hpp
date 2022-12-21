#pragma once

namespace je2be::tobe {

class Chunk {
  Chunk() = delete;

public:
  struct Result {
    Result() : fData(nullptr), fOk(false) {}

    std::shared_ptr<WorldData> fData;
    bool fOk;
  };

  struct PlayerAttachedEntities {
    CompoundTagPtr fVehicle;
    std::vector<CompoundTagPtr> fPassengers;
    std::vector<CompoundTagPtr> fShoulderRiders;
    int64_t fLocalPlayerUid;
  };

public:
  static Result Convert(mcfile::Dimension dim,
                        DbInterface &db,
                        mcfile::je::Region region,
                        int cx, int cz,
                        JavaEditionMap mapInfo,
                        std::filesystem::path entitiesDir,
                        std::optional<PlayerAttachedEntities> playerAttachedEntities,
                        int64_t gameTick,
                        int difficultyBedrock,
                        bool allowCommand,
                        GameMode gameType) {
    using namespace std;
    using namespace mcfile;
    try {
      auto const &chunk = region.chunkAt(cx, cz);
      Chunk::Result r;
      r.fOk = true;
      if (!chunk) {
        return r;
      }
      if (chunk->status() != mcfile::je::Chunk::Status::FULL) {
        return r;
      }
      if (chunk->dataVersion() >= 2724) {
        vector<shared_ptr<CompoundTag>> entities;
        if (region.entitiesAt(cx, cz, entities)) {
          chunk->fEntities.swap(entities);
        }
      }
      r.fData = MakeWorldData(chunk, region, dim, db, mapInfo, entitiesDir, playerAttachedEntities, gameTick, difficultyBedrock, allowCommand, gameType);
      return r;
    } catch (std::exception &e) {
      Chunk::Result r;
      r.fData = make_shared<WorldData>(dim);
      r.fData->addError(Status::ErrorData(Status::Where(__FILE__, __LINE__), e.what()));
      r.fOk = false;
      return r;
    } catch (char const *what) {
      Chunk::Result r;
      r.fData = make_shared<WorldData>(dim);
      r.fData->addError(Status::ErrorData(Status::Where(__FILE__, __LINE__), what));
      r.fOk = false;
      return r;
    } catch (...) {
      Chunk::Result r;
      r.fData = make_shared<WorldData>(dim);
      r.fData->addError(Status::ErrorData(Status::Where(__FILE__, __LINE__)));
      r.fOk = false;
      return r;
    }
  }

  static std::shared_ptr<WorldData> MakeWorldData(std::shared_ptr<mcfile::je::Chunk> const &chunk,
                                                  mcfile::je::Region region,
                                                  mcfile::Dimension dim,
                                                  DbInterface &db,
                                                  JavaEditionMap const &mapInfo,
                                                  std::filesystem::path entitiesDir,
                                                  std::optional<PlayerAttachedEntities> playerAttachedEntities,
                                                  int64_t gameTick,
                                                  int difficultyBedrock,
                                                  bool allowCommand,
                                                  GameMode gameType) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;

    if (!chunk) {
      return nullptr;
    }
    mcfile::je::CachedChunkLoader loader(region);

    unordered_map<Pos3i, shared_ptr<mcfile::je::Block const>, Pos3iHasher> tickingLiquidOriginalBlocks;
    for (auto const &it : chunk->fLiquidTicks) {
      Pos3i pos(it.fX, it.fY, it.fZ);
      auto block = chunk->blockAt(pos.fX, pos.fY, pos.fZ);
      if (!block) {
        continue;
      }
      tickingLiquidOriginalBlocks[pos] = block;
    }

    PreprocessChunk(loader, *chunk);

    auto ret = make_shared<WorldData>(dim);
    ChunkConversionMode mode = ConversionMode(*chunk);

    ChunkDataPackage cdp(mode);
    ChunkData cd(chunk->fChunkX, chunk->fChunkZ, dim, mode);

    for (auto const &section : chunk->fSections) {
      if (!section) {
        continue;
      }
      if (!SubChunk::Convert(*chunk, dim, section->y(), cd, cdp, *ret)) {
        return nullptr;
      }
    }

    for (int i = 0; i < chunk->fLiquidTicks.size(); i++) {
      mcfile::je::TickingBlock tb = chunk->fLiquidTicks[i];

      int64_t time = chunk->fLastUpdate + tb.fT;

      auto blockJ = make_shared<mcfile::je::Block const>(tb.fI);
      auto blockB = BlockData::From(blockJ, nullptr);
      if (!blockB) {
        continue;
      }

      if (auto found = tickingLiquidOriginalBlocks.find(Pos3i(tb.fX, tb.fY, tb.fZ)); found != tickingLiquidOriginalBlocks.end()) {
        auto original = found->second;
        if (auto level = strings::Toi(original->property("level")); level) {
          if (auto st = blockB->compoundTag("states"); st) {
            st->set("liquid_depth", Int(*level));
          }
        }
      }

      auto tick = Compound();
      tick->set("blockState", blockB);
      tick->set("time", Long(time));
      tick->set("x", Int(tb.fX));
      tick->set("y", Int(tb.fY));
      tick->set("z", Int(tb.fZ));
      cdp.addLiquidTick(i, tick);
    }

    for (int i = 0; i < chunk->fTileTicks.size(); i++) {
      mcfile::je::TickingBlock tb = chunk->fTileTicks[i];

      int64_t time = chunk->fLastUpdate + tb.fT;

      auto blockJ = make_shared<mcfile::je::Block const>(tb.fI);
      auto blockB = BlockData::From(blockJ, chunk->tileEntityAt(tb.fX, tb.fY, tb.fZ));
      if (!blockB) {
        continue;
      }

      auto tick = Compound();
      tick->set("blockState", blockB);
      tick->set("time", Long(time));
      tick->set("x", Int(tb.fX));
      tick->set("y", Int(tb.fY));
      tick->set("z", Int(tb.fZ));
      cdp.addTileTick(i, tick);
    }

    ret->addStructures(*chunk);
    ret->updateChunkLastUpdate(*chunk);

    unordered_map<Pos2i, vector<shared_ptr<CompoundTag>>, Pos2iHasher> entities;
    Context ctx(mapInfo, *ret, gameTick, difficultyBedrock, allowCommand, gameType, TileEntity::FromBlockAndTileEntity);
    cdp.build(*chunk, ctx, entities);
    if (!cdp.serialize(cd)) {
      return nullptr;
    }

    if (playerAttachedEntities) {
      Pos2i chunkPos(chunk->fChunkX, chunk->fChunkZ);
      auto vehicleB = playerAttachedEntities->fVehicle;
      if (vehicleB) {
        if (auto linksTag = vehicleB->listTag("LinksTag"); linksTag) {
          auto replace = List<Tag::Type::Compound>();
          auto localPlayer = Compound();
          localPlayer->set("entityID", Long(playerAttachedEntities->fLocalPlayerUid));
          localPlayer->set("linkID", Int(0));
          replace->push_back(localPlayer);
          for (int i = 0; i < linksTag->size(); i++) {
            auto item = linksTag->at(i);
            if (auto link = dynamic_pointer_cast<CompoundTag>(item); link) {
              link->set("linkID", Int(i + 1));
              replace->push_back(link);
            }
          }
          vehicleB->set("LinksTag", replace);
        }
        entities[chunkPos].push_back(vehicleB);
      }

      copy(playerAttachedEntities->fPassengers.begin(), playerAttachedEntities->fPassengers.end(), back_inserter(entities[chunkPos]));
      copy(playerAttachedEntities->fShoulderRiders.begin(), playerAttachedEntities->fShoulderRiders.end(), back_inserter(entities[chunkPos]));
    }

    if (!cd.put(db)) {
      return nullptr;
    }

    if (!entities.empty()) {
      Fs::CreateDirectories(entitiesDir);

      for (auto const &it : entities) {
        Pos2i p = it.first;
        auto file = entitiesDir / ("c." + to_string(p.fX) + "." + to_string(p.fZ) + ".nbt");
        auto stream = make_shared<FileOutputStream>(file);
        OutputStreamWriter writer(stream, Endian::Little);
        for (auto const &e : it.second) {
          if (!CompoundTag::Write(*e, writer)) {
            return nullptr;
          }
        }
      }
    }

    return ret;
  }

private:
  static void PreprocessChunk(mcfile::je::CachedChunkLoader &loader, mcfile::je::Chunk &chunk) {
    MovingPiston::PreprocessChunk(loader, chunk);
    InjectTickingLiquidBlocksAsBlocks(chunk);
    SortTickingBlocks(chunk.fTileTicks);
    SortTickingBlocks(chunk.fLiquidTicks);
    CompensateKelp(chunk);
    MushroomBlock(loader, chunk);
  }

  static void SortTickingBlocks(std::vector<mcfile::je::TickingBlock> &blocks) {
    std::stable_sort(blocks.begin(), blocks.end(), [](auto a, auto b) {
      return a.fP < b.fP;
    });
  }

  static void InjectTickingLiquidBlocksAsBlocks(mcfile::je::Chunk &chunk) {
    for (mcfile::je::TickingBlock const &tb : chunk.fLiquidTicks) {
      auto liquid = std::make_shared<mcfile::je::Block>(tb.fI);
      auto before = chunk.blockAt(tb.fX, tb.fY, tb.fZ);
      if (!before) {
        continue;
      }
      if (before->fName == "minecraft:lava") {
        if (liquid->fName == "minecraft:lava" || liquid->fName == "minecraft:flowing_lava") {
          chunk.setBlockAt(tb.fX, tb.fY, tb.fZ, liquid);
        }
      } else if (before->fName == "minecraft:water") {
        if (liquid->fName == "minecraft:water" || liquid->fName == "minecraft:flowing_water") {
          chunk.setBlockAt(tb.fX, tb.fY, tb.fZ, liquid);
        }
      }
    }
  }

  static ChunkConversionMode ConversionMode(mcfile::je::Chunk const &chunk) {
    if (chunk.minBlockY() < 0 || 256 <= chunk.maxBlockY() || chunk.dataVersion() >= mcfile::je::chunksection::ChunkSectionGenerator::kMinDataVersionChunkSection118) {
      return ChunkConversionMode::CavesAndCliffs2;
    } else {
      return ChunkConversionMode::Legacy;
    }
  }

  static void CompensateKelp(mcfile::je::Chunk &chunk) {
    using namespace std;
    using namespace mcfile::je;
    bool hasKelp = false;
    for (auto const &section : chunk.fSections) {
      if (!section) {
        continue;
      }
      section->eachBlockPalette([&hasKelp](shared_ptr<Block const> const &block, size_t) {
        if (block->fName == "minecraft:kelp_plant") {
          hasKelp = true;
          return false;
        }
        return true;
      });
      if (hasKelp) {
        break;
      }
    }
    if (!hasKelp) {
      return;
    }
    for (int x = chunk.minBlockX(); x <= chunk.maxBlockX(); x++) {
      for (int z = chunk.minBlockZ(); z <= chunk.maxBlockZ(); z++) {
        int minY = chunk.minBlockY();
        int maxY = chunk.maxBlockY();
        shared_ptr<Block const> lower;
        for (int y = minY; y <= maxY; y++) {
          auto block = chunk.blockAt(x, y, z);
          if (!block) {
            break;
          }
          if (lower) {
            if ((block->fName == "minecraft:kelp_plant" || block->fName == "minecraft:kelp") && lower->fName == "minecraft:water") {
              auto kelpPlant = make_shared<Block const>("minecraft:kelp_plant");
              for (int i = y - 1; i >= minY; i--) {
                auto b = chunk.blockAt(x, i, z);
                if (!b) {
                  break;
                }
                if (b->fName == "minecraft:water") {
                  chunk.setBlockAt(x, i, z, kelpPlant);
                } else {
                  break;
                }
              }
            } else if (block->fName == "minecraft:water" && lower->fName == "minecraft:kelp_plant") {
              map<string, string> props;
              props["age"] = "22";
              auto kelp = make_shared<Block const>("minecraft:kelp", props);
              chunk.setBlockAt(x, y, z, kelp);
              block = kelp;
            }
          }
          lower = block;
        }
      }
    }
  }

  static void MushroomBlock(mcfile::je::CachedChunkLoader &loader, mcfile::je::Chunk &chunk) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::je;
    using namespace mcfile::blocks::minecraft;
    bool hasMsurhoom = false;
    for (auto const &section : chunk.fSections) {
      if (!section) {
        continue;
      }
      section->eachBlockPalette([&hasMsurhoom](shared_ptr<Block const> const &block, size_t) {
        if (block->fId == red_mushroom_block || block->fId == brown_mushroom) {
          hasMsurhoom = true;
          return false;
        }
        return true;
      });
      if (hasMsurhoom) {
        break;
      }
    }
    if (!hasMsurhoom) {
      return;
    }
    static vector<pair<string, Pos3i>> directions({{"up", Pos3i(0, 1, 0)},
                                                   {"down", Pos3i(0, -1, 0)},
                                                   {"north", Pos3i(0, 0, -1)},
                                                   {"east", Pos3i(1, 0, 0)},
                                                   {"south", Pos3i(0, 0, 1)},
                                                   {"west", Pos3i(-1, 0, 0)}});
    for (int x = chunk.minBlockX(); x <= chunk.maxBlockX(); x++) {
      for (int z = chunk.minBlockZ(); z <= chunk.maxBlockZ(); z++) {
        int minY = chunk.minBlockY();
        int maxY = chunk.maxBlockY();
        for (int y = minY; y <= maxY; y++) {
          auto center = chunk.blockAt(x, y, z);
          if (!center) {
            continue;
          }
          auto id = center->fId;
          if (id != red_mushroom_block && id != brown_mushroom_block) {
            continue;
          }
          map<string, optional<string>> props;
          for (auto const &dir : directions) {
            auto pos = Pos3i(x, y, z) + dir.second;
            auto block = loader.blockAt(pos);
            if (block && !mcfile::blocks::IsTransparent(block->fId)) {
              props[dir.first] = nullopt;
            }
          }
          auto replace = center->applying(props);
          chunk.setBlockAt(x, y, z, replace);
        }
      }
    }
  }
};

} // namespace je2be::tobe
