#pragma once

#include "_mem.hpp"
#include "_pos3.hpp"
#include "_volume.hpp"

namespace je2be {

template <class Value, size_t Align = 64>
class Data3d {
public:
  Data3d(Pos3i const &start, Pos3i const &end, Value def) : fStart(start), fEnd(end) {
    int dx = end.fX - start.fX + 1;
    int dy = end.fY - start.fY + 1;
    int dz = end.fZ - start.fZ + 1;
    assert(dx > 0 && dy > 0 && dz > 0);
    fStorage = (Value *)Mem::AllocAligned<Align>(sizeof(Value) * dx * dy * dz);
    if (fStorage == nullptr) {
      throw std::bad_alloc();
    }
    std::fill_n(fStorage, dx * dy * dz, def);
  }

  ~Data3d() {
    Mem::FreeAligned(fStorage);
  }

  Value const &operator[](Pos3i const &p) const {
    return fStorage[index(p)];
  }

  Value &operator[](Pos3i const &p) {
    return fStorage[index(p)];
  }

  std::optional<Value> get(Pos3i const &p) const {
    int i = index(p);
    if (0 <= i) {
      return fStorage[i];
    } else {
      return std::nullopt;
    }
  }

  void set(Pos3i const &p, Value v) {
    int i = index(p);
    if (0 <= i) {
      fStorage[i] = v;
    }
  }

  void fill(Value v) {
    int dx = fEnd.fX - fStart.fX + 1;
    int dy = fEnd.fY - fStart.fY + 1;
    int dz = fEnd.fZ - fStart.fZ + 1;
    std::fill_n(fStorage, dx * dy * dz, v);
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
  Value *fStorage;
};

template <class Value, size_t Size, size_t Align = 64>
class Data3dSq {
public:
  Data3dSq(Pos3i const &start, size_t height, Value def) : fStart(start), fEnd(start.fX + (int)Size - 1, start.fY + (int)height - 1, start.fZ + (int)Size - 1), fHeight(height) {
    fStorage = (Value *)Mem::AllocAligned<Align>(sizeof(Value) * Size * height * Size);
    if (fStorage == nullptr) {
      throw std::bad_alloc();
    }
    std::fill_n(fStorage, Size * height * Size, def);
  }

  ~Data3dSq() {
    Mem::FreeAligned(fStorage);
  }

  Value const &operator[](Pos3i const &p) const {
    int i = index(p);
    assert(0 <= i && i < (int)Size * fHeight * (int)Size);
    return fStorage[i];
  }

  Value &operator[](Pos3i const &p) {
    int i = index(p);
    assert(0 <= i && i < (int)Size * fHeight * (int)Size);
    return fStorage[i];
  }

  std::optional<Value> get(Pos3i const &p) const {
    int i = index(p);
    if (0 <= i && i < Size * fHeight * Size) {
      return fStorage[i];
    } else {
      return std::nullopt;
    }
  }

  void set(Pos3i const &p, Value v) {
    int i = index(p);
    if (0 <= i && i < Size * fHeight * Size) {
      fStorage[i] = v;
    }
  }

  void fill(Value v) {
    std::fill(fStorage, fStorage + Size * fHeight * Size, v);
  }

  template <size_t SizeOther, size_t AlignOther>
  void copyFrom(Data3dSq<Value, SizeOther, AlignOther> const &other) {
    Volume vThis(fStart, fEnd);
    Volume vOther(other.fStart, other.fEnd);
    auto intersection = Volume::Intersection(vThis, vOther);
    if (!intersection) {
      return;
    }
    for (int y = intersection->fStart.fY; y <= intersection->fEnd.fY; y++) {
      for (int z = intersection->fStart.fZ; z <= intersection->fEnd.fZ; z++) {
        for (int x = intersection->fStart.fX; x <= intersection->fEnd.fX; x++) {
          (*this)[{x, y, z}] = other[{x, y, z}];
        }
      }
    }
  }

  Volume volume() const {
    return Volume(fStart, fEnd);
  }

private:
  int index(Pos3i const &p) const {
    int x = p.fX - fStart.fX;
    int y = p.fY - fStart.fY;
    int z = p.fZ - fStart.fZ;
    assert(x >= 0 && y >= 0 && z >= 0);
    assert(x < Size && y < fHeight && z < Size);
    return (y * Size + z) * Size + x;
  }

public:
  Pos3i const fStart;
  Pos3i const fEnd;

private:
  size_t const fHeight;
  Value *fStorage;
};

} // namespace je2be
