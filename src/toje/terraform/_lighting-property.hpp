#pragma once

namespace je2be::toje::lighting {

enum Transparency : uint8_t {
  CLEAR = 0,
  TRANSLUCENT = 1,
  SOLID = 2,
};

struct LightingProperty {
  Transparency fUp : 2;
  Transparency fNorth : 2;
  Transparency fEast : 2;
  Transparency fSouth : 2;
  Transparency fWest : 2;
  Transparency fDown : 2;
  uint8_t fEmission : 4;

  LightingProperty() {
    fUp = CLEAR;
    fNorth = CLEAR;
    fEast = CLEAR;
    fSouth = CLEAR;
    fWest = CLEAR;
    fDown = CLEAR;
    fEmission = 0;
  }

  explicit LightingProperty(Transparency v) {
    fUp = v;
    fNorth = v;
    fEast = v;
    fSouth = v;
    fWest = v;
    fDown = v;
    fEmission = 0;
  }

  bool isSolid() const {
    return fUp == SOLID && fNorth == SOLID && fEast == SOLID && fSouth == SOLID && fWest == SOLID && fDown == SOLID;
  }
};

} // namespace je2be::toje::lighting
