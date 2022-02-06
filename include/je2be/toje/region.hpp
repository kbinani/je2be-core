#pragma once

namespace je2be::toje {

class Region {
public:
  std::unordered_set<Pos2i, Pos2iHasher> fChunks;

  static std::shared_ptr<Context> Convert(mcfile::Dimension d, std::unordered_set<Pos2i, Pos2iHasher> chunks, int rx, int rz, leveldb::DB *db, std::filesystem::path destination, Context const &parentContext) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;
    namespace fs = std::filesystem;

    auto regionDir = File::CreateTempDir(fs::temp_directory_path());
    if (!regionDir) {
      return nullptr;
    }
    auto entityDir = File::CreateTempDir(fs::temp_directory_path());
    if (!entityDir) {
      return nullptr;
    }

    auto ctx = parentContext.make();

    defer {
      error_code ec1;
      fs::remove_all(*regionDir, ec1);
      error_code ec2;
      fs::remove_all(*entityDir, ec2);
    };

    for (int cz = rz * 32; cz < rz * 32 + 32; cz++) {
      unique_ptr<ChunkCache<3, 3>> cache(new ChunkCache<3, 3>(d, rx * 32 - 1, cz - 1, db));
      for (int cx = rx * 32; cx < rx * 32 + 32; cx++) {
        defer {
          unique_ptr<ChunkCache<3, 3>> next(cache->makeRelocated(cx, cz - 1));
          cache.swap(next);
        };

        assert(cache->fChunkX == cx - 1);
        assert(cache->fChunkZ == cz - 1);
        Pos2i p(cx, cz);
        auto found = chunks.find(p);
        if (found == chunks.end()) {
          continue;
        }
        auto b = cache->ensureLoadedAt(cx, cz);
        if (!b) {
          continue;
        }

        auto j = mcfile::je::WritableChunk::MakeEmpty(cx, cz);

        for (auto const &sectionB : b->fSubChunks) {
          if (!sectionB) {
            continue;
          }
          auto sectionJ = mcfile::je::chunksection::ChunkSection118::MakeEmpty(sectionB->fChunkY);
          vector<shared_ptr<mcfile::je::Block const>> paletteJ;

          vector<uint16_t> indicesJ(4096, 0);
          if (sectionB->fPaletteIndices.size() != 4096 || sectionB->fPalette.empty()) {
            paletteJ.push_back(make_shared<mcfile::je::Block const>("minecraft:air"));
          } else {
            for (size_t idx = 0; idx < sectionB->fPalette.size(); idx++) {
              auto const &blockB = sectionB->fPalette[idx];
              auto blockJ = BlockData::From(*blockB);
              assert(blockJ);
              paletteJ.push_back(blockJ);
            }
            for (int x = 0, indexB = 0; x < 16; x++) {
              for (int z = 0; z < 16; z++) {
                for (int y = 0; y < 16; y++, indexB++) {
                  int indexJ = *mcfile::je::chunksection::ChunkSection118::BlockIndex(x, y, z);
                  indicesJ[indexJ] = sectionB->fPaletteIndices[indexB];
                }
              }
            }
          }

          // waterlogged=true variant of palette[index] is stored at palette[waterLogged[index]], if waterLogged[index] >= 0.
          vector<int> waterLoggedJ(paletteJ.size(), -1);

          if (sectionB->fWaterPaletteIndices.size() == 4096) {
            vector<bool> isWaterB(sectionB->fWaterPalette.size());
            for (size_t i = 0; i < sectionB->fWaterPalette.size(); i++) {
              isWaterB[i] = sectionB->fWaterPalette[i]->fName == "minecraft:water";
            }
            for (int x = 0, indexB = 0; x < 16; x++) {
              for (int z = 0; z < 16; z++) {
                for (int y = 0; y < 16; y++, indexB++) {
                  uint16_t waterPaletteIndexB = sectionB->fWaterPaletteIndices[indexB];
                  if (!isWaterB[waterPaletteIndexB]) {
                    continue;
                  }

                  // (x, y, z) block is waterlogged
                  int indexJ = *mcfile::je::chunksection::ChunkSection118::BlockIndex(x, y, z);

                  uint16_t indexDryJ = indicesJ[indexJ];
                  auto dryBlockJ = paletteJ[indexDryJ];
                  int waterLoggedIndexJ = waterLoggedJ[indexDryJ];
                  if (waterLoggedIndexJ < 0) {
                    if (dryBlockJ->fProperties.find("waterlogged") == dryBlockJ->fProperties.end()) {
                      // This block can't be waterlogged in Java.
                      waterLoggedIndexJ = indexDryJ;
                      waterLoggedJ[indexDryJ] = indexDryJ;
                    } else {
                      map<string, string> props(dryBlockJ->fProperties);
                      props["waterlogged"] = "true";
                      auto waterLoggedBlockJ = make_shared<mcfile::je::Block const>(dryBlockJ->fName, props);
                      waterLoggedIndexJ = paletteJ.size();
                      paletteJ.push_back(waterLoggedBlockJ);
                      waterLoggedJ[indexDryJ] = waterLoggedIndexJ;
                    }
                  }
                  indicesJ[indexJ] = waterLoggedIndexJ;
                }
              }
            }
          }

          if (!sectionJ->fBlocks.reset(paletteJ, indicesJ)) {
            return nullptr;
          }
          int sectionIndex = sectionJ->fY - j->fChunkY;
          if (j->fSections.size() <= sectionIndex) {
            j->fSections.resize(sectionIndex + 1);
          }
          j->fSections[sectionIndex] = sectionJ;
        }

        for (int y = j->minBlockY(); y <= j->maxBlockY(); y += 4) {
          for (int z = j->minBlockZ(); z <= j->maxBlockZ(); z += 4) {
            for (int x = j->minBlockX(); x <= j->maxBlockX(); x += 4) {
              auto biome = b->biomeAt(x, y, z);
              if (biome) {
                j->setBiomeAt(x, y, z, *biome);
              }
            }
          }
        }

        for (auto const &it : b->fBlockEntities) {
          Pos3i const &pos = it.first;
          shared_ptr<CompoundTag> const &tagB = it.second;
          assert(tagB);
          if (!tagB) {
            continue;
          }
          auto const &blockB = b->blockAt(pos);
          if (!blockB) {
            continue;
          }
          auto const &blockJ = j->blockAt(pos);
          if (!blockJ) {
            continue;
          }
          auto result = BlockEntity::FromBlockAndBlockEntity(pos, *blockB, *tagB, *blockJ, *ctx);
          if (!result) {
            continue;
          }
          if (result->fTileEntity) {
            j->fTileEntities[pos] = result->fTileEntity;
          }
          if (result->fBlock) {
            mcfile::je::SetBlockOptions o;
            o.fRemoveTileEntity = false;
            j->setBlockAt(pos, result->fBlock, o);
          }
        }

        BlockPropertyAccessor accessor(*b);

        Piston::Do(*j, *cache, accessor);

        ShapeOfStairs::Do(*j, *cache, accessor);
        Kelp::Do(*j, *cache, accessor);
        TwistingVines::Do(*j, *cache, accessor);
        WeepingVines::Do(*j, *cache, accessor);
        AttachedStem::Do(*j, *cache, accessor);
        CaveVines::Do(*j, *cache, accessor);
        Snowy::Do(*j, *cache, accessor);
        ChorusPlant::Do(*j, *cache, accessor);
        FenceConnectable::Do(*j, *cache, accessor);
        Campfire::Do(*j, *cache, accessor);
        NoteBlock::Do(*j, *cache, accessor);
        RedstoneWire::Do(*j, *cache, accessor);
        Tripwire::Do(*j, *cache, accessor);
        Beacon::Do(*j, *cache, accessor);

        for (auto const &it : b->fBlockEntities) {
          auto id = it.second->string("id");
          if (!id) {
            continue;
          }
          if (id != "ItemFrame" && id != "GlowItemFrame") {
            continue;
          }
          Pos3i pos = it.first;
          auto blockB = cache->blockAt(pos.fX, pos.fY, pos.fZ);
          if (!blockB) {
            continue;
          }
          auto frameJ = Entity::ItemFrameFromBedrock(d, pos, *blockB, *it.second, *ctx);
          if (frameJ) {
            j->fEntities.push_back(frameJ);
          }
        }

        ChunkContext cctx(*ctx);
        unordered_map<Uuid, shared_ptr<CompoundTag>, UuidHasher, UuidPred> entities;
        for (auto const &entityB : b->fEntities) {
          auto result = Entity::From(*entityB, cctx);
          if (!result) {
            continue;
          }
          entities[result->fUuid] = result->fEntity;
        }
        for (auto const &it : cctx.fPassengers) {
          Uuid vehicleUuid = it.first;
          auto found = entities.find(vehicleUuid);
          if (found == entities.end()) {
            continue;
          }
          shared_ptr<CompoundTag> vehicle = found->second;
          auto passengers = make_shared<ListTag>(Tag::Type::Compound);
          for (auto const &passenger : it.second) {
            size_t passengerIndex = passenger.first;
            Uuid passengerUuid = passenger.second;
            auto f = entities.find(passengerUuid);
            if (f == entities.end()) {
              continue;
            }
            auto passenger = f->second;
            passengers->push_back(passenger);
            entities.erase(f);
          }
          vehicle->set("Passengers", passengers);
        }
        for (auto const &it : entities) {
          j->fEntities.push_back(it.second);
        }

        auto streamTerrain = make_shared<FileOutputStream>(*regionDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz));
        if (!j->write(*streamTerrain)) {
          return nullptr;
        }

        auto streamEntities = make_shared<FileOutputStream>(*entityDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz));
        if (!j->writeEntities(*streamEntities)) {
          return nullptr;
        }
      }
    }

    auto terrainMca = destination / "region" / mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
    if (!mcfile::je::Region::ConcatCompressedNbt(rx, rz, *regionDir, terrainMca)) {
      return nullptr;
    }

    auto entitiesMca = destination / "entities" / mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
    if (!mcfile::je::Region::ConcatCompressedNbt(rx, rz, *entityDir, entitiesMca)) {
      return nullptr;
    }

    return ctx;
  }
};

} // namespace je2be::toje
