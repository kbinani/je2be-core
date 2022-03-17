#pragma once

namespace je2be::terraform {

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
    MELON_STEM = 18,
    DOOR = 19,
    WALL = 20,
    LEAVES = 21,
  };

public:
  static DataType BlockProperties(mcfile::be::Block const &b) {
    return GetBlockProperties(b);
  }

  static DataType BlockProperties(mcfile::je::Block const &b) {
    return GetBlockProperties(b);
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

  static bool IsMelonStem(DataType p) {
    return p == MELON_STEM;
  }

  static bool IsDoor(DataType p) {
    return p == DOOR;
  }

  static bool IsWall(DataType p) {
    return p == WALL;
  }

  static bool IsLeaves(DataType p) {
    return p == LEAVES;
  }

  static bool IsStairs(mcfile::be::Block const &b) {
    return b.fName.ends_with("_stairs");
  }

  static bool IsStairs(mcfile::je::Block const &b) {
    return b.fName.ends_with("_stairs");
  }

  static bool IsKelp(mcfile::be::Block const &b) {
    return b.fName == "minecraft:kelp";
  }

  static bool IsKelp(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::kelp || b.fId == mcfile::blocks::minecraft::kelp_plant;
  }

  static bool IsTwistingVines(mcfile::be::Block const &b) {
    return b.fName == "minecraft:twisting_vines";
  }

  static bool IsTwistingVines(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::twisting_vines || b.fId == mcfile::blocks::minecraft::twisting_vines_plant;
  }

  static bool IsWeepingVines(mcfile::be::Block const &b) {
    return b.fName == "minecraft:weeping_vines";
  }

  static bool IsWeepingVines(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::weeping_vines || b.fId == mcfile::blocks::minecraft::weeping_vines_plant;
  }

  static bool IsPumpkinStem(mcfile::be::Block const &b) {
    return b.fName == "minecraft:pumpkin_stem";
  }

  static bool IsPumpkinStem(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::pumpkin_stem;
  }

  static bool IsCaveVines(mcfile::be::Block const &b) {
    return b.fName == "minecraft:cave_vines" ||
           b.fName == "minecraft:cave_vines_head_with_berries" ||
           b.fName == "minecraft:cave_vines_body_with_berries";
  }

  static bool IsCaveVines(mcfile::je::Block const &b) {
    using namespace mcfile::blocks;
    return b.fId == minecraft::cave_vines || b.fId == minecraft::cave_vines_plant;
  }

  static bool IsSnowy(mcfile::be::Block const &b) {
    return b.fName == "minecraft:grass" ||
           b.fName == "minecraft:podzol" ||
           b.fName == "minecraft:mycelium";
  }

  static bool IsSnowy(mcfile::je::Block const &b) {
    using namespace mcfile::blocks;
    return b.fId == minecraft::grass_block || b.fId == minecraft::podzol || b.fId == minecraft::mycelium;
  }

  static bool IsChorusPlant(mcfile::be::Block const &b) {
    return b.fName == "minecraft:chorus_plant" ||
           b.fName == "minecraft:chorus_flower";
  }

  static bool IsChorusPlant(mcfile::je::Block const &b) {
    using namespace mcfile::blocks;
    return b.fId == minecraft::chorus_plant || b.fId == minecraft::chorus_flower;
  }

  static bool IsFence(mcfile::be::Block const &b) {
    return b.fName.ends_with("fence");
  }

  static bool IsFence(mcfile::je::Block const &b) {
    return b.fName.ends_with("fence");
  }

  static bool IsGlassPaneOrIronBars(mcfile::be::Block const &b) {
    return b.fName.ends_with("glass_pane") ||
           b.fName == "minecraft:iron_bars";
  }

  static bool IsGlassPaneOrIronBars(mcfile::je::Block const &b) {
    return b.fName.ends_with("glass_pane") ||
           b.fId == mcfile::blocks::minecraft::iron_bars;
  }

  static bool IsCampfire(mcfile::be::Block const &b) {
    return b.fName.ends_with("campfire");
  }

  static bool IsCampfire(mcfile::je::Block const &b) {
    using namespace mcfile::blocks;
    return b.fId == minecraft::campfire || b.fId == minecraft::soul_campfire;
  }

  static bool IsNoteBlock(mcfile::be::Block const &b) {
    return b.fName == "minecraft:noteblock";
  }

  static bool IsNoteBlock(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::note_block;
  }

  static bool IsRedstoneWire(mcfile::be::Block const &b) {
    return b.fName == "minecraft:redstone_wire";
  }

  static bool IsRedstoneWire(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::redstone_wire;
  }

  static bool IsTripwire(mcfile::be::Block const &b) {
    return b.fName == "minecraft:tripWire";
  }

  static bool IsTripwire(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::tripwire;
  }

  static bool IsPiston(mcfile::be::Block const &b) {
    return b.fName == "minecraft:piston" ||
           b.fName == "minecraft:sticky_piston" ||
           b.fName == "minecraft:pistonArmCollision" ||
           b.fName == "minecraft:stickyPistonArmCollision" ||
           b.fName == "minecraft:movingBlock";
  }

  static bool IsPiston(mcfile::je::Block const &b) {
    using namespace mcfile::blocks;
    return b.fId == minecraft::piston || b.fId == minecraft::piston_head || b.fId == minecraft::sticky_piston;
  }

  static bool IsBeacon(mcfile::be::Block const &b) {
    return b.fName == "minecraft:beacon";
  }

  static bool IsBeacon(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::beacon;
  }

  static bool IsMelonStem(mcfile::be::Block const &b) {
    return b.fName == "minecraft:melon_stem";
  }

  static bool IsMelonStem(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::melon_stem;
  }

  static bool IsDoor(mcfile::be::Block const &b) {
    return b.fName.ends_with("door") && b.fName.find("trap") == std::string::npos;
  }

  static bool IsDoor(mcfile::je::Block const &b) {
    return b.fName.ends_with("door") && b.fName.find("trap") == std::string::npos;
  }

  static bool IsWall(mcfile::be::Block const &b) {
    return b.fName.ends_with("wall");
  }

  static bool IsWall(mcfile::je::Block const &b) {
    return b.fName.ends_with("wall");
  }

  static bool IsLeaves(mcfile::be::Block const &b) {
    return b.fName.ends_with("leaves");
  }

  static bool IsLeaves(mcfile::je::Block const &b) {
    return b.fName.ends_with("leaves");
  }

  BlockPropertyAccessor(int cx, int cy, int cz) : fChunkX(cx), fChunkY(cy), fChunkZ(cz) {}
  virtual ~BlockPropertyAccessor() {}
  virtual DataType property(int bx, int by, int bz) const = 0;
  virtual int minBlockY() const = 0;
  virtual int maxBlockY() const = 0;

  void updateHasProperties(DataType p) {
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
    fHasMelonStem |= IsMelonStem(p);
    fHasDoor |= IsDoor(p);
    fHasWall |= IsWall(p);
    fHasLeaves |= IsLeaves(p);
  }

private:
  template <class BlockType>
  static DataType GetBlockProperties(BlockType const &b) {
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
    } else if (IsMelonStem(b)) {
      return MELON_STEM;
    } else if (IsDoor(b)) {
      return DOOR;
    } else if (IsWall(b)) {
      return WALL;
    } else if (IsLeaves(b)) {
      return LEAVES;
    }
    return 0;
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
  bool fHasMelonStem = false;
  bool fHasDoor = false;
  bool fHasWall = false;
  bool fHasLeaves = false;

protected:
  std::vector<std::vector<DataType>> fSections;
  int const fChunkX;
  int const fChunkY;
  int const fChunkZ;
};

