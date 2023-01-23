#pragma once

namespace je2be {

template <class T>
class Data2d {
public:
  Data2d(Pos2i const &origin, uint32_t width, uint32_t height, T def)
      : fStart(origin), //
        fEnd(origin.fX + (int)width - 1, origin.fZ + (int)height - 1) {
    fStorage.resize(width * height, def);
  }

  typename std::vector<T>::reference operator[](Pos2i const &p) {
    int32_t dx = p.fX - fStart.fX;
    int32_t dz = p.fZ - fStart.fZ;
    int32_t index = dz * (fEnd.fX - fStart.fX + 1) + dx;
    return fStorage[index];
  }

  typename std::vector<T>::const_reference operator[](Pos2i const &p) const {
    int32_t dx = p.fX - fStart.fX;
    int32_t dz = p.fZ - fStart.fZ;
    int32_t index = dz * (fEnd.fX - fStart.fX + 1) + dx;
    return fStorage[index];
  }

  std::optional<T> get(Pos2i const &p) const {
    if (fStart.fX <= p.fX && p.fX <= fEnd.fX && fStart.fZ <= p.fZ && p.fZ <= fEnd.fZ) {
      int32_t dx = p.fX - fStart.fX;
      int32_t dz = p.fZ - fStart.fZ;
      int32_t index = dz * (fEnd.fX - fStart.fX + 1) + dx;
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
