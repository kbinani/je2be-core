#pragma once

namespace je2be::toje::lighting {

enum Transparency : u8 {
  CLEAR = 0,
  TRANSLUCENT = 1,
  SOLID = 2,
};

enum Model : u32 {
  // https://gyazo.com/be22882432da5269d283d90cdcd04282
  MODEL_SOLID = 0xffffff,
  MODEL_CLEAR = 0,
  MODEL_HALF_BOTTOM = 0xf3333,
  MODEL_HALF_TOP = 0xf0cccc,
  MODEL_BOTTOM = 0xF0000,
  MODEL_TOP = 0xF00000,

  MASK_NORTH = 0xf,
  MASK_WEST = 0xf000,
  MASK_EAST = 0xf00,
  MASK_SOUTH = 0xf0,
  MASK_DOWN = 0xf0000,
  MASK_UP = 0xf00000,
};

union LightingModel {
  struct {
    u32 fEmission : 4;
    u32 fBehaveAsAirWhenOpenUp : 1;
    u32 fTransparency : 3;
    u32 fModel : 24;
  };
  u32 fRaw;

  LightingModel(Transparency t) {
    fTransparency = t;
    fEmission = 0;
    fBehaveAsAirWhenOpenUp = false;
    fModel = MODEL_CLEAR;
  }

  LightingModel() {
    fTransparency = CLEAR;
    fEmission = 0;
    fBehaveAsAirWhenOpenUp = false;
    fModel = MODEL_CLEAR;
  }

  struct Hasher {
    size_t operator()(LightingModel const &k) const {
      return std::hash<u32>{}(k.fRaw);
    }
  };

  struct EqualTo {
    bool operator()(LightingModel const &a, LightingModel const &b) const {
      return a.fRaw == b.fRaw;
    }
  };
};

static_assert(sizeof(LightingModel) == sizeof(u32));

} // namespace je2be::toje::lighting
