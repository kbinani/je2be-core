#pragma once

namespace j2b {

class Pos3 {
public:
  Pos3(int x, int y, int z) : fX(x), fZ(z), fY(y) {}

  bool operator==(Pos3 const &other) const { return fX == other.fX && fZ == other.fZ && fY == other.fY; }

  static double DistanceSquare(Pos3 const &a, Pos3 const &b) {
    double dx = a.fX - b.fX;
    double dy = a.fY - b.fY;
    double dz = a.fZ - b.fZ;
    return dx * dx + dy * dy + dz * dz;
  }

public:
  int fX;
  int fZ;
  int fY;
};

class Pos3Hasher {
public:
  size_t operator()(Pos3 const &k) const {
    size_t res = 17;
    res = res * 31 + std::hash<int>{}(k.fX);
    res = res * 31 + std::hash<int>{}(k.fY);
    res = res * 31 + std::hash<int>{}(k.fZ);
    return res;
  }
};

} // namespace j2b
