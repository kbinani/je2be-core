#pragma once

namespace je2be::tobe {

class HeightMap {
public:
  HeightMap() { std::fill_n(fHeight, 256, 0); }

  void update(int localX, int y, int localZ) {
    assert(0 <= localX && localX < 16);
    assert(0 <= localZ && localZ < 16);
    assert(0 <= y && y < 256);
    size_t i = localZ * 16 + localX;
    int16_t clamped = mcfile::Clamp<int16_t>(y);
    int16_t height = (std::max)(fHeight[i], clamped);
    fHeight[i] = height;
  }

  [[nodiscard]] bool write(mcfile::stream::OutputStreamWriter &w) {
    size_t i = 0;
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++, i++) {
        int16_t h = fHeight[i];
        if (!w.write(h)) {
          return false;
        }
      }
    }
    return true;
  }

private:
  int16_t fHeight[256];
};

} // namespace je2be::tobe
