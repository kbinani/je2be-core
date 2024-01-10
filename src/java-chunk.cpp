#include "java/_chunk.hpp"

#include <je2be/fs.hpp>
#include <je2be/status.hpp>

#include "db/_db-interface.hpp"
#include "enums/_chunk-conversion-mode.hpp"
#include "enums/_game-mode.hpp"
#include "java/_chunk-data-package.hpp"
#include "java/_chunk-data.hpp"
#include "java/_context.hpp"
#include "java/_entity-store.hpp"
#include "java/_java-edition-map.hpp"
#include "java/_moving-piston.hpp"
#include "java/_sub-chunk.hpp"
#include "java/_world-data.hpp"

namespace je2be::java {

class WorldData;

class Chunk::Impl {
  Impl() = delete;

public:
  static Result Convert(mcfile::Dimension dim,
                        DbInterface &db,
                        mcfile::je::McaEditor &terrain,
                        mcfile::je::McaEditor *entities,
                        int cx, int cz,
                        JavaEditionMap mapInfo,
                        std::shared_ptr<EntityStore> const &entityStore,
                        std::optional<PlayerAttachedEntities> playerAttachedEntities,
                        i64 gameTick,
                        int difficultyBedrock,
                        bool allowCommand,
                        GameMode gameType) {
    using namespace std;
    using namespace mcfile;
    try {
      int rx = mcfile::Coordinate::RegionFromChunk(cx);
      int rz = mcfile::Coordinate::RegionFromChunk(cz);
      int localChunkX = cx - rx * 32;
      int localChunkZ = cz - rz * 32;
      auto root = terrain.get(localChunkX, localChunkZ);
      auto const &chunk = mcfile::je::Chunk::MakeChunk(cx, cz, root);
      Chunk::Result r;
      r.fOk = true;
      if (!chunk) {
        return r;
      }
      if (chunk->status() != mcfile::je::Chunk::Status::FULL) {
        return r;
      }
      if (chunk->getDataVersion() >= 2724 && entities) {
        if (auto nbt = entities->get(localChunkX, localChunkZ); nbt) {
          if (auto list = nbt->listTag(u8"Entities"); list) {
            for (auto const &it : *list) {
              if (auto e = std::dynamic_pointer_cast<CompoundTag>(it); e) {
                chunk->fEntities.push_back(e);
              }
            }
          }
        }
      }
      auto [data, st] = MakeWorldData(chunk, terrain, dim, db, mapInfo, entityStore, playerAttachedEntities, gameTick, difficultyBedrock, allowCommand, gameType);
      if (!data) {
        data = make_shared<WorldData>(dim);
      }
      if (!st.ok()) {
        data->addError(*st.error());
      }
      r.fData = data;
      r.fOk = st.ok();
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

  static std::pair<std::shared_ptr<WorldData>, Status> MakeWorldData(
      std::shared_ptr<mcfile::je::Chunk> const &chunk,
      mcfile::je::McaEditor &terrain,
      mcfile::Dimension dim,
      DbInterface &db,
      JavaEditionMap const &mapInfo,
      std::shared_ptr<EntityStore> const &entityStore,
      std::optional<PlayerAttachedEntities> playerAttachedEntities,
      i64 gameTick,
      int difficultyBedrock,
      bool allowCommand,
      GameMode gameType) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;

    if (!chunk) {
      return make_pair(nullptr, Status::Ok());
    }
    mcfile::je::CachedChunkLoader loader(terrain);

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
      if (auto st = SubChunk::Convert(*chunk, dim, section->y(), cd, cdp, *ret); !st.ok()) {
        return make_pair(nullptr, JE2BE_ERROR_PUSH(st));
      }
    }

    for (int i = 0; i < chunk->fLiquidTicks.size(); i++) {
      mcfile::je::TickingBlock tb = chunk->fLiquidTicks[i];

      i64 time = chunk->fLastUpdate + tb.fT;

      auto blockJ = mcfile::je::Block::FromName(tb.fI, chunk->getDataVersion());
      auto blockB = BlockData::From(blockJ, nullptr, {});
      if (!blockB) {
        continue;
      }

      if (auto found = tickingLiquidOriginalBlocks.find(Pos3i(tb.fX, tb.fY, tb.fZ)); found != tickingLiquidOriginalBlocks.end()) {
        auto original = found->second;
        if (auto level = strings::ToI32(original->property(u8"level")); level) {
          if (auto st = blockB->compoundTag(u8"states"); st) {
            st->set(u8"liquid_depth", Int(*level));
          }
        }
      }

      auto tick = Compound();
      tick->set(u8"blockState", blockB);
      tick->set(u8"time", Long(time));
      tick->set(u8"x", Int(tb.fX));
      tick->set(u8"y", Int(tb.fY));
      tick->set(u8"z", Int(tb.fZ));
      cdp.addLiquidTick(i, tick);
    }

    for (int i = 0; i < chunk->fTileTicks.size(); i++) {
      mcfile::je::TickingBlock tb = chunk->fTileTicks[i];

      i64 time = chunk->fLastUpdate + tb.fT;

      auto blockJ = mcfile::je::Block::FromName(tb.fI, chunk->getDataVersion());
      auto blockB = BlockData::From(blockJ, chunk->tileEntityAt(tb.fX, tb.fY, tb.fZ), {});
      if (!blockB) {
        continue;
      }

      auto tick = Compound();
      tick->set(u8"blockState", blockB);
      tick->set(u8"time", Long(time));
      tick->set(u8"x", Int(tb.fX));
      tick->set(u8"y", Int(tb.fY));
      tick->set(u8"z", Int(tb.fZ));
      cdp.addTileTick(i, tick);
    }

    ret->addStructures(*chunk);
    ret->updateChunkLastUpdate(*chunk);

    unordered_map<Pos2i, vector<shared_ptr<CompoundTag>>, Pos2iHasher> entities;
    Context ctx(mapInfo, *ret, gameTick, difficultyBedrock, allowCommand, gameType);
    cdp.build(*chunk, ctx, entities);
    if (!cdp.serialize(cd)) {
      return make_pair(nullptr, Status::Ok());
    }
    for (auto const &ex : ctx.fExperiments) {
      ret->addExperiment(ex);
    }

    if (playerAttachedEntities) {
      Pos2i chunkPos(chunk->fChunkX, chunk->fChunkZ);
      auto vehicleB = playerAttachedEntities->fVehicle;
      if (vehicleB) {
        if (auto linksTag = vehicleB->listTag(u8"LinksTag"); linksTag) {
          auto replace = List<Tag::Type::Compound>();
          auto localPlayer = Compound();
          localPlayer->set(u8"entityID", Long(playerAttachedEntities->fLocalPlayerUid));
          localPlayer->set(u8"linkID", Int(0));
          replace->push_back(localPlayer);
          for (int i = 0; i < linksTag->size(); i++) {
            auto item = linksTag->at(i);
            if (auto link = dynamic_pointer_cast<CompoundTag>(item); link) {
              link->set(u8"linkID", Int(i + 1));
              replace->push_back(link);
            }
          }
          vehicleB->set(u8"LinksTag", replace);
        }
        entities[chunkPos].push_back(vehicleB);
      }

      copy(playerAttachedEntities->fPassengers.begin(), playerAttachedEntities->fPassengers.end(), back_inserter(entities[chunkPos]));
      copy(playerAttachedEntities->fShoulderRiders.begin(), playerAttachedEntities->fShoulderRiders.end(), back_inserter(entities[chunkPos]));
    }

    if (auto st = cd.put(db); !st.ok()) {
      return make_pair(nullptr, JE2BE_ERROR_PUSH(st));
    }

    if (!entities.empty()) {
      for (auto const &it : entities) {
        vector<i64> uuids;
        for (auto const &entity : it.second) {
          auto id = entity->int64(u8"UniqueID");
          if (!id) {
            assert(false);
            continue;
          }
          uuids.push_back(*id);

          i64 v = *id;
          string prefix;
          prefix.assign((char const *)&v, sizeof(v));

          auto key = mcfile::be::DbKey::Actorprefix(prefix);
          optional<string> value = CompoundTag::Write(*entity, Endian::Little);
          if (!value) {
            return make_pair(ret, JE2BE_ERROR);
          }
          if (auto st = db.put(key, leveldb::Slice(*value)); !st.ok()) {
            return make_pair(ret, JE2BE_ERROR_PUSH(st));
          }
        }

        Pos2i entityChunk = it.first;
        Pos2i fromChunk(chunk->fChunkX, chunk->fChunkZ);
        entityStore->add(uuids, entityChunk, fromChunk);
      }
    }

    return make_pair(ret, Status::Ok());
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
      auto liquid = mcfile::je::Block::FromName(tb.fI, chunk.getDataVersion());
      auto before = chunk.blockAt(tb.fX, tb.fY, tb.fZ);
      if (!before) {
        continue;
      }
      if (before->fName == u8"minecraft:lava") {
        if (liquid->fName == u8"minecraft:lava" || liquid->fName == u8"minecraft:flowing_lava") {
          chunk.setBlockAt(tb.fX, tb.fY, tb.fZ, liquid);
        }
      } else if (before->fName == u8"minecraft:water") {
        if (liquid->fName == u8"minecraft:water" || liquid->fName == u8"minecraft:flowing_water") {
          chunk.setBlockAt(tb.fX, tb.fY, tb.fZ, liquid);
        }
      }
    }
  }

