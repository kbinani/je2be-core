#pragma once

namespace je2be::toje {

class BlockPropertyAccessor {
  enum Properties : uint8_t {
    STAIRS = 1,
    KELP = 2,
    TWISTING_VINES = 3,
    WEEPING_VINES = 4,
    PUMPKIN_STEM = 5,
    CAVE_VINES = 6,
    SNOWY = 7,
    CHORUS_PLANT = 8,
    FENCE = 9,
    GLASS_PANE_OR_IRON_BARS = 10,
    CAMPFIRE = 11,
    NOTE_BLOCK = 12,
  };

public:
  static uint8_t BlockProperties(mcfile::be::Block const &b) {
    if (IsStairs(b)) {
      return STAIRS;
    } else if (IsKelp(b)) {
      return KELP;
    } else if (IsTwistingVines(b)) {
      return TWISTING_VINES;
    } else if (IsWeepingVines(b)) {
      return WEEPING_VINES;
    } else if (IsPumpkinStem(b)) {
      return PUMPKIN_STEM;
    } else if (IsCaveVines(b)) {
      return CAVE_VINES;
    } else if (IsSnowy(b)) {
      return SNOWY;
    } else if (IsChorusPlant(b)) {
      return CHORUS_PLANT;
    } else if (IsFence(b)) {
      return FENCE;
    } else if (IsGlassPaneOrIronBars(b)) {
      return GLASS_PANE_OR_IRON_BARS;
    } else if (IsCampfire(b)) {
      return CAMPFIRE;
    } else if (IsNoteBlock(b)) {
      return NOTE_BLOCK;
    }
    return 0;
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

  static bool IsWeepingVines(uint8_t p) {
    return p == WEEPING_VINES;
  }

  static bool IsPumpkinStem(uint8_t p) {
    return p == PUMPKIN_STEM;
  }

  static bool IsCaveVines(uint8_t p) {
    return p == CAVE_VINES;
  }

  static bool IsSnowy(uint8_t p) {
    return p == SNOWY;
  }

  static bool IsChorusPlant(uint8_t p) {
    return p == CHORUS_PLANT;
  }

  static bool IsFence(uint8_t p) {
    return p == FENCE;
  }

  static bool IsGlassPaneOrIronBars(uint8_t p) {
    return p == GLASS_PANE_OR_IRON_BARS;
  }

  static bool IsCampfire(uint8_t p) {
    return p == CAMPFIRE;
  }

  static bool IsNoteBlock(uint8_t p) {
    return p == NOTE_BLOCK;
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

  static bool IsWeepingVines(mcfile::be::Block const &b) {
    return b.fName == "minecraft:weeping_vines";
  }

  static bool IsPumpkinStem(mcfile::be::Block const &b) {
    return b.fName == "minecraft:pumpkin_stem";
  }

  static bool IsCaveVines(mcfile::be::Block const &b) {
    return b.fName == "minecraft:cave_vines" ||
           b.fName == "minecraft:cave_vines_head_with_berries" ||
           b.fName == "minecraft:cave_vines_body_with_berries";
  }

  static bool IsSnowy(mcfile::be::Block const &b) {
    return b.fName == "minecraft:grass" ||
           b.fName == "minecraft:podzol" ||
           b.fName == "minecraft:mycelium";
  }

  static bool IsChorusPlant(mcfile::be::Block const &b) {
    return b.fName == "minecraft:chorus_plant" ||
           b.fName == "minecraft:chorus_flower";
  }

  static bool IsFence(mcfile::be::Block const &b) {
    return b.fName.ends_with("fence");
  }

  static bool IsGlassPaneOrIronBars(mcfile::be::Block const &b) {
    return b.fName.ends_with("glass_pane") ||
           b.fName == "minecraft:iron_bars";
  }

  static bool IsCampfire(mcfile::be::Block const &b) {
    return b.fName.ends_with("campfire");
  }

  static bool IsNoteBlock(mcfile::be::Block const &b) {
    return b.fName == "minecraft:noteblock";
  }

  explicit BlockPropertyAccessor(mcfile::be::Chunk const &chunk) : fChunkX(chunk.fChunkX), fChunkZ(chunk.fChunkZ), fChunk(chunk) {
    using namespace std;
    fSections.resize(mcfile::be::Chunk::kNumSubChunks);
    for (int i = 0; i < mcfile::be::Chunk::kNumSubChunks; i++) {
      auto const &section = chunk.fSubChunks[i];
      if (!section) {
        continue;
      }
      fSections[i].resize(section->fPalette.size());
      for (int j = 0; j < section->fPalette.size(); j++) {
        shared_ptr<mcfile::be::Block const> const &blockB = section->fPalette[j];
        auto p = BlockProperties(*blockB);
        fHasStairs |= IsStairs(p);
        fHasKelp |= IsKelp(p);
        fHasTwistingVines |= IsTwistingVines(p);
        fHasWeepingVines |= IsWeepingVines(p);
        fHasPumpkinStem |= IsPumpkinStem(p);
        fHasCaveVines |= IsCaveVines(p);
        fHasSnowy |= IsSnowy(p);
        fHasChorusPlant |= IsChorusPlant(p);
        fHasFence |= IsFence(p);
        fHasGlassPaneOrIronBars |= IsGlassPaneOrIronBars(p);
        fHasCampfire |= IsCampfire(p);
        fHasNoteBlock |= IsNoteBlock(p);
        fSections[i][j] = p;
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

public:
  bool fHasStairs = false;
  bool fHasKelp = false;
  bool fHasTwistingVines = false;
  bool fHasWeepingVines = false;
  bool fHasPumpkinStem = false;
  bool fHasCaveVines = false;
  bool fHasSnowy = false;
  bool fHasChorusPlant = false;
  bool fHasFence = false;
  bool fHasGlassPaneOrIronBars = false;
  bool fHasCampfire = false;
  bool fHasNoteBlock = false;

private:
  std::vector<std::vector<uint8_t>> fSections;
  int const fChunkX;
  int const fChunkZ;
  mcfile::be::Chunk const &fChunk;
};

} // namespace je2be::toje
