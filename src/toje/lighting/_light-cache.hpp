#pragma once

#include "toje/lighting/_lighting-model.hpp"
#include "toje/lighting/_lighting-property.hpp"

namespace je2be::toje::lighting {

class LightCache {
public:
  LightCache(int rx, int rz)
      : fRx(rx), fRz(rz), fProperties({rx * 32 - 1, rz * 32 - 1}, 34, 34, nullptr), fModels({rx * 32 - 1, rz * 32 - 1}, 34, 34, nullptr) {}

  std::shared_ptr<Data3dSq<LightingProperty, 16>> getProperty(int cx, int cz) {
    return fProperties[{cx, cz}];
  }

  void setProperty(int cx, int cz, std::shared_ptr<Data3dSq<LightingProperty, 16>> const &data) {
    fProperties[{cx, cz}] = data;
  }

  std::shared_ptr<Data3dSq<LightingModel, 16>> getModel(int cx, int cz) {
    return fModels[{cx, cz}];
  }

  void setModel(int cx, int cz, std::shared_ptr<Data3dSq<LightingModel, 16>> const &data) {
    fModels[{cx, cz}] = data;
  }

private:
  int const fRx;
  int const fRz;
  Data2d<std::shared_ptr<Data3dSq<LightingProperty, 16>>> fProperties;
  Data2d<std::shared_ptr<Data3dSq<LightingModel, 16>>> fModels;
};

} // namespace je2be::toje::lighting
