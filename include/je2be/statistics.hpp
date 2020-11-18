#pragma once

namespace j2b {

class Statistics {
public:
  void merge(Statistics const &o) {
    for (auto const &it : o.fChunkDataVersions) {
      fChunkDataVersions[it.first] += it.second;
    }
    fNumChunks += o.fNumChunks;
    fNumBlockEntities += o.fNumBlockEntities;
    fNumEntities += o.fNumEntities;
  }

  void addChunkVersion(uint32_t chunkVersion) {
    fChunkDataVersions[chunkVersion] += 1;
  }

  void add(uint64_t numChunks, uint64_t numBlockEntities,
           uint64_t numEntities) {
    fNumChunks += numChunks;
    fNumBlockEntities += numBlockEntities;
    fNumEntities = numEntities;
  }

public:
  std::unordered_map<uint32_t, uint64_t> fChunkDataVersions;
  uint64_t fNumChunks = 0;
  uint64_t fNumBlockEntities = 0;
  uint64_t fNumEntities = 0;
};

} // namespace j2b
