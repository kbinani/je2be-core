#pragma once

#include "toje/lighting/_lighting-model.hpp"

namespace je2be::toje::lighting {

class ChunkLightingModel {

public:
  using ValueType = LightingModel;
  using Section = mcfile::PaletteList<LightingModel, u16, 4096, LightingModel::Hasher, LightingModel::EqualTo>;

  ChunkLightingModel(int cx, int cy, int cz) : fChunkX(cx), fChunkY(cy), fChunkZ(cz) {}

  LightingModel operator[](Pos3i const &p) const {
    static LightingModel const sAir(CLEAR);
    int cy = mcfile::Coordinate::ChunkFromBlock(p.fY);
    int iy = cy - fChunkY;
    if (iy < 0 || fSections.size() <= iy) {
      return sAir;
    }
    auto const &section = fSections[iy];
    if (!section) {
      return sAir;
    }
    if (section->empty()) {
      return sAir;
    }
    int dx = p.fX - fChunkX * 16;
    int dy = p.fY - cy * 16;
    int dz = p.fZ - fChunkZ * 16;
    assert(0 <= dx && dx < 16 && 0 <= dy && dy < 16 && 0 <= dz && dz < 16);
    return section->getUnchecked((dy * 16 + dz) * 16 + dx);
  }

  void setSection(int cy, std::shared_ptr<Section> const &s) {
    int i = cy - fChunkY;
    if (fSections.size() <= i) {
      fSections.resize(i + 1);
    }
    fSections[i] = s;
  }

public:
  int const fChunkX;
  int const fChunkY;
  int const fChunkZ;
  std::vector<std::shared_ptr<Section>> fSections;
};

} // namespace je2be::toje::lighting
