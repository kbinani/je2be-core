#pragma once

namespace je2be::tobe {

class ChunkDataPackage {
public:
  explicit ChunkDataPackage(ChunkConversionMode mode)
      : fMode(mode),
        fHeightMap(std::make_shared<HeightMap>(mode == ChunkConversionMode::CavesAndCliffs2 ? -4 : 0)) {
  }

  void build(mcfile::je::Chunk const &chunk, Context const &ctx, std::unordered_map<Pos2i, std::vector<CompoundTagPtr>, Pos2iHasher> &entities) {
    buildEntities(chunk, ctx, entities);
    buildData2D(chunk, ctx.fWorldData.fDim);
    buildTileEntities(chunk, ctx);
    if (chunk.status() == mcfile::je::Chunk::Status::FULL) {
      fFinalizedState = 2;
    }
    ctx.fWorldData.addStatChunkVersion(chunk.fDataVersion);
    fChunkLastUpdate = chunk.fLastUpdate;
  }

  [[nodiscard]] bool serialize(ChunkData &cd) {
    if (!serializeData2D(cd)) {
      return false;
    }
    if (!serializeBlockEntity(cd)) {
      return false;
    }
    if (!serializePendingTicks(cd)) {
      return false;
    }
    cd.fFinalizedState = fFinalizedState;
    return true;
  }

  void updateAltitude(int x, int y, int z) {
    fHeightMap->update(x, y, z);
  }

  void addTileBlock(int x, int y, int z, std::shared_ptr<mcfile::je::Block const> const &block) { fTileBlocks.insert(make_pair(Pos3i(x, y, z), block)); }

  void addLiquidTick(int order, CompoundTagPtr const &pendingTick) {
    fLiquidTicks.insert(std::make_pair(order, pendingTick));
  }

  void addTileTick(int order, CompoundTagPtr const &pendingTick) {
    fTileTicks.insert(std::make_pair(order, pendingTick));
  }

private:
  void buildTileEntities(mcfile::je::Chunk const &chunk, Context const &ctx) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::je;

    vector<int32_t> mapIdList;

    for (auto it : chunk.fTileEntities) {
      Pos3i pos = it.first;
      auto const &e = it.second;

      auto found = fTileBlocks.find(pos);
      if (found == fTileBlocks.end()) {
        auto sa = TileEntity::StandaloneTileEntityData(e);
        if (!sa) {
          continue;
        }

        fTileEntities.push_back(sa);
      } else {
        shared_ptr<Block const> block = found->second;
        fTileBlocks.erase(found);

        auto tag = TileEntity::FromBlockAndTileEntity(pos, *block, e, ctx);
        if (!tag) {
          continue;
        }

        fTileEntities.push_back(tag);
      }
    }

    for (auto const &it : fTileBlocks) {
      Pos3i const &pos = it.first;
      Block const &block = *it.second;
      auto tag = TileEntity::FromBlock(pos, block, ctx);
      if (!tag) {
        continue;
      }
      fTileEntities.push_back(tag);
    }

    for (auto e : chunk.fEntities) {
      if (!Entity::IsTileEntity(*e)) {
        continue;
      }
      auto tag = Entity::ToTileEntityData(*e, ctx);
      if (!tag) {
        continue;
      }
      fTileEntities.push_back(tag);
    }
  }

  void buildEntities(mcfile::je::Chunk const &chunk, Context const &ctx, std::unordered_map<Pos2i, std::vector<CompoundTagPtr>, Pos2iHasher> &entities) {
    using namespace std;
    Pos2i chunkPos(chunk.fChunkX, chunk.fChunkZ);
    for (auto e : chunk.fEntities) {
      auto c = e->asCompound();
      if (!c) {
        continue;
      }
      auto result = Entity::From(*c, ctx);
      if (!result.fEntity) {
        continue;
      }
      entities[chunkPos].push_back(result.fEntity);
      copy(result.fPassengers.begin(), result.fPassengers.end(), back_inserter(entities[chunkPos]));
      for (auto const &it : result.fLeashKnots) {
        copy(it.second.begin(), it.second.end(), back_inserter(entities[it.first]));
      }
    }
  }

  void buildData2D(mcfile::je::Chunk const &chunk, mcfile::Dimension dim) {
    switch (fMode) {
    case ChunkConversionMode::Legacy:
      buildData2DLegacy(chunk, dim);
      break;
    case ChunkConversionMode::CavesAndCliffs2:
    default:
      buildData2DCavesAndCliffs2(chunk, dim);
      break;
    }
  }

  void buildData2DCavesAndCliffs2(mcfile::je::Chunk const &chunk, mcfile::Dimension dim) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::be;
    using namespace mcfile::biomes;

    int minChunkY = 0;
    if (dim == mcfile::Dimension::Overworld) {
      minChunkY = -4;
    }
    int maxChunkY = minChunkY;
    for (auto const &section : chunk.fSections) {
      if (!section) {
        continue;
      }
      maxChunkY = (std::max)(maxChunkY, section->y());
    }

    fHeightMap->offset(minChunkY);

