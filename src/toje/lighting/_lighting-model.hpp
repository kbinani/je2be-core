#pragma once

namespace je2be::toje::lighting {

enum Transparency : uint8_t {
  CLEAR = 0,
  TRANSLUCENT = 1,
  SOLID = 2,
};

union LightingModel {
  struct {
    uint32_t fEmission : 4;
    uint32_t fBehaveAsAirWhenOpenUp : 1;
    uint32_t fTransparency : 3;
    uint32_t fModel : 24;
  };
  uint32_t fRaw;

  LightingModel(Transparency t) {
    fTransparency = t;
    fEmission = 0;
    fBehaveAsAirWhenOpenUp = false;
    fModel = 0;
  }

  LightingModel() {
    fTransparency = CLEAR;
    fEmission = 0;
    fBehaveAsAirWhenOpenUp = false;
    fModel = 0;
  }

  struct Hasher {
    size_t operator()(LightingModel const &k) const {
      return std::hash<uint32_t>{}(k.fRaw);
    }
  };

  struct EqualTo {
    bool operator()(LightingModel const &a, LightingModel const &b) const {
      return a.fRaw == b.fRaw;
    }
  };
};

static_assert(sizeof(LightingModel) == sizeof(uint32_t));

} // namespace je2be::toje::lighting
