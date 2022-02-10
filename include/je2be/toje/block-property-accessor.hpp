#pragma once

namespace je2be::toje {

class BlockPropertyAccessor {
public:
  using DataType = uint8_t;

private:
  enum Properties : DataType {
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
    REDSTONE_WIRE = 13,
    TRIPWIRE = 14,
    PISTON = 16,
    BEACON = 17,
  };

public:
  static DataType BlockProperties(mcfile::be::Block const &b) {
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
    } else if (IsRedstoneWire(b)) {
      return REDSTONE_WIRE;
    } else if (IsTripwire(b)) {
      return TRIPWIRE;
    } else if (IsPiston(b)) {
      return PISTON;
    } else if (IsBeacon(b)) {
      return BEACON;
    }
    return 0;
  }

  static bool IsStairs(DataType p) {
    return p == STAIRS;
  }

  static bool IsKelp(DataType p) {
    return p == KELP;
  }

  static bool IsTwistingVines(DataType p) {
    return p == TWISTING_VINES;
  }

  static bool IsWeepingVines(DataType p) {
    return p == WEEPING_VINES;
  }

  static bool IsPumpkinStem(DataType p) {
    return p == PUMPKIN_STEM;
  }

  static bool IsCaveVines(DataType p) {
    return p == CAVE_VINES;
  }

  static bool IsSnowy(DataType p) {
    return p == SNOWY;
  }

  static bool IsChorusPlant(DataType p) {
    return p == CHORUS_PLANT;
  }

  static bool IsFence(DataType p) {
    return p == FENCE;
  }

  static bool IsGlassPaneOrIronBars(DataType p) {
    return p == GLASS_PANE_OR_IRON_BARS;
  }

  static bool IsCampfire(DataType p) {
    return p == CAMPFIRE;
  }

  static bool IsNoteBlock(DataType p) {
    return p == NOTE_BLOCK;
  }

  static bool IsRedstoneWire(DataType p) {
    return p == REDSTONE_WIRE;
  }

  static bool IsTripwire(DataType p) {
    return p == TRIPWIRE;
  }

  static bool IsPiston(DataType p) {
    return p == PISTON;
  }

  static bool IsBeacon(DataType p) {
    return p == BEACON;
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

  static bool IsRedstoneWire(mcfile::be::Block const &b) {
    return b.fName == "minecraft:redstone_wire";
  }

  static bool IsTripwire(mcfile::be::Block const &b) {
    return b.fName == "minecraft:tripWire";
  }

  static bool IsPiston(mcfile::be::Block const &b) {
    return b.fName == "minecraft:piston" ||
           b.fName == "minecraft:sticky_piston" ||
           b.fName == "minecraft:pistonArmCollision" ||
           b.fName == "minecraft:stickyPistonArmCollision" ||
           b.fName == "minecraft:movingBlock";
  }

  static bool IsBeacon(mcfile::be::Block const &b) {
    return b.fName == "minecraft:beacon";
  }

  explicit BlockPropertyAccessor(mcfile::be::Chunk const &chunk) : fChunkX(chunk.fChunkX), fChunkY(chunk.fChunkY), fChunkZ(chunk.fChunkZ), fChunk(chunk) {
    using namespace std;
    fSections.resize(chunk.fSubChunks.size());
    for (int i = 0; i < chunk.fSubChunks.size(); i++) {
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
        fHasRedstoneWire |= IsRedstoneWire(p);
        fHasTripwire |= IsTripwire(p);
        fHasPiston |= IsPiston(p);
        fHasBeacon |= IsBeacon(p);
        fSections[i][j] = p;
      }
    }
  }

  DataType property(int bx, int by, int bz) const {
    using namespace mcfile;
    using namespace mcfile::be;
    int cx = Coordinate::ChunkFromBlock(bx);
    int cy = Coordinate::ChunkFromBlock(by);
    int cz = Coordinate::ChunkFromBlock(bz);
    int sectionIndex = cy - fChunkY;
    if (sectionIndex < 0 || fChunk.fSubChunks.size() <= sectionIndex) {
      return 0;
    }
    auto const &section = fChunk.fSubChunks[sectionIndex];
    if (!section) {
      return 0;
    }
    int lx = bx - cx * 16;
    int ly = by - cy * 16;
    int lz = bz - cz * 16;
    int index = mcfile::be::SubChunk::BlockIndex(lx, ly, lz);
    auto i = section->fPaletteIndices[index];
    return fSections[sectionIndex][i];
  }

  int minBlockY() const {
    return fChunk.minBlockY();
  }

  int maxBlockY() const {
    return fChunk.maxBlockY();
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
  bool fHasRedstoneWire = false;
  bool fHasTripwire = false;
  bool fHasPiston = false;
  bool fHasBeacon = false;

private:
  std::vector<std::vector<DataType>> fSections;
  int const fChunkX;
  int const fChunkY;
  int const fChunkZ;
  mcfile::be::Chunk const &fChunk;
};

} // namespace je2be::toje
