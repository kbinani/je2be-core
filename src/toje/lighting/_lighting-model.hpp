#pragma once

namespace je2be::toje::lighting {

enum Transparency : uint8_t {
  CLEAR = 0,
  TRANSLUCENT = 1,
  SOLID = 2,
};

struct LightingModel {
  uint32_t fModel = 0;
  uint8_t fEmission = 0;
  bool fBehaveAsAirWhenOpenUp = false;
  Transparency fTransparency = CLEAR;
};

} // namespace je2be::toje::lighting
