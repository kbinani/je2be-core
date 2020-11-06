#pragma once

namespace j2b {

struct Rgba {
  Rgba(uint8_t r, uint8_t g, uint8_t b) : fR(r), fG(g), fB(b), fA(255) {}
  Rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
      : fR(r), fG(g), fB(b), fA(a) {}

  uint8_t fR;
  uint8_t fG;
  uint8_t fB;
  uint8_t fA;

  int32_t toRGB() const {
    uint32_t c = ((uint32_t)fR << 16) | ((uint32_t)fG << 8) | ((uint32_t)fB);
    return *(int32_t *)&c;
  }

  int32_t toARGB() const {
    uint32_t c = ((uint32_t)fA << 16) | ((uint32_t)fR << 16) |
                 ((uint32_t)fG << 8) | ((uint32_t)fB);
    return *(int32_t *)&c;
  }

  static Rgba FromRGB(int32_t rgb) {
    uint32_t u = *(uint32_t *)&rgb;
    uint8_t r = 0xff & (u >> 16);
    uint8_t g = 0xff & (u >> 8);
    uint8_t b = 0xff & u;
    return Rgba(r, g, b);
  }
};

} // namespace j2b
