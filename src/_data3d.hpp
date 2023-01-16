#pragma once

#include "_pos3.hpp"

namespace je2be {

template <class Value>
class Data3d {
public:
  Data3d(Pos3i const &start, Pos3i const &end, Value def) : fStart(start), fEnd(end) {
    int dx = end.fX - start.fX + 1;
    int dy = end.fY - start.fY + 1;
    int dz = end.fZ - start.fZ + 1;
    assert(dx > 0 && dy > 0 && dz > 0);
    fStorage.resize((size_t)dx * (size_t)dy * (size_t)dz, def);
  }

  Value const &operator[](Pos3i const &p) const {
    return fStorage[index(p)];
  }

  Value &operator[](Pos3i const &p) {
    return fStorage[index(p)];
  }

  std::optional<Value> get(Pos3i const &p) const {
    int i = index(p);
    if (0 <= i && i < fStorage.size()) {
      return fStorage[i];
    } else {
      return std::nullopt;
    }
  }

  void set(Pos3i const &p, Value v) {
    int i = index(p);
    if (0 <= i && i < fStorage.size()) {
      fStorage[i] = v;
    }
  }

  void fill(Value v) {
    std::fill(fStorage.begin(), fStorage.end(), v);
  }

  typename std::vector<Value>::const_iterator cbegin() const {
    return fStorage.cbegin();
  }

  typename std::vector<Value>::const_iterator cend() const {
    return fStorage.cend();
  }

private:
  int index(Pos3i const &p) const {
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

public:
  Pos3i const fStart;
  Pos3i const fEnd;

private:
  std::vector<Value> fStorage;
};

} // namespace je2be
