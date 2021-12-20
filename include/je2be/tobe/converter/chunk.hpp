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

public:
  static Result Convert(mcfile::Dimension dim, DbInterface &db, mcfile::je::Region region, int cx, int cz, JavaEditionMap mapInfo) {
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
      if (chunk->fDataVersion >= 2724) {
        vector<shared_ptr<nbt::CompoundTag>> entities;
        if (region.entitiesAt(cx, cz, entities)) {
          chunk->fEntities.swap(entities);
        }
      }
      r.fData = MakeWorldData(chunk, region, dim, db, mapInfo);
      return r;
    } catch (...) {
      Chunk::Result r;
      r.fData = make_shared<WorldData>(dim);
      r.fData->addStatError(dim, cx, cz);
      r.fOk = false;
      return r;
    }
  }

  static std::shared_ptr<WorldData> MakeWorldData(std::shared_ptr<mcfile::je::Chunk> const &chunk, mcfile::je::Region region, mcfile::Dimension dim, DbInterface &db, JavaEditionMap const &mapInfo) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;
    using namespace mcfile::nbt;

    if (!chunk) {
      return nullptr;
    }
    PreprocessChunk(chunk, region);

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

    ret->addStructures(*chunk);
    ret->updateChunkLastUpdate(*chunk);

    cdp.build(*chunk, mapInfo, *ret);
    if (!cdp.serialize(cd)) {
      return nullptr;
    }

    if (!cd.put(db)) {
      return nullptr;
    }

    return ret;
  }

private:
  static void PreprocessChunk(std::shared_ptr<mcfile::je::Chunk> const &chunk, mcfile::je::Region const &region) {
    MovingPiston::PreprocessChunk(chunk, region);
    InjectTickingLiquidBlocksAsBlocks(*chunk);
    SortTickingBlocks(chunk->fTileTicks);
    SortTickingBlocks(chunk->fLiquidTicks);
  }

  static void SortTickingBlocks(std::vector<mcfile::je::TickingBlock> &blocks) {
    std::stable_sort(blocks.begin(), blocks.end(), [](auto a, auto b) {
      return a.fP < b.fP;
    });
  }

  static void InjectTickingLiquidBlocksAsBlocks(mcfile::je::Chunk &chunk) {
    for (mcfile::je::TickingBlock const &tb : chunk.fLiquidTicks) {
      auto block = std::make_shared<mcfile::je::Block>(tb.fI);
      chunk.setBlockAt(tb.fX, tb.fY, tb.fZ, block);
    }
  }

  static ChunkConversionMode ConversionMode(mcfile::je::Chunk const &chunk) {
    if (chunk.minBlockY() < 0 || 256 < chunk.maxBlockY()) {
      return ChunkConversionMode::CavesAndCliffs2;
    } else {
      return ChunkConversionMode::Legacy;
    }
  }
};

} // namespace je2be::tobe
