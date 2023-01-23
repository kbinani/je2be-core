#pragma once

namespace je2be::box360 {

class Grid {
  Grid() = delete;

public:
  static u16 BlockDataFromBytes(u8 v1, u8 v2) {
    u8 t1 = v1 >> 4;
    u8 t2 = 0xf & v1;
    u8 t3 = v2 >> 4;
    u8 t4 = 0xf & v2;
    u8 blockId = t4 << 4 | t1;
    u8 data = t3 << 4 | t2;
    return (u16)blockId << 8 | (u16)data;
  }

  static void ParsePalette(u8 const *buffer, std::vector<u16> &palette, int maxSize) {
    palette.clear();
    for (int i = 0; i < maxSize; i++) {
      u8 v1 = buffer[i * 2];
      u8 v2 = buffer[i * 2 + 1];
      auto block = BlockDataFromBytes(v1, v2);
      palette.push_back(block);
    }
  }

  static void ParseFormat0(u8 v1, u8 v2, u16 grid[64]) {
    auto block = BlockDataFromBytes(v1, v2);
    std::fill_n(grid, 64, block);
  }

  static void ParseFormatF(u8 const *buffer, u16 grid[64]) {
    std::vector<u16> palette;
    ParsePalette(buffer, palette, 64);
    std::copy_n(palette.begin(), 64, grid);
  }

  template <size_t BitsPerBlock>
  static bool Parse(u8 const *buffer, u16 grid[64]) {
    using namespace std;
    int size = 1 << BitsPerBlock;
    vector<u16> palette;
    ParsePalette(buffer, palette, size);
    for (int i = 0; i < 8; i++) {
      u8 v[BitsPerBlock];
      for (int j = 0; j < BitsPerBlock; j++) {
        v[j] = buffer[size * 2 + i + j * 8];
      }
      for (int j = 0; j < 8; j++) {
        u8 mask = (u8)0x80 >> j;
        u16 idx = 0;
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