  static ChunkConversionMode ConversionMode(mcfile::je::Chunk const &chunk) {
    if (chunk.minBlockY() < 0 || 256 <= chunk.maxBlockY() || chunk.getDataVersion() >= mcfile::je::chunksection::ChunkSectionGenerator::kMinDataVersionChunkSection118) {
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
        if (block->fName == u8"minecraft:kelp_plant") {
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
            if ((block->fName == u8"minecraft:kelp_plant" || block->fName == u8"minecraft:kelp") && lower->fName == u8"minecraft:water") {
              auto kelpPlant = Block::FromId(mcfile::blocks::minecraft::kelp_plant, chunk.getDataVersion());
              for (int i = y - 1; i >= minY; i--) {
                auto b = chunk.blockAt(x, i, z);
                if (!b) {
                  break;
                }
                if (b->fName == u8"minecraft:water") {
                  chunk.setBlockAt(x, i, z, kelpPlant);
                } else {
                  break;
                }
              }
            } else if (block->fName == u8"minecraft:water" && lower->fName == u8"minecraft:kelp_plant") {
              map<u8string, u8string> props;
              props[u8"age"] = u8"22";
              auto kelp = Block::FromIdAndProperties(mcfile::blocks::minecraft::kelp, chunk.getDataVersion(), props);
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
    static vector<pair<u8string, Pos3i>> directions({{u8"up", Pos3i(0, 1, 0)},
                                                     {u8"down", Pos3i(0, -1, 0)},
                                                     {u8"north", Pos3i(0, 0, -1)},
                                                     {u8"east", Pos3i(1, 0, 0)},
                                                     {u8"south", Pos3i(0, 0, 1)},
                                                     {u8"west", Pos3i(-1, 0, 0)}});
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
          map<u8string, optional<u8string>> props;
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

Chunk::Result Chunk::Convert(mcfile::Dimension dim,
                             DbInterface &db,
                             mcfile::je::McaEditor &terrain,
                             mcfile::je::McaEditor *entities,
                             int cx, int cz,
                             JavaEditionMap mapInfo,
                             std::shared_ptr<EntityStore> const &entityStore,
                             std::optional<PlayerAttachedEntities> playerAttachedEntities,
                             i64 gameTick,
                             int difficultyBedrock,
                             bool allowCommand,
                             GameMode gameType) {
  return Impl::Convert(dim, db, terrain, entities, cx, cz, mapInfo, entityStore, playerAttachedEntities, gameTick, difficultyBedrock, allowCommand, gameType);
}

} // namespace je2be::java
