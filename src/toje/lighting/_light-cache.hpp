#pragma once

#include "toje/lighting/_lighting-model.hpp"

namespace je2be::toje::lighting {

class LightCache {
public:
  LightCache(int rx, int rz)
      : fRx(rx), fRz(rz), fModels({rx * 32 - 1, rz * 32 - 1}, 34, 34, nullptr) {}

  std::shared_ptr<Data3dSq<LightingModel, 16>> getModel(int cx, int cz) {
    return fModels[{cx, cz}];
  }

  void setModel(int cx, int cz, std::shared_ptr<Data3dSq<LightingModel, 16>> const &data) {
    fModels[{cx, cz}] = data;
  }

  // disposes fModels from [0, 0] to [cx, cz] (z first as `for(z = ...) { for (x = ...`)
  void dispose(int cx, int cz) {
    if (cx < fRx * 32 || cz < fRz * 32) {
      return;
    }
    for (int z = fRz * 32; z <= cz; z++) {
      for (int x = fRx * 32; x < fRx * 32 + 32; x++) {
        fModels[{x, z}].reset();
        if (z == cz && x == cx) {
          break;
        }
      }
    }
  }

private:
  int const fRx;
  int const fRz;
  Data2d<std::shared_ptr<Data3dSq<LightingModel, 16>>> fModels;
};

} // namespace je2be::toje::lighting
