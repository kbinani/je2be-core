#pragma once

namespace je2be::tobe {

class Statistics {
public:
  void merge(Statistics const &o) {
    for (auto const &e : o.fErrors) {
      fErrors.push_back(e);
    }
  }

  struct ChunkError {
    mcfile::Dimension fDim;
    int32_t fChunkX;
    int32_t fChunkZ;
    Status::Where fWhere;

    ChunkError(mcfile::Dimension dim, int32_t cx, int32_t cz, Status::Where where) : fDim(dim), fChunkX(cx), fChunkZ(cz), fWhere(where) {}
  };

  void addChunkError(mcfile::Dimension dim, int32_t chunkX, int32_t chunkZ, Status::Where where) {
    ChunkError e(dim, chunkX, chunkZ, where);
    fErrors.push_back(e);
  }

public:
  std::vector<ChunkError> fErrors;
};

} // namespace je2be::tobe
