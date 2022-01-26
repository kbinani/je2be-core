#pragma once

namespace je2be::toje {

class Region {
public:
  std::unordered_set<Pos2i, Pos2iHasher> fChunks;

  static bool Convert(mcfile::Dimension d, std::unordered_set<Pos2i, Pos2iHasher> chunks, int rx, int rz, leveldb::DB *db, std::filesystem::path destination) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;
    namespace fs = std::filesystem;

    auto dir = File::CreateTempDir(fs::temp_directory_path());
    if (!dir) {
      return false;
    }

    defer {
      error_code ec;
      fs::remove_all(*dir, ec);
    };

    for (int cz = rz * 32; cz < rz * 32 + 32; cz++) {
      unique_ptr<ChunkCache<3, 3>> cache(new ChunkCache<3, 3>(d, rx * 32, cz));
      for (int cx = rx * 32; cx < rx * 32 + 32; cx++) {
        defer {
          unique_ptr<ChunkCache<3, 3>> next(new ChunkCache<3, 3>(d, cx + 1, cz));
          next->set(cx + 1, cz, cache->at(cx + 1, cz));
          cache.swap(next);
        };
        Pos2i p(cx, cz);
        auto found = chunks.find(p);
        if (found == chunks.end()) {
          continue;
        }
        cache->load(cx, cz, *db);
        auto b = cache->at(cx, cz);
        if (!b) {
          continue;
        }
        cache->load(cx, cz - 1, *db);
        cache->load(cx + 1, cz, *db);
        cache->load(cx, cz + 1, *db);
        cache->load(cx - 1, cz, *db);

        auto j = mcfile::je::WritableChunk::MakeEmpty(cx, cz);

        for (auto const &sectionB : b->fSubChunks) {
          if (!sectionB) {
            continue;
          }
          auto sectionJ = mcfile::je::chunksection::ChunkSection118::MakeEmpty(sectionB->fChunkY);
          vector<shared_ptr<mcfile::je::Block const>> paletteJ;

          for (size_t idx = 0; idx < sectionB->fPalette.size(); idx++) {
            auto const &blockB = sectionB->fPalette[idx];
            auto blockJ = BlockData::From(*blockB);
            assert(blockJ);
            paletteJ.push_back(blockJ);
          }

          vector<uint16_t> indicesJ(4096);
          for (int x = 0, indexB = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
              for (int y = 0; y < 16; y++, indexB++) {
                int indexJ = *mcfile::je::chunksection::ChunkSection118::BlockIndex(x, y, z);
                indicesJ[indexJ] = sectionB->fPaletteIndices[indexB];
              }
            }
          }

          // waterlogged=true variant of palette[index] is stored at palette[waterLogged[index]], if waterLogged[index] >= 0.
          vector<int> waterLoggedJ(paletteJ.size(), -1);

          if (sectionB->fChunkY == 4) {
            int a = 0;
          }

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
            return false;
          }
          int sectionIndex = sectionJ->fY - j->fChunkY;
          if (j->fSections.size() <= sectionIndex) {
            j->fSections.resize(sectionIndex + 1);
          }
          j->fSections[sectionIndex] = sectionJ;
        }

        //TODO: "distance" of leaves
        //TODO: colored standing banner
        //TODO: colored bed
        //TODO: "type" of chest, trapped_chest
        //TODO: type of skull
        //TODO: "lit" of furnace
        //TODO: "has_record" of jukebox
        //TODO: movingBlock
        //TODO: "instrument", "note", "powered" of noteblock
        //TODO: "extended" of piston, sticky_piston
        //TODO: "lit" of redstone_torch
        //TODO: east,north,south,west of redstone_wire
        //TODO: "locked" of repeater
        //TODO: "bottom" of scafforlding
        //TODO: power,sculk_sensor_phase of sculk_sensor
        //TODO: facing of shulker_box, undyed_shulker_box
        //TODO: "lit" of smoker
        //TODO: "power" of target
        //TODO: east,north,south,west of tripwire

        BlockPropertyAccessor accessor(*b);

        ShapeOfStairs::Do(*j, *cache, accessor);
        Kelp::Do(*j, *cache, accessor);
        TwistingVines::Do(*j, *cache, accessor);
        WeepingVines::Do(*j, *cache, accessor);
        AttachedStem::Do(*j, *cache, accessor);
        CaveVines::Do(*j, *cache, accessor);
        Snowy::Do(*j, *cache, accessor);
        ChorusPlant::Do(*j, *cache, accessor);
        FenceConnectable::Do(*j, *cache, accessor);

        auto fos = make_shared<FileOutputStream>(*dir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz));
        if (!fos) {
          return false;
        }
        if (!j->write(*fos)) {
          return false;
        }
      }
    }

    auto mca = destination / mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
    return mcfile::je::Region::ConcatCompressedNbt(rx, rz, *dir, mca);
  }
};

} // namespace je2be::toje
