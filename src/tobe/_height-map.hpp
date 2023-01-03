#pragma once

namespace je2be::tobe {

class HeightMap {
public:
  explicit HeightMap(int minChunkY) : fMinChunkY(minChunkY) {
    std::fill_n(fHeight, 256, 0);
  }

  void update(int localX, int y, int localZ) {
    assert(0 <= localX && localX < 16);
    assert(0 <= localZ && localZ < 16);
    size_t i = localZ * 16 + localX;
    int height = y - fMinChunkY * 16;
    uint16_t clamped = mcfile::Clamp<uint16_t>(height);
    fHeight[i] = (std::max)(fHeight[i], clamped);
  }

  void offset(int newMinChunkY) {
    if (newMinChunkY == fMinChunkY) {
      return;
    }
    int delta = (fMinChunkY - newMinChunkY) * 16;
    for (int i = 0; i < 256; i++) {
      uint16_t prev = fHeight[i];
      uint16_t next = mcfile::Clamp<uint16_t>((int)prev + delta);
      fHeight[i] = next;
    }
    fMinChunkY = newMinChunkY;
  }

  [[nodiscard]] bool write(mcfile::stream::OutputStreamWriter &w) {
    size_t i = 0;
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++, i++) {
        uint16_t h = fHeight[i];
        if (!w.write(h)) {
          return false;
        }
      }
    }
    return true;
  }

private:
  int fMinChunkY;
  uint16_t fHeight[256];
};

} // namespace je2be::tobe
