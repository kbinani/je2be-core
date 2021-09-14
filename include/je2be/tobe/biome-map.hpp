#pragma once

namespace je2be::tobe {

class BiomeMap {
public:
  BiomeMap() {
    auto ocean = mcfile::be::Biome::ToUint8(mcfile::biomes::minecraft::ocean);
    std::fill_n(fBiome, 256, ocean);
  }

  void set(int localX, int localZ, mcfile::biomes::BiomeId biome) {
    assert(0 <= localX && localX < 16);
    assert(0 <= localZ && localZ < 16);
    size_t i = localZ * 16 + localX;
    fBiome[i] = mcfile::be::Biome::ToUint8(biome);
  }

  [[nodiscard]] bool write(mcfile::stream::OutputStreamWriter &w) {
    size_t i = 0;
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++, i++) {
        uint8_t h = fBiome[i];
        if (!w.write(h)) {
          return false;
        }
      }
    }
    return true;
  }

private:
  uint8_t fBiome[256];
};

} // namespace je2be::tobe
