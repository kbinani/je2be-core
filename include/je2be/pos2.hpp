#pragma once

#include <minecraft-file.hpp>

namespace je2be {
using Pos2i = mcfile::Pos2i;
using Pos2iHasher = mcfile::Pos2iHasher;

using Pos2d = mcfile::detail::Pos2<double>;

static inline Pos2i Right90(Pos2i const &vec) {
  constexpr int cos90 = 0;
  constexpr int sin90 = 1;
  int x = cos90 * vec.fX - sin90 * vec.fZ;
  int z = sin90 * vec.fX + cos90 * vec.fZ;
  return Pos2i(x, z);
}

static inline Pos2i Left90(Pos2i const &vec) {
  constexpr int cosM90 = 0;
  constexpr int sinM90 = -1;
  int x = cosM90 * vec.fX - sinM90 * vec.fZ;
  int z = sinM90 * vec.fX + cosM90 * vec.fZ;
  return Pos2i(x, z);
}

static inline bool IsOrthogonal(Pos2i const &a, Pos2i const &b) {
  if (a.fX == 0 && a.fZ == 0) {
    return false;
  }
  if (b.fX == 0 && b.fZ == 0) {
    return false;
  }
  int innerProduct = a.fX * b.fX + a.fZ * b.fZ;
  return innerProduct == 0;
}

static inline bool IsParallel(Pos2i const &a, Pos2i const &b) {
  auto rot = Right90(b);
  return IsOrthogonal(a, rot);
}
} // namespace je2be