    int const x0 = chunk.minBlockX();
    int const y0 = chunk.minBlockY();
    int const z0 = chunk.minBlockZ();
    fBiomeMap = make_shared<BiomeMap>(minChunkY, maxChunkY);
    if (minChunkY < chunk.chunkY()) {
      // Extend BiomeMap below chunk.chunkY()

      // Lookup most used biome in lowest chunk section
      unordered_map<BiomeId, int> used;
      for (int ly = 0; ly < 16; ly += 4) {
        for (int lz = 0; lz < 16; lz += 4) {
          for (int lx = 0; lx < 16; lx += 4) {
            auto biome = chunk.biomeAt(x0 + lx, y0 + ly, z0 + lz);
            used[biome] += 1;
          }
        }
      }
      vector<pair<BiomeId, int>> sorted;
      copy(used.begin(), used.end(), back_inserter(sorted));
      stable_sort(sorted.begin(), sorted.end(), [](auto lhs, auto rhs) {
        return lhs.second > rhs.second;
      });

      // Copy biome to below y < chunk.chunkY()
      BiomeId biome;
      if (sorted.empty()) {
        if (dim == Dimension::Nether) {
          biome = minecraft::nether_wastes;
        } else if (dim == Dimension::End) {
          biome = minecraft::the_end;
        } else {
          biome = minecraft::plains;
        }
      } else {
        biome = sorted.front().first;
      }
      for (int cy = minChunkY; cy < chunk.chunkY(); cy++) {
        for (int ly = 0; ly < 16; ly++) {
          int by = ly + cy * 16;
          for (int lz = 0; lz < 16; lz++) {
            for (int lx = 0; lx < 16; lx++) {
              fBiomeMap->set(lx, by, lz, biome);
            }
          }
        }
      }
    }
    for (int cy = chunk.chunkY(); cy <= maxChunkY; cy++) {
      for (int ly = 0; ly < 16; ly++) {
        int by = ly + cy * 16;
        for (int lz = 0; lz < 16; lz++) {
          int bz = lz + z0;
          for (int lx = 0; lx < 16; lx++) {
            int bx = lx + x0;
            auto biome = chunk.biomeAt(bx, by, bz);
            fBiomeMap->set(lx, by, lz, biome);
          }
        }
      }
    }
  }

  void buildData2DLegacy(mcfile::je::Chunk const &chunk, mcfile::Dimension dim) {
    int const y = 0;
    int const x0 = chunk.minBlockX();
    int const z0 = chunk.minBlockZ();
    BiomeMapLegacy m;
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        auto biome = chunk.biomeAt(x + x0, z + z0);
        m.set(x, z, biome);
      }
    }
    fBiomeMapLegacy = m;
  }

  [[nodiscard]] bool serializeData2D(ChunkData &cd) {
    switch (fMode) {
    case ChunkConversionMode::Legacy:
      return serializeData2DLegacy(cd);
    case ChunkConversionMode::CavesAndCliffs2:
    default:
      return serializeData2DCavesAndCliffs2(cd);
    }
  }

  [[nodiscard]] bool serializeData2DCavesAndCliffs2(ChunkData &cd) {
    using namespace std;
    using namespace mcfile::stream;
    if (!fBiomeMap) {
      //NOTE: BE client is permissive for the lack of Data2D
      return true;
    }
    auto s = make_shared<ByteStream>();
    OutputStreamWriter w(s, std::endian::little);
    if (!fHeightMap->write(w)) {
      return false;
    }
    auto encoded = fBiomeMap->encode();
    if (!encoded) {
      return false;
    }
    if (!w.write(encoded->data(), encoded->size())) {
      return false;
    }
    string trailing(16, 0xff);
    if (!w.write(trailing.data(), trailing.size())) {
      return false;
    }
    s->drain(cd.fData2D);
    return true;
  }

  [[nodiscard]] bool serializeData2DLegacy(ChunkData &cd) {
    using namespace std;
    using namespace mcfile::stream;
    if (!fBiomeMapLegacy) {
      return true;
    }
    auto s = make_shared<ByteStream>();
    OutputStreamWriter w(s, std::endian::little);
    if (!fHeightMap->write(w)) {
      return false;
    }
    if (!fBiomeMapLegacy->write(w)) {
      return false;
    }
    s->drain(cd.fData2D);
    return true;
  }

  [[nodiscard]] bool serializeBlockEntity(ChunkData &cd) {
    using namespace std;
    using namespace mcfile::stream;

    if (fTileEntities.empty()) {
      return true;
    }
    auto s = make_shared<ByteStream>();
    OutputStreamWriter w(s, std::endian::little);
    for (auto const &tag : fTileEntities) {
      if (!tag->writeAsRoot(w)) {
        return false;
      }
    }
    s->drain(cd.fBlockEntity);
    return true;
  }

  [[nodiscard]] bool serializePendingTicks(ChunkData &cd) {
    if (fLiquidTicks.empty() && fTileTicks.empty()) {
      return true;
    }
    using namespace std;
    using namespace mcfile::stream;

    auto tickList = List<Tag::Type::Compound>();
    for (auto it : fLiquidTicks) {
      tickList->push_back(it.second);
    }
    for (auto it : fTileTicks) {
      tickList->push_back(it.second);
    }

    auto pendingTicks = Compound();
    pendingTicks->set("currentTick", Int(fChunkLastUpdate));
    pendingTicks->set("tickList", tickList);

    auto s = make_shared<mcfile::stream::ByteStream>();
    mcfile::stream::OutputStreamWriter w(s, std::endian::little);
    if (!pendingTicks->writeAsRoot(w)) {
      return false;
    }
    s->drain(cd.fPendingTicks);
    return true;
  }

private:
  ChunkConversionMode fMode;
  std::shared_ptr<HeightMap> fHeightMap;
  std::shared_ptr<mcfile::be::BiomeMap> fBiomeMap;
  std::optional<BiomeMapLegacy> fBiomeMapLegacy;
  std::unordered_map<Pos3i, std::shared_ptr<mcfile::je::Block const>, Pos3iHasher> fTileBlocks;
  std::vector<CompoundTagPtr> fTileEntities;
  std::optional<int32_t> fFinalizedState;
  std::map<int, CompoundTagPtr> fLiquidTicks;
  std::map<int, CompoundTagPtr> fTileTicks;
  int64_t fChunkLastUpdate;
};

} // namespace je2be::tobe
