#pragma once

namespace je2be::box360 {

class Biome {
  Biome() = delete;

public:
  static mcfile::biomes::BiomeId FromUint32(mcfile::Dimension d, u8 raw) {
    if (d == mcfile::Dimension::Nether) {
      switch (raw) {
      case 7:
      default:
        return mcfile::biomes::minecraft::nether_wastes;
      }
    } else {
      return mcfile::be::Biome::FromUint32(raw);
    }
  }
};

} // namespace je2be::box360
