#pragma once

namespace je2be::tobe {

class StructurePieceCollection {
public:
  void add(StructurePiece const &p) { fPieces.push_back(p); }

  decltype(auto) begin() const { return fPieces.begin(); }

  decltype(auto) end() const { return fPieces.end(); }

  [[nodiscard]] bool put(DbInterface &db, mcfile::Dimension dim) {
    using namespace std;
    unordered_map<Pos3i, vector<StructurePiece>, Pos3iHasher> splitted;
    for (auto const &it : fPieces) {
      int minChunkX = mcfile::Coordinate::ChunkFromBlock(it.fVolume.fStart.fX);
      int minChunkZ = mcfile::Coordinate::ChunkFromBlock(it.fVolume.fStart.fZ);
      int maxChunkX = mcfile::Coordinate::ChunkFromBlock(it.fVolume.fEnd.fX);
      int maxChunkZ = mcfile::Coordinate::ChunkFromBlock(it.fVolume.fEnd.fZ);
      for (int cx = minChunkX; cx <= maxChunkX; cx++) {
        for (int cz = minChunkZ; cz <= maxChunkZ; cz++) {
          Volume chunkVolume(Pos3i(cx * 16, -64, cz * 16), Pos3i(cx * 16 + 15, 320, cz * 16 + 15));
          auto intersection = Volume::Intersection(chunkVolume, it.fVolume);
          if (intersection) {
            Pos3i idx(cx, 0, cz);
            StructurePiece p(intersection->fStart, intersection->fEnd, it.fType);
            splitted[idx].push_back(p);
          }
        }
      }
    }

    for (auto const &it : splitted) {
      using namespace mcfile::stream;
      auto s = std::make_shared<ByteStream>();
      OutputStreamWriter w(s, mcfile::Endian::Little);
      if (!w.write((uint32_t)it.second.size())) {
        return false;
      }
      for (auto const &piece : it.second) {
        if (!w.write(piece.fVolume.fStart.fX)) {
          return false;
        }
        if (!w.write(piece.fVolume.fStart.fY)) {
          return false;
        }
        if (!w.write(piece.fVolume.fStart.fZ)) {
          return false;
        }
        if (!w.write(piece.fVolume.fEnd.fX)) {
          return false;
        }
        if (!w.write(piece.fVolume.fEnd.fY)) {
          return false;
        }
        if (!w.write(piece.fVolume.fEnd.fZ)) {
          return false;
        }
        if (!w.write((uint8_t)piece.fType)) {
          return false;
        }
      }
      vector<uint8_t> buffer;
      s->drain(buffer);

      auto key = mcfile::be::DbKey::StructureBounds(it.first.fX, it.first.fZ, dim);
      leveldb::Slice value((char const *)buffer.data(), buffer.size());
      db.put(key, value);
    }

    return true;
  }

  std::shared_ptr<Tag> toNbt() const {
    using namespace std;
    auto ret = List<Tag::Type::Compound>();
    for (auto const &piece : fPieces) {
      ret->push_back(piece.toNbt());
    }
    return ret;
  }

  static std::optional<StructurePieceCollection> FromNbt(Tag const &tag) {
    using namespace std;
    auto list = tag.asList();
    if (!list) {
      return nullopt;
    }
    StructurePieceCollection ret;
    for (auto const &it : *list) {
      auto piece = StructurePiece::FromNbt(*it);
      if (!piece) {
        return nullopt;
      }
      ret.fPieces.push_back(*piece);
    }
    return ret;
  }

private:
  std::vector<StructurePiece> fPieces;
};

} // namespace je2be::tobe
