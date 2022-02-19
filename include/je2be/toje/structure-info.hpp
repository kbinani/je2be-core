#pragma once

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

  static int64_t PackStructureStartsReference(int32_t cx, int32_t cz) {
    int64_t r;
    *(int32_t *)&r = cx;
    *((int32_t *)&r + 1) = cz;
    return r;
  }
};

} // namespace je2be::toje
