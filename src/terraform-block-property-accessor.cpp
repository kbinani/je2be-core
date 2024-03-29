#include "terraform/_block-property-accessor.hpp"

#include <minecraft-file.hpp>

namespace je2be::terraform {

class BlockPropertyAccessor::Impl {
  Impl() = delete;

public:
  static DataType BlockProperties(mcfile::be::Block const &b) {
    return GetBlockProperties(b);
  }

  static DataType BlockProperties(mcfile::je::Block const &b) {
    return GetBlockProperties(b);
  }

  static bool IsStairs(mcfile::be::Block const &b) {
    return b.fName.ends_with(u8"_stairs");
  }

  static bool IsStairs(mcfile::je::Block const &b) {
    return b.fName.ends_with(u8"_stairs");
  }

  static bool IsKelp(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:kelp";
  }

  static bool IsKelp(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::kelp || b.fId == mcfile::blocks::minecraft::kelp_plant;
  }

  static bool IsTwistingVines(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:twisting_vines";
  }

  static bool IsTwistingVines(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::twisting_vines || b.fId == mcfile::blocks::minecraft::twisting_vines_plant;
  }

  static bool IsWeepingVines(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:weeping_vines";
  }

  static bool IsWeepingVines(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::weeping_vines || b.fId == mcfile::blocks::minecraft::weeping_vines_plant;
  }

  static bool IsPumpkinStem(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:pumpkin_stem";
  }

  static bool IsPumpkinStem(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::pumpkin_stem;
  }

  static bool IsCaveVines(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:cave_vines" ||
           b.fName == u8"minecraft:cave_vines_head_with_berries" ||
           b.fName == u8"minecraft:cave_vines_body_with_berries";
  }

  static bool IsCaveVines(mcfile::je::Block const &b) {
    using namespace mcfile::blocks;
    return b.fId == minecraft::cave_vines || b.fId == minecraft::cave_vines_plant;
  }

  static bool IsSnowy(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:grass" ||
           b.fName == u8"minecraft:podzol" ||
           b.fName == u8"minecraft:mycelium";
  }

  static bool IsSnowy(mcfile::je::Block const &b) {
    using namespace mcfile::blocks;
    return b.fId == minecraft::grass_block || b.fId == minecraft::podzol || b.fId == minecraft::mycelium;
  }

  static bool IsChorusPlant(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:chorus_plant" ||
           b.fName == u8"minecraft:chorus_flower";
  }

  static bool IsChorusPlant(mcfile::je::Block const &b) {
    using namespace mcfile::blocks;
    return b.fId == minecraft::chorus_plant || b.fId == minecraft::chorus_flower;
  }

  static bool IsFence(mcfile::be::Block const &b) {
    return b.fName.ends_with(u8"fence");
  }

  static bool IsFence(mcfile::je::Block const &b) {
    return b.fName.ends_with(u8"fence");
  }

  static bool IsGlassPaneOrIronBars(mcfile::be::Block const &b) {
    return b.fName.ends_with(u8"glass_pane") ||
           b.fName == u8"minecraft:iron_bars";
  }

  static bool IsGlassPaneOrIronBars(mcfile::je::Block const &b) {
    return b.fName.ends_with(u8"glass_pane") ||
           b.fId == mcfile::blocks::minecraft::iron_bars;
  }

  static bool IsCampfire(mcfile::be::Block const &b) {
    return b.fName.ends_with(u8"campfire");
  }

  static bool IsCampfire(mcfile::je::Block const &b) {
    using namespace mcfile::blocks;
    return b.fId == minecraft::campfire || b.fId == minecraft::soul_campfire;
  }

  static bool IsNoteBlock(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:noteblock";
  }

  static bool IsNoteBlock(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::note_block;
  }

  static bool IsRedstoneWire(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:redstone_wire";
  }

  static bool IsRedstoneWire(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::redstone_wire;
  }

