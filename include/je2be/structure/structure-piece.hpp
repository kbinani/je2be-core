#pragma once

namespace j2b {

enum class StructureType : uint8_t {
  Fortress = 1,
  Monument = 3,
  Outpost = 5,
};

struct StructurePiece {
  StructurePiece(Pos start, Pos end, StructureType type)
      : fVolume(start, end), fType(type) {}

  Volume fVolume;
  StructureType fType;
};

} // namespace j2b
