#pragma once

namespace je2be::tobe {

class ChunkDataPackage {
public:
  explicit ChunkDataPackage(int minChunkY) {
    fHeightMap = std::make_shared<HeightMap>(minChunkY);
  }

  void build(mcfile::je::Chunk const &chunk, JavaEditionMap const &mapInfo, DimensionDataFragment &ddf) {
    buildEntities(chunk, mapInfo, ddf);
    buildBiomeMap(chunk);
    buildTileEntities(chunk, mapInfo, ddf);
    if (chunk.status() == mcfile::je::Chunk::Status::FULL) {
      fFinalizedState = 2;
    }
    ddf.addStatChunkVersion(chunk.fDataVersion);
    ddf.addStat(1, fTileEntities.size(), fEntities.size());
    fChunkLastUpdate = chunk.fLastUpdate;
  }

  [[nodiscard]] bool serialize(ChunkData &cd) {
    if (!serializeData2D(cd)) {
      return false;
    }
    if (!serializeBlockEntity(cd)) {
      return false;
    }
    if (!serializeEntity(cd)) {
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

  void addPendingTick(int order, std::shared_ptr<mcfile::nbt::CompoundTag> const &pendingTick) {
    fPendingTicks.insert(std::make_pair(order, pendingTick));
  }

private:
  void buildTileEntities(mcfile::je::Chunk const &chunk, JavaEditionMap const &mapInfo, DimensionDataFragment &ddf) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::je;
    using namespace mcfile::nbt;
    using namespace props;

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

        auto tag = TileEntity::FromBlockAndTileEntity(pos, *block, e, mapInfo, ddf);
        if (!tag) {
          continue;
        }

        fTileEntities.push_back(tag);
      }
    }

    for (auto const &it : fTileBlocks) {
      Pos3i const &pos = it.first;
      Block const &block = *it.second;
      auto tag = TileEntity::FromBlock(pos, block, mapInfo, ddf);
      if (!tag) {
        continue;
      }
      fTileEntities.push_back(tag);
    }

    for (auto e : chunk.fEntities) {
      if (!Entity::IsTileEntity(*e)) {
        continue;
      }
      auto tag = Entity::ToTileEntityData(*e, mapInfo, ddf);
      if (!tag) {
        continue;
      }
      fTileEntities.push_back(tag);
    }
  }

  void buildEntities(mcfile::je::Chunk const &chunk, JavaEditionMap const &mapInfo, DimensionDataFragment &ddf) {
    using namespace std;
    using namespace props;
    for (auto e : chunk.fEntities) {
      auto c = e->asCompound();
      if (!c) {
        continue;
      }
      auto converted = Entity::From(*c, mapInfo, ddf);
      copy_n(converted.begin(), converted.size(), back_inserter(fEntities));
    }
  }

  void buildBiomeMap(mcfile::je::Chunk const &chunk) {
    int const x0 = chunk.minBlockX();
    int const z0 = chunk.minBlockZ();
    int minChunkY = chunk.chunkY();
    int maxChunkY = minChunkY + chunk.fSections.size() - 1;
    fBiomeMap = std::make_shared<mcfile::be::BiomeMap>(minChunkY, maxChunkY);
    for (int cy = minChunkY; cy <= maxChunkY; cy++) {
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

  [[nodiscard]] bool serializeData2D(ChunkData &cd) {
    using namespace std;
    using namespace mcfile::stream;
    if (!fBiomeMap) {
      //NOTE: BE client is permissive for the lack of Data2D
      return true;
    }
    auto s = make_shared<ByteStream>();
    OutputStreamWriter w(s, {.fLittleEndian = true});
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

  [[nodiscard]] bool serializeBlockEntity(ChunkData &cd) {
    using namespace std;
    using namespace mcfile::stream;
    using namespace mcfile::nbt;

    if (fTileEntities.empty()) {
      return true;
    }
    auto s = make_shared<ByteStream>();
    OutputStreamWriter w(s, {.fLittleEndian = true});
    for (auto const &tag : fTileEntities) {
      if (!tag->writeAsRoot(w)) {
        return false;
      }
    }
    s->drain(cd.fBlockEntity);
    return true;
  }

  [[nodiscard]] bool serializeEntity(ChunkData &cd) {
    using namespace std;
    using namespace mcfile::stream;
    using namespace mcfile::nbt;

    if (fEntities.empty()) {
      return true;
    }
    auto s = make_shared<ByteStream>();
    OutputStreamWriter w(s, {.fLittleEndian = true});
    for (auto const &tag : fEntities) {
      if (!tag->writeAsRoot(w)) {
        return false;
      }
    }
    s->drain(cd.fEntity);
    return true;
  }

  [[nodiscard]] bool serializePendingTicks(ChunkData &cd) {
    if (fPendingTicks.empty()) {
      return true;
    }
    using namespace std;
    using namespace mcfile::nbt;
    using namespace mcfile::stream;
    using namespace props;

    auto tickList = make_shared<ListTag>(Tag::Type::Compound);
    for (auto it : fPendingTicks) {
      tickList->push_back(it.second);
    }

    auto pendingTicks = make_shared<CompoundTag>();
    pendingTicks->set("currentTick", Int(fChunkLastUpdate));
    pendingTicks->set("tickList", tickList);

    auto s = make_shared<mcfile::stream::ByteStream>();
    mcfile::stream::OutputStreamWriter w(s, {.fLittleEndian = true});
    if (!pendingTicks->writeAsRoot(w)) {
      return false;
    }
    s->drain(cd.fPendingTicks);
    return true;
  }

private:
  std::shared_ptr<HeightMap> fHeightMap;
  std::shared_ptr<mcfile::be::BiomeMap> fBiomeMap;
  std::unordered_map<Pos3i, std::shared_ptr<mcfile::je::Block const>, Pos3iHasher> fTileBlocks;
  std::vector<std::shared_ptr<mcfile::nbt::CompoundTag>> fTileEntities;
  std::vector<std::shared_ptr<mcfile::nbt::CompoundTag>> fEntities;
  std::optional<int32_t> fFinalizedState;
  std::map<int, std::shared_ptr<mcfile::nbt::CompoundTag>> fPendingTicks;
  int64_t fChunkLastUpdate;
};

} // namespace je2be::tobe
