#pragma once

namespace je2be::tobe {

class Statistics {
public:
  void merge(Statistics const &o) {
    for (auto const &it : o.fChunkDataVersions) {
      fChunkDataVersions[it.first] += it.second;
    }
    fNumChunks += o.fNumChunks;
    fNumBlockEntities += o.fNumBlockEntities;
    fNumEntities += o.fNumEntities;
    for (auto const &e : o.fErrors) {
      fErrors.push_back(e);
    }
  }

  void addChunkVersion(uint32_t chunkVersion) { fChunkDataVersions[chunkVersion] += 1; }

  void add(uint64_t numChunks, uint64_t numBlockEntities, uint64_t numEntities) {
    fNumChunks += numChunks;
    fNumBlockEntities += numBlockEntities;
    fNumEntities = numEntities;
  }

  struct Error {
    mcfile::Dimension fDim;
    int32_t fChunkX;
    int32_t fChunkZ;
  };

  void addError(mcfile::Dimension dim, int32_t chunkX, int32_t chunkZ) {
    Error e;
    e.fDim = dim;
    e.fChunkX = chunkX;
    e.fChunkZ = chunkZ;
    fErrors.push_back(e);
  }

public:
  std::unordered_map<uint32_t, uint64_t> fChunkDataVersions;
  uint64_t fNumChunks = 0;
  uint64_t fNumBlockEntities = 0;
  uint64_t fNumEntities = 0;
  std::vector<Error> fErrors;
};

} // namespace je2be::tobe