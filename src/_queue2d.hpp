#pragma once

#include "_data2d.hpp"

#include <deque>

namespace je2be {

class Queue2d {
public:
  Queue2d(Pos2i const &origin, uint32_t width, uint32_t height, uint32_t lockRadius = 0)
      : fOrigin(origin), fWidth(width), fHeight(height), fLockRadius(lockRadius), fDone(origin, width, height, false), fLock(origin, width, height, false), fWeight(origin, width, height, 0.0f) {
  }

  std::optional<Pos2i> next() {
    using namespace std;
    optional<pair<Pos2i, float>> next;
    for (int z = 0; z < fHeight; z++) {
      for (int x = 0; x < fWidth; x++) {
        Pos2i center(fOrigin.fX + x, fOrigin.fZ + z);
        if (fDone[center]) {
          continue;
        }
        float sum = 0;
        bool ok = true;
        for (int dz = -fLockRadius; dz <= fLockRadius; dz++) {
          for (int dx = -fLockRadius; dx <= fLockRadius; dx++) {
            Pos2i p{center.fX + dx, center.fZ + dz};
            if (auto v = fWeight.get(p); v) {
              if (fLock[p]) {
                ok = false;
                break;
              }
              if (!fDone[p]) {
                sum += *v;
              }
            }
          }
          if (!ok) {
            break;
          }
        }
        if (next) {
          if (next->second < sum) {
            next = make_pair(center, sum);
          }
        } else {
          next = make_pair(center, sum);
        }
      }
    }
    if (next) {
      fDone[next->first] = true;
      int centerX = next->first.fX;
      int centerZ = next->first.fZ;
      for (int x = (std::max)(centerX - fLockRadius, fOrigin.fX); x <= (std::min)(centerX + fLockRadius, fOrigin.fX + fWidth - 1); x++) {
        for (int z = (std::max)(centerZ - fLockRadius, fOrigin.fZ); z <= (std::min)(centerZ + fLockRadius, fOrigin.fZ + fHeight - 1); z++) {
          fLock[{x, z}] = true;
        }
      }
      return next->first;
    } else {
      return std::nullopt;
    }
  }

  void unlockAround(Pos2i const &p) {
    for (int x = p.fX - fLockRadius; x <= p.fX + fLockRadius; x++) {
      for (int z = p.fZ - fLockRadius; z <= p.fZ + fLockRadius; z++) {
        unlock({x, z});
      }
    }
  }

  void unlock(std::initializer_list<Pos2i> positions) {
    for (auto const &pos : positions) {
      unlock(pos);
    }
  }

  void setDone(Pos2i const &p, bool done) {
    fDone[p] = done;
  }

  void unlock(Pos2i const &p) {
    if (fOrigin.fX <= p.fX && p.fX < fOrigin.fX + fWidth && fOrigin.fZ <= p.fZ && p.fZ < fOrigin.fZ + fHeight) {
      fLock[p] = false;
    }
  }

  void setWeight(Pos2i const &p, float weight) {
    if (fOrigin.fX <= p.fX && p.fX < fOrigin.fX + fWidth && fOrigin.fZ <= p.fZ && p.fZ < fOrigin.fZ + fHeight) {
      fWeight[p] = weight;
    }
  }

private:
  Pos2i const fOrigin;
  int const fWidth;
  int const fHeight;
  int const fLockRadius;
  Data2d<bool> fDone;
  Data2d<bool> fLock;
  Data2d<float> fWeight;
};

} // namespace je2be
