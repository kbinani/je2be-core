#pragma once

#include <cstdint>

namespace je2be {

struct Rgba {
  constexpr Rgba(u8 r, u8 g, u8 b) : fR(r), fG(g), fB(b), fA(255) {}
  constexpr Rgba(u8 r, u8 g, u8 b, u8 a) : fR(r), fG(g), fB(b), fA(a) {}

  u8 fR;
  u8 fG;
  u8 fB;
  u8 fA;

  i32 toRGB() const {
    u32 c = ((u32)fR << 16) | ((u32)fG << 8) | ((u32)fB);
    return *(i32 *)&c;
  }

  i32 toARGB() const {
    u32 c = ((u32)fA << 24) | ((u32)fR << 16) | ((u32)fG << 8) | ((u32)fB);
    return *(i32 *)&c;
  }

  static Rgba FromRGB(i32 rgb) {
    u32 u = *(u32 *)&rgb;
    u8 r = 0xff & (u >> 16);
    u8 g = 0xff & (u >> 8);
    u8 b = 0xff & u;
    return Rgba(r, g, b);
  }
};

} // namespace je2be
