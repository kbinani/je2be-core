#pragma once

namespace je2be {
using Pos2i = mcfile::Pos2i;
using Pos2iHasher = mcfile::Pos2iHasher;

static inline Pos2i Right90(Pos2i vec) {
  constexpr int cos90 = 0;
  constexpr int sin90 = 1;
  int x = cos90 * vec.fX - sin90 * vec.fZ;
  int z = sin90 * vec.fX + cos90 * vec.fZ;
  return Pos2i(x, z);
}

static inline Pos2i Left90(Pos2i vec) {
  constexpr int cosM90 = 0;
  constexpr int sinM90 = -1;
  int x = cosM90 * vec.fX - sinM90 * vec.fZ;
  int z = sinM90 * vec.fX + cosM90 * vec.fZ;
  return Pos2i(x, z);
}

static inline bool IsOrthogonal(Pos2i a, Pos2i b) {
  int innerProduct = a.fX * b.fX + a.fZ * b.fZ;
  return innerProduct == 0;
}
} // namespace je2be
