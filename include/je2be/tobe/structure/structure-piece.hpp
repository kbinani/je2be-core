#pragma once

namespace je2be::tobe {

enum class StructureType : uint8_t {
  Fortress = 1,
  Monument = 3,
  Outpost = 5,
};

struct StructurePiece {
  StructurePiece(Pos3i start, Pos3i end, StructureType type) : fVolume(start, end), fType(type) {}

  Volume fVolume;
  StructureType fType;
};

} // namespace je2be::tobe