  static bool IsTripwire(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:tripWire" ||
           b.fName == u8"minecraft:trip_wire";
  }

  static bool IsTripwire(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::tripwire;
  }

  static bool IsBed(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:bed";
  }

  static bool IsBed(mcfile::je::Block const &b) {
    return b.fName.ends_with(u8"_bed");
  }

  static bool IsPiston(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:piston" ||
           b.fName == u8"minecraft:sticky_piston" ||
           b.fName == u8"minecraft:pistonArmCollision" ||
           b.fName == u8"minecraft:piston_arm_collision" ||
           b.fName == u8"minecraft:stickyPistonArmCollision" ||
           b.fName == u8"minecraft:sticky_piston_arm_collision" ||
           b.fName == u8"minecraft:movingBlock" ||
           b.fName == u8"minecraft:moving_block";
  }

  static bool IsPiston(mcfile::je::Block const &b) {
    using namespace mcfile::blocks;
    return b.fId == minecraft::piston || b.fId == minecraft::piston_head || b.fId == minecraft::sticky_piston;
  }

  static bool IsBeacon(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:beacon";
  }

  static bool IsBeacon(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::beacon;
  }

  static bool IsMelonStem(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:melon_stem";
  }

  static bool IsMelonStem(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::melon_stem;
  }

  static bool IsDoor(mcfile::be::Block const &b) {
    return b.fName.ends_with(u8"door") && b.fName.find(u8"trap") == std::string::npos;
  }

  static bool IsDoor(mcfile::je::Block const &b) {
    return b.fName.ends_with(u8"door") && b.fName.find(u8"trap") == std::string::npos;
  }

  static bool IsWall(mcfile::be::Block const &b) {
    return b.fName.ends_with(u8"wall");
  }

  static bool IsWall(mcfile::je::Block const &b) {
    return b.fName.ends_with(u8"wall");
  }

  static bool IsLeaves(mcfile::be::Block const &b) {
    return b.fName.ends_with(u8"leaves") || b.fName.ends_with(u8"leaves2") || b.fName == u8"minecraft:azalea_leaves_flowered";
  }

  static bool IsLeaves(mcfile::je::Block const &b) {
    return b.fName.ends_with(u8"leaves");
  }

  static bool IsChest(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:chest" || b.fName == u8"minecraft:trapped_chest";
  }

  static bool IsChest(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::chest || b.fId == mcfile::blocks::minecraft::trapped_chest;
  }

  static bool IsNetherPortal(mcfile::je::Block const &b) {
    return b.fId == mcfile::blocks::minecraft::nether_portal;
  }

  static bool IsNetherPortal(mcfile::be::Block const &b) {
    return b.fName == u8"minecraft:portal";
  }

  static bool IsDoublePlantUpperSunflower(mcfile::je::Block const &b) {
    if (b.fId != mcfile::blocks::minecraft::sunflower) {
      return false;
    }
    return b.property(u8"half") == u8"upper";
  }

  static bool IsDoublePlantUpperSunflower(mcfile::be::Block const &b) {
    if (b.fName != u8"minecraft:double_plant") {
      return false;
    }
    if (!b.fStates) {
      return false;
    }
    return b.fStates->string(u8"double_plant_type") == u8"sunflower" && b.fStates->boolean(u8"upper_block_bit") == true;
  }

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
    } else if (IsChest(b)) {
      return CHEST;
    } else if (IsNetherPortal(b)) {
      return NETHER_PORTAL;
    } else if (IsBed(b)) {
      return BED;
    } else if (IsDoublePlantUpperSunflower(b)) {
      return DOUBLE_PLANT_UPPER_SUNFLOWER;
    }
    return 0;
  }
};

