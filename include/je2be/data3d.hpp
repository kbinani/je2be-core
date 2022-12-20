#pragma once

#include <je2be/pos3.hpp>

namespace je2be {

template <class Value>
class Data3D {
public:
  Data3D(Pos3i start, Pos3i end, Value def) : fStart(start), fEnd(end) {
    int dx = end.fX - start.fX + 1;
    int dy = end.fY - start.fY + 1;
    int dz = end.fZ - start.fZ + 1;
    assert(dx > 0 && dy > 0 && dz > 0);
    fStorage.resize(dx * dy * dz, def);
  }

  std::optional<Value> get(Pos3i p) const {
    int i = index(p);
    if (0 <= i && i < fStorage.size()) {
      return fStorage[i];
    } else {
      return std::nullopt;
    }
  }

  void set(Pos3i p, Value v) {
    int i = index(p);
    if (0 <= i && i < fStorage.size()) {
      fStorage[i] = v;
    }
  }

  void applying(std::function<Value(Pos3i p, Value const &v, Data3D<Value> const &self)> action) {
    int idx = 0;
    for (int y = fStart.fY; y <= fEnd.fY; y++) {
      for (int z = fStart.fZ; z <= fEnd.fZ; z++) {
        for (int x = fStart.fX; x <= fEnd.fX; x++, idx++) {
          fStorage[idx] = action({x, y, z}, fStorage[idx], *this);
        }
      }
    }
  }

private:
  int index(Pos3i p) const {
    int x = p.fX - fStart.fX;
    int y = p.fY - fStart.fY;
    int z = p.fZ - fStart.fZ;
    if (x < 0 || y < 0 || z < 0) {
      return -1;
    }
    int dx = fEnd.fX - fStart.fX + 1;
    int dy = fEnd.fY - fStart.fY + 1;
    int dz = fEnd.fZ - fStart.fZ + 1;
    if (x >= dx || y >= dy || z >= dz) {
      return -1;
    }
    return (y * dz + z) * dx + x;
  }

private:
  Pos3i const fStart;
  Pos3i const fEnd;
  std::vector<Value> fStorage;
};

} // namespace je2be
