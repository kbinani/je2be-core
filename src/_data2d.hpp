#pragma once

namespace je2be {

template <class T>
class Data2d {
public:
  Data2d(Pos2i const &origin, uint32_t width, uint32_t height, T def)
      : fOrigin(origin), fWidth(width), fHeight(height) {
    fStorage.resize(fWidth * fHeight, def);
  }

  std::vector<T>::reference operator[](Pos2i const &p) {
    int32_t dx = p.fX - fOrigin.fX;
    int32_t dz = p.fZ - fOrigin.fZ;
    int32_t index = (dz * fWidth) + dx;
    return fStorage[index];
  }

  std::vector<T>::const_reference operator[](Pos2i const &p) const {
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

private:
  Pos2i const fOrigin;
  uint32_t const fWidth;
  uint32_t const fHeight;
  std::vector<T> fStorage;
};

} // namespace je2be
