#pragma once

#include "_data2d.hpp"

namespace je2be {

class Queue2d {
public:
  Queue2d(Pos2i const &origin, uint32_t width, uint32_t height, uint32_t radius = 0)
      : fOrigin(origin), fWidth(width), fHeight(height), fRadius(radius), fDone(origin, width, height, false), fUse(origin, width, height, false) {
  }

  std::optional<Pos2i> next() {
    std::lock_guard<std::mutex> lock(fMut);
    for (int centerX = fOrigin.fX; centerX < fOrigin.fX + fWidth; centerX++) {
      for (int centerZ = fOrigin.fZ; centerZ < fOrigin.fZ + fHeight; centerZ++) {
        if (fDone[{centerX, centerZ}]) {
          continue;
        }
        bool found = true;
        for (int x = (std::max)(centerX - fRadius, fOrigin.fX); x <= (std::min)(centerX + fRadius, fOrigin.fX + fWidth - 1); x++) {
          for (int z = (std::max)(centerZ - fRadius, fOrigin.fZ); z <= (std::min)(centerZ + fRadius, fOrigin.fZ + fHeight - 1); z++) {
            if (fUse[{x, z}]) {
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
          for (int x = (std::max)(centerX - fRadius, fOrigin.fX); x <= (std::min)(centerX + fRadius, fOrigin.fX + fWidth - 1); x++) {
            for (int z = (std::max)(centerZ - fRadius, fOrigin.fZ); z <= (std::min)(centerZ + fRadius, fOrigin.fZ + fHeight - 1); z++) {
              fUse[{x, z}] = true;
            }
          }
          return next;
        }
      }
    }
    return std::nullopt;
  }

  void finish(Pos2i const &p) {
    std::lock_guard<std::mutex> lock(fMut);
    for (int x = (std::max)(p.fX - fRadius, fOrigin.fX); x <= (std::min)(p.fX + fRadius, fOrigin.fX + fWidth - 1); x++) {
      for (int z = (std::max)(p.fZ - fRadius, fOrigin.fZ); z <= (std::min)(p.fZ + fRadius, fOrigin.fZ + fHeight - 1); z++) {
        fUse[{x, z}] = false;
      }
    }
  }

  void unsafeSetDone(Pos2i const &p, bool done) {
    fDone[p] = done;
  }

private:
  std::mutex fMut;
  Pos2i const fOrigin;
  int const fWidth;
  int const fHeight;
  int const fRadius;
  Data2d<bool> fDone;
  Data2d<bool> fUse;
};

} // namespace je2be
