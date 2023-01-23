#pragma once

#include <minecraft-file.hpp>

#include "structure/_structure-piece.hpp"

namespace je2be::toje {

class StructureInfo {
public:
  struct Structure {
    StructureType fType;
    Volume fBounds;
    Pos2i fStartChunk;

    Structure(StructureType type, Volume bounds, Pos2i startChunk) : fType(type), fBounds(bounds), fStartChunk(startChunk) {}
  };

  std::unordered_map<mcfile::Dimension, std::vector<Structure>> fStructures;

public:
  void add(mcfile::Dimension d, Structure s) {
    fStructures[d].push_back(s);
  }

  void structures(mcfile::Dimension d, Pos2i chunk, std::vector<Structure> &out) const {
    out.clear();
    auto found = fStructures.find(d);
    if (found == fStructures.end()) {
      return;
    }
    Volume chunkVolume(Pos3i(chunk.fX * 16, -64, chunk.fZ * 16), Pos3i(chunk.fX * 16 + 15, 319, chunk.fZ * 16 + 15));
    for (Structure const &it : found->second) {
      if (Volume::Intersection(it.fBounds, chunkVolume)) {
        out.push_back(it);
      }
    }
  }

  static i64 PackStructureStartsReference(i32 cx, i32 cz) {
    i64 r;
    *(i32 *)&r = cx;
    *((i32 *)&r + 1) = cz;
    return r;
  }
};

} // namespace je2be::toje
