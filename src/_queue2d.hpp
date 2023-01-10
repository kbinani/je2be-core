#pragma once

#include "_data2d.hpp"

#include <deque>

namespace je2be {

class Queue2d {
public:
  Queue2d(Pos2i const &origin, uint32_t width, uint32_t height, uint32_t lockRadius = 0)
      : fOrigin(origin), fWidth(width), fHeight(height), fLockRadius(lockRadius), fDone(origin, width, height, false), fLock(origin, width, height, false) {
    for (int centerX = fOrigin.fX; centerX < fOrigin.fX + fWidth; centerX++) {
      for (int centerZ = fOrigin.fZ; centerZ < fOrigin.fZ + fHeight; centerZ++) {
        fWeight.push_back(std::make_pair(Pos2i(centerX, centerZ), 0.0f));
      }
    }
    fWeightSorted = true;
  }

  std::optional<Pos2i> next() {
    if (!fWeightSorted) {
      std::sort(fWeight.begin(), fWeight.end(), [](auto const &a, auto const &b) {
        return a.second > b.second;
      });
      fWeightSorted = true;
    }
    for (auto it = fWeight.begin(); it != fWeight.end(); ++it) {
      int centerX = it->first.fX;
      int centerZ = it->first.fZ;
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
        fWeight.erase(it);
        return next;
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

  void setWeight(Pos2i const &p, float weight) {
    if (fOrigin.fX <= p.fX && p.fX < fOrigin.fX + fWidth && fOrigin.fZ <= p.fZ && p.fZ < fOrigin.fZ + fHeight) {
      auto found = std::find_if(fWeight.begin(), fWeight.end(), [p](auto const &it) { return it.first == p; });
      if (found == fWeight.end()) {
        fWeight.push_back(std::make_pair(p, weight));
      } else {
        found->second = weight;
      }
      fWeightSorted = false;
    }
  }

private:
  Pos2i const fOrigin;
  int const fWidth;
  int const fHeight;
  int const fLockRadius;
  Data2d<bool> fDone;
  Data2d<bool> fLock;
  std::deque<std::pair<Pos2i, float>> fWeight;
  bool fWeightSorted = false;
};

} // namespace je2be
