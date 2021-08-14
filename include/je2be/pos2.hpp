#pragma once

namespace j2b {

class Pos2 {
public:
  Pos2(int x, int z) : fX(x), fZ(z) {}

  bool operator==(Pos2 const &other) const { return fX == other.fX && fZ == other.fZ; }

  static double DistanceSquare(Pos2 const &a, Pos2 const &b) {
    double dx = a.fX - b.fX;
    double dz = a.fZ - b.fZ;
    return dx * dx + dz * dz;
  }

public:
  int fX;
  int fZ;
};

class Pos2Hasher {
public:
  size_t operator()(Pos2 const &k) const {
    size_t res = 17;
    res = res * 31 + std::hash<int>{}(k.fX);
    res = res * 31 + std::hash<int>{}(k.fZ);
    return res;
  }
};

} // namespace j2b
