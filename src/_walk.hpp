#pragma once

namespace je2be {

class Walk {
  Walk() = delete;

public:
  static bool Spiral(Pos2i const &northWest, Pos2i const &southEast, std::function<bool(Pos2i const &)> action) {
    int x0 = northWest.fX;
    int z0 = northWest.fZ;
    int x1 = southEast.fX;
    int z1 = southEast.fZ;
    for (int x = x0; x <= x1; x++) {
      if (!action({x, z0})) {
        return false;
      }
    }
    for (int z = z0 + 1; z <= z1; z++) {
      if (!action({x1, z})) {
        return false;
      }
    }
    for (int x = x1 - 1; x >= x0; x--) {
      if (!action({x, z1})) {
        return false;
      }
    }
    for (int z = z1 - 1; z >= z0 + 1; z--) {
      if (!action({x0, z})) {
        return false;
      }
    }
    return true;
  }
};

} // namespace je2be