BlockPropertyAccessorBedrock::BlockPropertyAccessorBedrock(mcfile::be::Chunk const &chunk) : BlockPropertyAccessor(chunk.fChunkX, chunk.fChunkY, chunk.fChunkZ), fChunk(chunk) {
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

BlockPropertyAccessorBedrock::~BlockPropertyAccessorBedrock() {}

int BlockPropertyAccessorBedrock::minBlockY() const {
  return fChunk.minBlockY();
}

int BlockPropertyAccessorBedrock::maxBlockY() const {
  return fChunk.maxBlockY();
}

BlockPropertyAccessor::DataType BlockPropertyAccessorBedrock::property(int bx, int by, int bz) const {
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

BlockPropertyAccessorJava::BlockPropertyAccessorJava(mcfile::je::Chunk const &chunk) : BlockPropertyAccessor(chunk.fChunkX, chunk.fChunkY, chunk.fChunkZ), fChunk(chunk) {
  using namespace std;
  fSections.resize(chunk.fSections.size());
  for (int i = 0; i < chunk.fSections.size(); i++) {
    auto const &section = chunk.fSections[i];
    if (!section) {
      continue;
    }
    section->eachBlockPalette([this, i](shared_ptr<mcfile::je::Block const> const &blockJ, size_t) {
      auto p = BlockProperties(*blockJ);
      updateHasProperties(p);
      fSections[i].push_back(p);
      return true;
    });
  }
}

BlockPropertyAccessorJava::~BlockPropertyAccessorJava() {}

int BlockPropertyAccessorJava::minBlockY() const {
  return fChunk.minBlockY();
}

int BlockPropertyAccessorJava::maxBlockY() const {
  return fChunk.maxBlockY();
}

BlockPropertyAccessor::DataType BlockPropertyAccessorJava::property(int bx, int by, int bz) const {
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

BlockPropertyAccessor::DataType BlockPropertyAccessor::BlockProperties(mcfile::be::Block const &b) {
  return Impl::GetBlockProperties(b);
}

BlockPropertyAccessor::DataType BlockPropertyAccessor::BlockProperties(mcfile::je::Block const &b) {
  return Impl::GetBlockProperties(b);
}

void BlockPropertyAccessor::updateHasProperties(DataType p) {
  fHasStairs |= p == STAIRS;
  fHasKelp |= p == KELP;
  fHasTwistingVines |= p == TWISTING_VINES;
  fHasWeepingVines |= p == WEEPING_VINES;
  fHasPumpkinStem |= p == PUMPKIN_STEM;
  fHasCaveVines |= p == CAVE_VINES;
  fHasSnowy |= p == SNOWY;
  fHasChorusPlant |= p == CHORUS_PLANT;
  fHasFence |= p == FENCE;
  fHasGlassPaneOrIronBars |= p == GLASS_PANE_OR_IRON_BARS;
  fHasCampfire |= p == CAMPFIRE;
  fHasNoteBlock |= p == NOTE_BLOCK;
  fHasRedstoneWire |= p == REDSTONE_WIRE;
  fHasTripwire |= p == TRIPWIRE;
  fHasPiston |= p == PISTON;
  fHasBeacon |= p == BEACON;
  fHasMelonStem |= p == MELON_STEM;
  fHasDoor |= p == DOOR;
  fHasWall |= p == WALL;
  fHasLeaves |= p == LEAVES;
  fHasChest |= p == CHEST;
  fHasNetherPortal |= p == NETHER_PORTAL;
  fHasBed |= p == BED;
  fHasDoublePlantUpperSunflower |= p == DOUBLE_PLANT_UPPER_SUNFLOWER;
}

bool BlockPropertyAccessor::IsChorusPlant(mcfile::je::Block const &b) {
  return Impl::IsChorusPlant(b);
}

bool BlockPropertyAccessor::IsLeaves(mcfile::je::Block const &b) {
  return Impl::IsLeaves(b);
}

bool BlockPropertyAccessor::IsLeaves(mcfile::be::Block const &b) {
  return Impl::IsLeaves(b);
}

bool BlockPropertyAccessor::IsTripwire(mcfile::be::Block const &b) {
  return Impl::IsTripwire(b);
}

} // namespace je2be::terraform
