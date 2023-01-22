#pragma once

namespace je2be {

template <class T>
class Data2d {
public:
  Data2d(Pos2i const &origin, uint32_t width, uint32_t height, T def)
      : fOrigin(origin), fStart(origin), fWidth(width), fHeight(height), fEnd(origin.fX + (int)width - 1, origin.fZ + (int)height - 1) {
    fStorage.resize(fWidth * fHeight, def);
  }

  typename std::vector<T>::reference operator[](Pos2i const &p) {
    int32_t dx = p.fX - fOrigin.fX;
    int32_t dz = p.fZ - fOrigin.fZ;
    int32_t index = (dz * fWidth) + dx;
    return fStorage[index];
  }

  typename std::vector<T>::const_reference operator[](Pos2i const &p) const {
    int32_t dx = p.fX - fOrigin.fX;
    int32_t dz = p.fZ - fOrigin.fZ;
    int32_t index = (dz * fWidth) + dx;
    return fStorage[index];
  }

  std::optional<T> get(Pos2i const &p) const {
    int32_t dx = p.fX - fOrigin.fX;
    int32_t dz = p.fZ - fOrigin.fZ;
    if (0 <= dx && dx < fWidth && 0 <= dz && dz < fHeight) {
      int32_t index = (dz * fWidth) + dx;
      return fStorage[index];
    } else {
      return std::nullopt;
    }
  }

  [[deprecated]] Pos2i const fOrigin;
  Pos2i const fStart;
  [[deprecated]] uint32_t const fWidth;
  [[deprecated]] uint32_t const fHeight;
  Pos2i const fEnd;

public:
  std::vector<T> fStorage;
};

} // namespace je2be
