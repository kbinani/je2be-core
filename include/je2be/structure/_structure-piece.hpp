#pragma once

#include <minecraft-file.hpp>

#include <je2be/_pos3.hpp>
#include <je2be/_volume.hpp>

namespace je2be {

enum class StructureType : uint8_t {
  Fortress = 1,
  Monument = 3,
  Outpost = 5,
};

struct StructurePiece {
  StructurePiece(Pos3i start, Pos3i end, StructureType type) : fVolume(start, end), fType(type) {}

  Volume fVolume;
  StructureType fType;

  static bool Parse(std::string const &data, std::vector<StructurePiece> &buffer) {
    auto s = std::make_shared<mcfile::stream::ByteInputStream>(data);
    mcfile::stream::InputStreamReader isr(s, mcfile::Endian::Little);
    uint32_t size = 0;
    if (!isr.read(&size)) {
      return false;
    }
    for (int i = 0; i < size; i++) {
      Pos3i start;
      Pos3i end;
      if (!isr.read(&start.fX)) {
        return false;
      }
      if (!isr.read(&start.fY)) {
        return false;
      }
      if (!isr.read(&start.fZ)) {
        return false;
      }
      if (!isr.read(&end.fX)) {
        return false;
      }
      if (!isr.read(&end.fY)) {
        return false;
      }
      if (!isr.read(&end.fZ)) {
        return false;
      }
      uint8_t type;
      if (!isr.read(&type)) {
        return false;
      }
      StructurePiece sp(start, end, static_cast<StructureType>(type));
      buffer.push_back(sp);
    }
    return true;
  }
};

} // namespace je2be
