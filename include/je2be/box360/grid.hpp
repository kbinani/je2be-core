#pragma once

namespace je2be::box360 {

class Grid {
  Grid() = delete;

public:
  static BlockData BlockDataFromBytes(uint8_t v1, uint8_t v2) {
    uint8_t t1 = v1 >> 4;
    uint8_t t2 = 0xf & v1;
    uint8_t t3 = v2 >> 4;
    uint8_t t4 = 0xf & v2;
    uint8_t blockId = t4 << 4 | t1;
    uint8_t data = t3 << 4 | t2;
    BlockData bd(blockId, data);
    return bd;
  }

  static void ParsePalette(uint8_t const *buffer, std::vector<BlockData> &palette, int maxSize) {
    palette.clear();
    for (uint16_t i = 0; i < maxSize; i++) {
      uint8_t v1 = buffer[i * 2];
      uint8_t v2 = buffer[i * 2 + 1];
      auto block = BlockDataFromBytes(v1, v2);
      palette.push_back(block);
    }
  }

  static void ParseFormat0(uint8_t v1, uint8_t v2, BlockData grid[64]) {
    auto block = BlockDataFromBytes(v1, v2);
    std::fill_n(grid, 64, block);
  }

  static void ParseFormatF(uint8_t const *buffer, BlockData grid[64]) {
    std::vector<BlockData> palette;
    ParsePalette(buffer, palette, 64);
    std::copy_n(palette.begin(), 64, grid);
  }

  template <size_t BitsPerBlock>
  static bool Parse(uint8_t const *buffer, BlockData grid[64]) {
    using namespace std;
    int size = 1 << BitsPerBlock;
    vector<BlockData> palette;
    ParsePalette(buffer, palette, size);
    for (int i = 0; i < 8; i++) {
      uint8_t v[BitsPerBlock];
      for (int j = 0; j < BitsPerBlock; j++) {
        v[j] = buffer[size * 2 + i + j * 8];
      }
      for (int j = 0; j < 8; j++) {
        uint8_t mask = (uint8_t)0x80 >> j;
        uint16_t idx = 0;
        for (int k = 0; k < BitsPerBlock; k++) {
          idx |= ((v[k] & mask) >> (7 - j)) << k;
        }
        if (idx >= palette.size()) [[unlikely]] {
          return false;
        }
        grid[i * 8 + j] = palette[idx];
      }
    }
    return true;
  }
};

} // namespace je2be::box360
