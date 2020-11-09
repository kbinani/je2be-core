#pragma once

namespace j2b {

class StructurePieceCollection {
public:
  void add(StructurePiece const &p) { fPieces.push_back(p); }

  decltype(auto) begin() const { return fPieces.begin(); }

  decltype(auto) end() const { return fPieces.end(); }

  void put(DbInterface &db, Dimension dim) {
    using namespace std;
    unordered_map<Pos, vector<StructurePiece>, PosHasher> splitted;
    for (auto const &it : fPieces) {
      int minChunkX = mcfile::Coordinate::ChunkFromBlock(it.fVolume.fStart.fX);
      int minChunkZ = mcfile::Coordinate::ChunkFromBlock(it.fVolume.fStart.fZ);
      int maxChunkX = mcfile::Coordinate::ChunkFromBlock(it.fVolume.fEnd.fX);
      int maxChunkZ = mcfile::Coordinate::ChunkFromBlock(it.fVolume.fEnd.fZ);
      for (int cx = minChunkX; cx <= maxChunkX; cx++) {
        for (int cz = minChunkZ; cz < maxChunkZ; cz++) {
          Volume chunkVolume(Pos(cx * 16, 0, cz * 16),
                             Pos(cx * 16 + 15, 255, cz * 16 + 15));
          auto intersection = Volume::Intersection(chunkVolume, it.fVolume);
          if (intersection) {
            Pos idx(cx, 0, cz);
            StructurePiece p(intersection->fStart, intersection->fEnd,
                             it.fType);
            splitted[idx].push_back(p);
          }
        }
      }
    }

    for (auto const &it : splitted) {
      using namespace mcfile::stream;
      auto s = std::make_shared<ByteStream>();
      OutputStreamWriter w(s, {.fLittleEndian = true});
      w.write((uint32_t)it.second.size());
      for (auto const &s : it.second) {
        w.write(s.fVolume.fStart.fX);
        w.write(s.fVolume.fStart.fY);
        w.write(s.fVolume.fStart.fZ);
        w.write(s.fVolume.fEnd.fX);
        w.write(s.fVolume.fEnd.fY);
        w.write(s.fVolume.fEnd.fZ);
        w.write((uint8_t)s.fType);
      }
      vector<uint8_t> buffer;
      s->drain(buffer);

      auto key = Key::StructureBounds(it.first.fX, it.first.fZ, dim);
      leveldb::Slice value((char const *)buffer.data(), buffer.size());
      db.put(key, value);
    }
  }

private:
  std::vector<StructurePiece> fPieces;
};

} // namespace j2b
