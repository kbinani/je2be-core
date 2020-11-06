#pragma once

namespace j2b {

class BiomeMap {
public:
  BiomeMap() {
    auto ocean = From(mcfile::biomes::minecraft::ocean);
    std::fill_n(fBiome, 256, ocean);
  }

  void set(int localX, int localZ, mcfile::biomes::BiomeId biome) {
    assert(0 <= localX && localX < 16);
    assert(0 <= localZ && localZ < 16);
    size_t i = localZ * 16 + localX;
    fBiome[i] = From(biome);
  }

  static uint8_t From(mcfile::biomes::BiomeId biome) {
    using namespace mcfile::biomes::minecraft;
    switch (biome) {
    case ocean:
      return 42;
    case deep_ocean:
      return 24;
    case frozen_ocean:
      return 10;
    case deep_frozen_ocean:
      return 50;
    case cold_ocean:
      return 46;
    case deep_cold_ocean:
      return 49;
    case lukewarm_ocean:
      return 45;
    case deep_lukewarm_ocean:
      return 48;
    case warm_ocean:
      return 44;
    case deep_warm_ocean:
      return 47;
    case river:
      return 7;
    case frozen_river:
      return 11;
    case beach:
      return 16;
    case stone_shore:
      return 25;
    case snowy_beach:
      return 26;
    case forest:
      return 4;
    case wooded_hills:
      return 18;
    case flower_forest:
      return 132;
    case birch_forest:
      return 27;
    case birch_forest_hills:
      return 28;
    case tall_birch_forest:
      return 155;
    case tall_birch_hills:
      return 156;
    case dark_forest:
      return 29;
    case dark_forest_hills:
      return 157;
    case jungle:
      return 21;
    case jungle_hills:
      return 22;
    case modified_jungle:
      return 149;
    case jungle_edge:
      return 23;
    case modified_jungle_edge:
      return 151;
    case bamboo_jungle:
      return 168;
    case bamboo_jungle_hills:
      return 169;
    case taiga:
      return 5;
    case taiga_hills:
      return 19;
    case taiga_mountains:
      return 133;
    case snowy_taiga:
      return 30;
    case snowy_taiga_hills:
      return 31;
    case snowy_taiga_mountains:
      return 158;
    case giant_tree_taiga:
      return 32;
    case giant_tree_taiga_hills:
      return 33;
    case giant_spruce_taiga:
      return 160;
    case giant_spruce_taiga_hills:
      return 161;
    case mushroom_fields:
      return 14;
    case mushroom_field_shore:
      return 15;
    case swamp:
      return 6;
    case swamp_hills:
      return 134;
    case savanna:
      return 35;
    case savanna_plateau:
      return 36;
    case shattered_savanna:
      return 163;
    case shattered_savanna_plateau:
      return 164;
    case plains:
      return 1;
    case sunflower_plains:
      return 129;
    case desert:
      return 2;
    case desert_hills:
      return 17;
    case desert_lakes:
      return 130;
    case snowy_tundra:
      return 12;
    case snowy_mountains:
      return 13;
    case ice_spikes:
      return 140;
    case mountains:
      return 3;
    case wooded_mountains:
      return 34;
    case gravelly_mountains:
      return 131;
    case modified_gravelly_mountains:
      return 162;
    case mountain_edge:
      return 20;
    case badlands:
      return 37;
    case badlands_plateau:
      return 39;
    case modified_badlands_plateau:
      return 167;
    case wooded_badlands_plateau:
      return 38;
    case modified_wooded_badlands_plateau:
      return 166;
    case eroded_badlands:
      return 165;
    case nether_wastes:
      return 8;
    case crimson_forest:
      return 179;
    case warped_forest:
      return 180;
    case soul_sand_valley:
      return 178;
    case basalt_deltas:
      return 181;
    case the_end:
    case small_end_islands:
    case end_midlands:
    case end_highlands:
    case end_barrens:
      return 9;
    case the_void:
    default:
      assert(false);
      return 42;
    }
  }

  void write(mcfile::stream::OutputStreamWriter &w) {
    size_t i = 0;
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++, i++) {
        uint8_t h = fBiome[i];
        w.write(h);
      }
    }
  }

private:
  uint8_t fBiome[256];
};

} // namespace j2b
