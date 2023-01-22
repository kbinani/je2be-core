#pragma once

#include "_lattice-container-wrapper.hpp"
#include "toje/lighting/_chunk-lighting-model.hpp"
#include "toje/lighting/_lighting-model.hpp"

namespace je2be::toje::lighting {

class LightCache {
public:
  LightCache(int rx, int rz)
      : fRx(rx), fRz(rz), fModels({rx * 32 - 1, rz * 32 - 1}, 34, 34, nullptr), fSkyLights({rx * 32 - 1, rz * 32 - 1}, 34, 34, nullptr), fBlockLights({rx * 32 - 1, rz * 32 - 1}, 34, 34, nullptr) {}

  std::shared_ptr<ChunkLightingModel> getModel(int cx, int cz) {
    return fModels[{cx, cz}];
  }

  void setModel(int cx, int cz, std::shared_ptr<ChunkLightingModel> const &data) {
    fModels[{cx, cz}] = data;
  }

  // disposes fModels from [0, 0] to [cx, cz] (z first as `for(z = ...) { for (x = ...`)
  void dispose(int cx, int cz) {
    if (cx < fRx * 32 - 1 || cz < fRz * 32 - 1) {
      return;
    }
    for (int z = fRz * 32 - 1; z <= cz; z++) {
      for (int x = fRx * 32 - 1; x <= fRx * 32 + 32; x++) {
        fModels[{x, z}].reset();
        fSkyLights[{x, z}].reset();
        fBlockLights[{x, z}].reset();
        if (z == cz && x == cx) {
          return;
        }
      }
    }
  }

  std::shared_ptr<ChunkLightCache> getSkyLight(int cx, int cz) {
    return fSkyLights[{cx, cz}];
  }

  void setSkyLight(int cx, int cz, std::shared_ptr<ChunkLightCache> const &light) {
    fSkyLights[{cx, cz}] = light;
  }

  std::shared_ptr<ChunkLightCache> getBlockLight(int cx, int cz) {
    return fBlockLights[{cx, cz}];
  }

  void setBlockLight(int cx, int cz, std::shared_ptr<ChunkLightCache> const &light) {
    fBlockLights[{cx, cz}] = light;
  }

private:
  int const fRx;
  int const fRz;
  Data2d<std::shared_ptr<ChunkLightingModel>> fModels;
  Data2d<std::shared_ptr<ChunkLightCache>> fSkyLights;
  Data2d<std::shared_ptr<ChunkLightCache>> fBlockLights;
};

} // namespace je2be::toje::lighting
