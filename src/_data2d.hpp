#pragma once

#include <je2be/pos2.hpp>

namespace je2be {

template <class T>
class Data2d {
public:
  Data2d(Pos2i const &start, Pos2i const &end, T def)
      : fStart(start), //
        fEnd(end),
        fStorage(((size_t)(end.fX - start.fX) + 1) * ((size_t)(end.fZ - start.fZ) + 1), def) {
  }

  Data2d(Pos2i const &start, u32 width, u32 height, T def)
      : fStart(start), //
        fEnd(start.fX + (int)width - 1, start.fZ + (int)height - 1),
        fStorage((size_t)width * height, def) {
  }

  typename std::vector<T>::reference operator[](Pos2i const &p) {
    i32 dx = p.fX - fStart.fX;
    i32 dz = p.fZ - fStart.fZ;
    i32 index = dz * (fEnd.fX - fStart.fX + 1) + dx;
    return fStorage[index];
  }

  typename std::vector<T>::const_reference operator[](Pos2i const &p) const {
    i32 dx = p.fX - fStart.fX;
    i32 dz = p.fZ - fStart.fZ;
    i32 index = dz * (fEnd.fX - fStart.fX + 1) + dx;
    return fStorage[index];
  }

  std::optional<T> get(Pos2i const &p) const {
    if (fStart.fX <= p.fX && p.fX <= fEnd.fX && fStart.fZ <= p.fZ && p.fZ <= fEnd.fZ) {
      i32 dx = p.fX - fStart.fX;
      i32 dz = p.fZ - fStart.fZ;
      i32 index = dz * (fEnd.fX - fStart.fX + 1) + dx;
      return fStorage[index];
    } else {
      return std::nullopt;
    }
  }

  Pos2i const fStart;
  Pos2i const fEnd;

public:
  std::vector<T> fStorage;
};

} // namespace je2be
