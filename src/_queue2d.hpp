#pragma once

#include "_data2d.hpp"

namespace je2be {

class Queue2d {
public:
  Queue2d(Pos2i const &origin, uint32_t width, uint32_t height, uint32_t lockRadius = 0)
      : fOrigin(origin), fWidth(width), fHeight(height), fLockRadius(lockRadius), fDone(origin, width, height, false), fLock(origin, width, height, false) {
  }

  std::optional<Pos2i> next() {
    for (int centerX = fOrigin.fX; centerX < fOrigin.fX + fWidth; centerX++) {
      for (int centerZ = fOrigin.fZ; centerZ < fOrigin.fZ + fHeight; centerZ++) {
        if (fDone[{centerX, centerZ}]) {
          continue;
        }
        bool found = true;
        for (int x = (std::max)(centerX - fLockRadius, fOrigin.fX); x <= (std::min)(centerX + fLockRadius, fOrigin.fX + fWidth - 1); x++) {
          for (int z = (std::max)(centerZ - fLockRadius, fOrigin.fZ); z <= (std::min)(centerZ + fLockRadius, fOrigin.fZ + fHeight - 1); z++) {
            if (fLock[{x, z}]) {
              found = false;
              break;
            }
          }
          if (!found) {
            break;
          }
        }
        if (found) {
          Pos2i next(centerX, centerZ);
          fDone[next] = true;
          for (int x = (std::max)(centerX - fLockRadius, fOrigin.fX); x <= (std::min)(centerX + fLockRadius, fOrigin.fX + fWidth - 1); x++) {
            for (int z = (std::max)(centerZ - fLockRadius, fOrigin.fZ); z <= (std::min)(centerZ + fLockRadius, fOrigin.fZ + fHeight - 1); z++) {
              fLock[{x, z}] = true;
            }
          }
          return next;
        }
      }
    }
    return std::nullopt;
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

private:
  Pos2i const fOrigin;
  int const fWidth;
  int const fHeight;
  int const fLockRadius;
  Data2d<bool> fDone;
  Data2d<bool> fLock;
};

} // namespace je2be
