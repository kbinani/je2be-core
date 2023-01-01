#pragma once

#include <je2be/mcfile-fwd.hpp>

#include <cstdint>
#include <vector>

namespace je2be::terraform {

class BlockPropertyAccessor {
  class Impl;

public:
  using DataType = uint8_t;

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
    CHEST = 22, // chest or trapped_chest
  };

public:
  static DataType BlockProperties(mcfile::be::Block const &b);
  static DataType BlockProperties(mcfile::je::Block const &b);

  BlockPropertyAccessor(int cx, int cy, int cz) : fChunkX(cx), fChunkY(cy), fChunkZ(cz) {}
  virtual ~BlockPropertyAccessor() {}
  virtual DataType property(int bx, int by, int bz) const = 0;
  virtual int minBlockY() const = 0;
  virtual int maxBlockY() const = 0;

  void updateHasProperties(DataType p);

  static bool IsChorusPlant(mcfile::je::Block const &b);
  static bool IsLeaves(mcfile::je::Block const &b);
  static bool IsLeaves(mcfile::be::Block const &b);
  static bool IsTripwire(mcfile::be::Block const &b);

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
  bool fHasChest = false;

protected:
  std::vector<std::vector<DataType>> fSections;
  int const fChunkX;
  int const fChunkY;
  int const fChunkZ;
};

class BlockPropertyAccessorBedrock : public BlockPropertyAccessor {
public:
  explicit BlockPropertyAccessorBedrock(mcfile::be::Chunk const &chunk);
  ~BlockPropertyAccessorBedrock();

  DataType property(int bx, int by, int bz) const override;

  int minBlockY() const override;

  int maxBlockY() const override;

private:
  mcfile::be::Chunk const &fChunk;
};

class BlockPropertyAccessorJava : public BlockPropertyAccessor {
public:
  explicit BlockPropertyAccessorJava(mcfile::je::Chunk const &chunk);
  ~BlockPropertyAccessorJava();

  DataType property(int bx, int by, int bz) const override;

  int minBlockY() const override;

  int maxBlockY() const override;

private:
  mcfile::je::Chunk const &fChunk;
};

} // namespace je2be::terraform
