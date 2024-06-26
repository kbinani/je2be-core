#pragma once

#include "structure/_structure-piece.hpp"

namespace je2be::java {

class StructurePieceCollection {
public:
  void add(StructurePiece const &p) { fPieces.push_back(p); }

  decltype(auto) begin() const { return fPieces.begin(); }

  decltype(auto) end() const { return fPieces.end(); }

  [[nodiscard]] Status put(DbInterface &db, mcfile::Dimension dim) {
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
      OutputStreamWriter w(s, mcfile::Encoding::LittleEndian);
      if (!w.write((u32)it.second.size())) {
        return JE2BE_ERROR;
      }
      for (auto const &piece : it.second) {
        if (!w.write(piece.fVolume.fStart.fX)) {
          return JE2BE_ERROR;
        }
        if (!w.write(piece.fVolume.fStart.fY)) {
          return JE2BE_ERROR;
        }
        if (!w.write(piece.fVolume.fStart.fZ)) {
          return JE2BE_ERROR;
        }
        if (!w.write(piece.fVolume.fEnd.fX)) {
          return JE2BE_ERROR;
        }
        if (!w.write(piece.fVolume.fEnd.fY)) {
          return JE2BE_ERROR;
        }
        if (!w.write(piece.fVolume.fEnd.fZ)) {
          return JE2BE_ERROR;
        }
        if (!w.write((u8)piece.fType)) {
          return JE2BE_ERROR;
        }
      }
      vector<u8> buffer;
      s->drain(buffer);

      auto key = mcfile::be::DbKey::StructureBounds(it.first.fX, it.first.fZ, dim);
      leveldb::Slice value((char const *)buffer.data(), buffer.size());
      if (auto st = db.put(key, value); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
    }

    return Status::Ok();
  }

private:
  std::vector<StructurePiece> fPieces;
};

} // namespace je2be::java
