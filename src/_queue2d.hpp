#pragma once

#include <sparse.hpp>

#include <variant>

namespace je2be {

template <int LockRadius, bool DefaultDone, template <typename...> class Container>
class Queue2d {
  struct Element {
    float fWeight = 0;
    bool fDone = DefaultDone;
    bool fLock = false;
  };

public:
  Queue2d(Pos2i const &origin, u32 width, u32 height)
      : fOrigin(origin), fWidth(width), fHeight(height), fElements((size_t)width * height, Element()) {
  }

  struct Busy {
  };
  struct Dequeue {
    Pos2i fRegion;
  };

  std::optional<std::variant<Dequeue, Busy>> next() {
    using namespace std;
    optional<pair<Pos2i, float>> next;
    bool remaining = false;
    for (int z = 0; z < fHeight; z++) {
      for (int x = 0; x < fWidth; x++) {
        Pos2i center(fOrigin.fX + x, fOrigin.fZ + z);
        if (auto centerIndex = index(center); centerIndex) {
          Element const &element = fElements[*centerIndex];
          if (element.fDone) {
            continue;
          }
        } else {
          continue;
        }
        remaining = true;
        float sum = 0;
        bool ok = true;
        for (int dz = -LockRadius; dz <= LockRadius; dz++) {
          for (int dx = -LockRadius; dx <= LockRadius; dx++) {
            Pos2i p(center.fX + dx, center.fZ + dz);
            if (auto pIndex = index(p); pIndex) {
              Element const &element = fElements[*pIndex];
              if (element.fLock) {
                ok = false;
                break;
              }
              if (!element.fDone) {
                sum += element.fWeight;
              }
            }
          }
          if (!ok) {
            break;
          }
        }
        if (ok) {
          if (!next || next->second < sum) {
            next = make_pair(center, sum);
          }
        }
      }
    }
    if (next) {
      size_t centerIndex = *index(next->first);
      Element centerElement = fElements[centerIndex];
      centerElement.fDone = true;
      centerElement.fLock = true;
      fElements[centerIndex] = centerElement;

      int centerX = next->first.fX;
      int centerZ = next->first.fZ;
      for (int x = centerX - LockRadius; x <= centerX + LockRadius; x++) {
        for (int z = centerZ - LockRadius; z <= centerZ + LockRadius; z++) {
          if (auto idx = index(Pos2i(x, z)); idx && *idx != centerIndex) {
            Element element = fElements[*idx];
            element.fLock = true;
            fElements[*idx] = element;
          }
        }
      }
      Dequeue d;
      d.fRegion = next->first;
      return d;
    } else if (remaining) {
      return Busy();
    } else {
      return std::nullopt;
    }
  }

  void unlockAround(Pos2i const &p) {
    for (int x = p.fX - LockRadius; x <= p.fX + LockRadius; x++) {
      for (int z = p.fZ - LockRadius; z <= p.fZ + LockRadius; z++) {
        unlock({x, z});
      }
    }
  }

  void unlock(std::initializer_list<Pos2i> positions) {
    for (auto const &pos : positions) {
      unlock(pos);
    }
  }

  void unlock(Pos2i const &p) {
    if (auto idx = index(p); idx) {
      Element element = fElements[*idx];
      element.fLock = false;
      fElements[*idx] = element;
    }
  }

  void markTask(Pos2i const &p, float weight) {
    if (auto idx = index(p); idx) {
      Element element = fElements[*idx];
      element.fWeight = weight;
      element.fDone = false;
      fElements[*idx] = element;
    }
  }

private:
  std::optional<size_t> index(Pos2i const &p) const {
    if (fOrigin.fX <= p.fX && p.fX < fOrigin.fX + fWidth && fOrigin.fZ <= p.fZ && p.fZ < fOrigin.fZ + fHeight) {
      size_t dx = p.fX - fOrigin.fX;
      size_t dz = p.fZ - fOrigin.fZ;
      return dz * (fWidth) + dx;
    } else {
      return std::nullopt;
    }
  }

private:
  Pos2i const fOrigin;
  int const fWidth;
  int const fHeight;
  Container<Element> fElements;
};

} // namespace je2be
