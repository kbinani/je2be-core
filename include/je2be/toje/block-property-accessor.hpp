#pragma once

namespace je2be::toje {

class BlockPropertyAccessor {
  enum Properties : uint8_t {
    STAIRS = 1,
    KELP = 2,
    TWISTING_VINES = 3,
  };

public:
  static uint8_t BlockProperties(mcfile::be::Block const &b) {
    uint8_t p = 0;
    if (IsStairs(b)) {
      p = STAIRS;
    } else if (IsKelp(b)) {
      p = KELP;
    } else if (IsTwistingVines(b)) {
      p = TWISTING_VINES;
    }
    return p;
  }

  static bool IsStairs(uint8_t p) {
    return p == STAIRS;
  }

  static bool IsKelp(uint8_t p) {
    return p == KELP;
  }

  static bool IsTwistingVines(uint8_t p) {
    return p == TWISTING_VINES;
  }

  static bool IsStairs(mcfile::be::Block const &b) {
    return b.fName.ends_with("_stairs");
  }

  static bool IsKelp(mcfile::be::Block const &b) {
    return b.fName == "minecraft:kelp";
  }

  static bool IsTwistingVines(mcfile::be::Block const &b) {
    return b.fName == "minecraft:twisting_vines";
  }

  explicit BlockPropertyAccessor(mcfile::be::Chunk const &chunk) : fChunkX(chunk.fChunkX), fChunkZ(chunk.fChunkZ), fChunk(chunk) {
    using namespace std;
    using namespace mcfile::be;
    fSections.resize(Chunk::kNumSubChunks);
    for (int i = 0; i < Chunk::kNumSubChunks; i++) {
      auto const &section = chunk.fSubChunks[i];
      if (!section) {
        continue;
      }
      fSections[i].resize(section->fPalette.size());
      for (int j = 0; j < section->fPalette.size(); j++) {
        shared_ptr<Block const> const &blockB = section->fPalette[j];
        fSections[i][j] = BlockProperties(*blockB);
      }
    }
  }

  uint8_t property(int bx, int by, int bz) const {
    using namespace mcfile;
    using namespace mcfile::be;
    int cx = Coordinate::ChunkFromBlock(bx);
    int cy = Coordinate::ChunkFromBlock(by);
    int cz = Coordinate::ChunkFromBlock(bz);
    int sectionIndex = cy - Chunk::kMinChunkY;
    auto const &section = fChunk.fSubChunks[sectionIndex];
    if (!section) {
      return 0;
    }
    int lx = bx - cx * 16;
    int ly = by - cy * 16;
    int lz = bz - cz * 16;
    int index = SubChunk::BlockIndex(lx, ly, lz);
    auto i = section->fPaletteIndices[index];
    return fSections[sectionIndex][i];
  }

private:
  std::vector<std::vector<uint8_t>> fSections;
  int const fChunkX;
  int const fChunkZ;
  mcfile::be::Chunk const &fChunk;
};

} // namespace je2be::toje