class BlockPropertyAccessorBedrock : public BlockPropertyAccessor {
public:
  explicit BlockPropertyAccessorBedrock(mcfile::be::Chunk const &chunk) : BlockPropertyAccessor(chunk.fChunkX, chunk.fChunkY, chunk.fChunkZ), fChunk(chunk) {
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
        updateHasProperties(p);
        fSections[i][j] = p;
      }
    }
  }

  DataType property(int bx, int by, int bz) const override {
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
    if (index < 0 || section->fPaletteIndices.size() <= index) {
      return 0;
    }
    auto i = section->fPaletteIndices[index];
    return fSections[sectionIndex][i];
  }

  int minBlockY() const override {
    return fChunk.minBlockY();
  }

  int maxBlockY() const override {
    return fChunk.maxBlockY();
  }

private:
  mcfile::be::Chunk const &fChunk;
};

class BlockPropertyAccessorJava : public BlockPropertyAccessor {
public:
  explicit BlockPropertyAccessorJava(mcfile::je::Chunk const &chunk) : BlockPropertyAccessor(chunk.fChunkX, chunk.fChunkY, chunk.fChunkZ), fChunk(chunk) {
    using namespace std;
    fSections.resize(chunk.fSections.size());
    for (int i = 0; i < chunk.fSections.size(); i++) {
      auto const &section = chunk.fSections[i];
      if (!section) {
        continue;
      }
      section->eachBlockPalette([this, i](mcfile::je::Block const &blockJ) {
        auto p = BlockProperties(blockJ);
        updateHasProperties(p);
        fSections[i].push_back(p);
        return true;
      });
    }
  }

  DataType property(int bx, int by, int bz) const override {
    using namespace mcfile;
    using namespace mcfile::be;
    int cx = Coordinate::ChunkFromBlock(bx);
    int cy = Coordinate::ChunkFromBlock(by);
    int cz = Coordinate::ChunkFromBlock(bz);
    int sectionIndex = cy - fChunkY;
    if (sectionIndex < 0 || fChunk.fSections.size() <= sectionIndex) {
      return 0;
    }
    auto const &section = fChunk.fSections[sectionIndex];
    if (!section) {
      return 0;
    }
    int lx = bx - cx * 16;
    int ly = by - cy * 16;
    int lz = bz - cz * 16;
    auto index = section->blockPaletteIndexAt(lx, ly, lz);
    if (!index) {
      return 0;
    }
    if (0 <= *index && *index < fSections[sectionIndex].size()) {
      return fSections[sectionIndex][*index];
    } else {
      return 0;
    }
  }

  int minBlockY() const override {
    return fChunk.minBlockY();
  }

  int maxBlockY() const override {
    return fChunk.maxBlockY();
  }

private:
  mcfile::je::Chunk const &fChunk;
};

} // namespace je2be::terraform
