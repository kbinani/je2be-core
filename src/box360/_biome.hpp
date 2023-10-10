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
    } else if (d == mcfile::Dimension::End) {
      switch (raw) {
      case 255:
        return mcfile::biomes::minecraft::the_end;
      default:
        return mcfile::be::Biome::FromUint32(raw);
      }
    } else {
      return mcfile::be::Biome::FromUint32(raw);
    }
  }
};

} // namespace je2be::box360
