#include <je2be/toje/context.hpp>

#include <je2be/db/async-iterator.hpp>
#include <je2be/structure/structure-piece.hpp>

namespace je2be::toje {

class Context::Impl {
public:
  static std::shared_ptr<Context> Init(leveldb::DB &db,
                                       Options opt,
                                       mcfile::Endian endian,
                                       std::map<mcfile::Dimension, std::unordered_map<Pos2i, ChunksInRegion, Pos2iHasher>> &regions,
                                       int &totalChunks,
                                       int64_t gameTick,
                                       GameMode gameMode,
                                       unsigned int concurrency,
                                       std::function<std::optional<BlockEntityConvertResult>(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx)> fromBlockAndBlockEntity) {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;
    namespace fs = std::filesystem;

    totalChunks = 0;

    auto mapInfo = make_shared<MapInfo>();
    auto structureInfo = make_shared<StructureInfo>();
    unordered_map<Dimension, vector<StructurePiece>> pieces;

    AsyncIterator::IterateUnordered(db, concurrency, [opt, &regions, &totalChunks, &pieces, endian, &mapInfo](string const &key, string const &value) {
      auto parsed = mcfile::be::DbKey::Parse(key);
      if (!parsed) {
        return;
      }
      if (parsed->fIsTagged) {
        uint8_t tag = parsed->fTagged.fTag;
        Dimension d = parsed->fTagged.fDimension;
        if (!opt.fDimensionFilter.empty()) {
          if (opt.fDimensionFilter.find(d) == opt.fDimensionFilter.end()) {
            return;
          }
        }
        switch (tag) {
        case static_cast<uint8_t>(mcfile::be::DbKey::Tag::Data3D):
        case static_cast<uint8_t>(mcfile::be::DbKey::Tag::Data2D): {
          int cx = parsed->fTagged.fChunk.fX;
          int cz = parsed->fTagged.fChunk.fZ;
          Pos2i c(cx, cz);
          if (!opt.fChunkFilter.empty()) {
            if (opt.fChunkFilter.find(c) == opt.fChunkFilter.end()) {
              return;
            }
          }
          int rx = Coordinate::RegionFromChunk(cx);
          int rz = Coordinate::RegionFromChunk(cz);
          Pos2i r(rx, rz);
          regions[d][r].fChunks.insert(c);
          totalChunks++;
          break;
        }
        case static_cast<uint8_t>(mcfile::be::DbKey::Tag::StructureBounds): {
          vector<StructurePiece> buffer;
          StructurePiece::Parse(value, buffer);
          copy(buffer.begin(), buffer.end(), back_inserter(pieces[d]));
          break;
        }
        }
      } else if (parsed->fUnTagged.starts_with("map_")) {
        int64_t mapId;
        auto parsed = MapInfo::Parse(value, mapId, endian);
        if (!parsed) {
          return;
        }
        mapInfo->add(*parsed, mapId);
      }
    });

    for (auto const &i : pieces) {
      Dimension d = i.first;
      unordered_map<StructureType, vector<StructurePiece>> categorized;
      for (StructurePiece const &piece : i.second) {
        StructureType type = piece.fType;
        switch (type) {
        case StructureType::Monument:
        case StructureType::Fortress:
        case StructureType::Outpost:
          categorized[type].push_back(piece);
          break;
        }
      }
      for (auto const &i : categorized) {
        StructureType type = i.first;
        vector<Volume> volumes;
        for (StructurePiece const &piece : i.second) {
          volumes.push_back(piece.fVolume);
        }
        Volume::Connect(volumes);
        switch (type) {
        case StructureType::Monument:
          Volume::ConnectGreed(volumes, 58, 23, 58);
          break;
        default:
          break;
        }
        for (Volume const &v : volumes) {
          int cx;
          int cz;
          if (type == StructureType::Monument) {
            if (v.size<0>() == 58 && v.size<1>() == 23 && v.size<2>() == 58) {
              int x = v.fStart.fX;
              int z = v.fStart.fZ;
              cx = Coordinate::ChunkFromBlock(x) + 2;
              cz = Coordinate::ChunkFromBlock(z) + 2;
              // NW corner offset from chunk:
              // BE: (11, 11)
              // JE: (3, 3)
            } else {
              // TODO: bounds is incomplete for monument
              continue;
            }
          } else {
            int x = v.fStart.fX + v.size<0>() / 2;
            int z = v.fStart.fZ + v.size<2>() / 2;
            cx = Coordinate::ChunkFromBlock(x);
            cz = Coordinate::ChunkFromBlock(z);
          }
          Pos2i chunk(cx, cz);
          StructureInfo::Structure s(type, v, chunk);
          structureInfo->add(d, s);
        }
      }
    }

    fs::path temp = opt.fTempDirectory ? *opt.fTempDirectory : fs::temp_directory_path();
    return std::shared_ptr<Context>(new Context(endian, temp, mapInfo, structureInfo, gameTick, gameMode, fromBlockAndBlockEntity));
  }
};

std::shared_ptr<Context> Context::Init(leveldb::DB &db,
                                       Options opt,
                                       mcfile::Endian endian,
                                       std::map<mcfile::Dimension, std::unordered_map<Pos2i, ChunksInRegion, Pos2iHasher>> &regions,
                                       int &totalChunks,
                                       int64_t gameTick,
                                       GameMode gameMode,
                                       unsigned int concurrency,
                                       std::function<std::optional<BlockEntityConvertResult>(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx)> fromBlockAndBlockEntity) {
  return Impl::Init(db, opt, endian, regions, totalChunks, gameTick, gameMode, concurrency, fromBlockAndBlockEntity);
}

} // namespace je2be::toje
