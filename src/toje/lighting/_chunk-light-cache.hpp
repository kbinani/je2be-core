#pragma once

#include <je2be/integers.hpp>

namespace je2be::toje::lighting {

class ChunkLightCache {
public:
  template <size_t Size>
  static std::shared_ptr<ChunkLightCache> Create(int cx, int cz, Data3dSq<u8, Size> const &src) {
    int x0 = cx * 16;
    int y0 = src.fStart.fY;
    int z0 = cz * 16;
    int height = src.fEnd.fY - src.fStart.fY + 1;
    std::shared_ptr<ChunkLightCache> ret(new ChunkLightCache(cx, cz));
    ret->fNorth = std::make_shared<mcfile::Data4b3d>(Pos3i(x0, y0, z0), 16, height, 1);
    ret->fSouth = std::make_shared<mcfile::Data4b3d>(Pos3i(x0, y0, z0 + 15), 16, height, 1);
    ret->fWest = std::make_shared<mcfile::Data4b3d>(Pos3i(x0, y0, z0 + 1), 1, height, 14);
    ret->fEast = std::make_shared<mcfile::Data4b3d>(Pos3i(x0 + 15, y0, z0 + 1), 1, height, 14);
    Copy(src, *ret->fNorth);
    Copy(src, *ret->fSouth);
    Copy(src, *ret->fWest);
    Copy(src, *ret->fEast);
    return ret;
  }

  u8 get(Pos3i const &p) const {
    if (Contains(*fNorth, p)) {
      return fNorth->getUnchecked(p);
    } else if (Contains(*fEast, p)) {
      return fEast->getUnchecked(p);
    } else if (Contains(*fSouth, p)) {
      return fSouth->getUnchecked(p);
    } else if (Contains(*fWest, p)) {
      return fWest->getUnchecked(p);
    } else {
      return 0;
    }
  }

  template <size_t Size>
  void copyTo(Data3dSq<u8, Size> &dest) const {
    CopyAvailable(*fNorth, dest);
    CopyAvailable(*fEast, dest);
    CopyAvailable(*fSouth, dest);
    CopyAvailable(*fWest, dest);
  }

  static std::optional<std::pair<int, int>> Intersection(std::pair<int, int> const &a, std::pair<int, int> const &b) {
    int minend = std::min(a.second, b.second);
    if (b.first <= a.first && a.first <= b.second) {
      return std::make_pair(a.first, minend);
    } else if (a.first <= b.first && b.first <= a.second) {
      return std::make_pair(b.first, minend);
    } else {
      return std::nullopt;
    }
  }

private:
  ChunkLightCache(int cx, int cz) : fChunkX(cx), fChunkZ(cz) {}

  template <size_t Size>
  static void CopyAvailable(mcfile::Data4b3d const &src, Data3dSq<u8, Size> &dest) {
    using namespace std;
    auto xRange = Intersection(XRange(src), XRange(dest));
    if (!xRange) {
      return;
    }
    auto yRange = Intersection(YRange(src), YRange(dest));
    if (!yRange) {
      return;
    }
    auto zRange = Intersection(ZRange(src), ZRange(dest));
    if (!zRange) {
      return;
    }
    for (int y = yRange->first; y <= yRange->second; y++) {
      for (int z = zRange->first; z <= zRange->second; z++) {
        for (int x = xRange->first; x <= xRange->second; x++) {
          dest[{x, y, z}] = src.getUnchecked({x, y, z});
        }
      }
    }
  }

  static std::pair<int, int> XRange(mcfile::Data4b3d const &data) {
    return std::make_pair(data.fOrigin.fX, data.fOrigin.fX + data.fWidthX - 1);
  }

  template <size_t Size>
  static std::pair<int, int> XRange(Data3dSq<u8, Size> const &data) {
    return std::make_pair(data.fStart.fX, data.fEnd.fX);
  }

  static std::pair<int, int> YRange(mcfile::Data4b3d const &data) {
    return std::make_pair(data.fOrigin.fY, data.fOrigin.fY + data.fHeight - 1);
  }

  template <size_t Size>
  static std::pair<int, int> YRange(Data3dSq<u8, Size> const &data) {
    return std::make_pair(data.fStart.fY, data.fEnd.fY);
  }

  static std::pair<int, int> ZRange(mcfile::Data4b3d const &data) {
    return std::make_pair(data.fOrigin.fZ, data.fOrigin.fZ + data.fWidthZ - 1);
  }

  template <size_t Size>
  static std::pair<int, int> ZRange(Data3dSq<u8, Size> const &data) {
    return std::make_pair(data.fStart.fZ, data.fEnd.fZ);
  }

  static bool Contains(mcfile::Data4b3d const &data, Pos3i const &p) {
    return data.fOrigin.fX <= p.fX && p.fX < data.fOrigin.fX + data.fWidthX &&
           data.fOrigin.fY <= p.fY && p.fY < data.fOrigin.fY + data.fHeight &&
           data.fOrigin.fZ <= p.fZ && p.fZ < data.fOrigin.fZ + data.fWidthZ;
  }

  template <size_t Size>
  static void Copy(Data3dSq<u8, Size> const &src, mcfile::Data4b3d &dest) {
    for (int y = dest.fOrigin.fY; y < dest.fOrigin.fY + dest.fHeight; y++) {
      for (int z = dest.fOrigin.fZ; z < dest.fOrigin.fZ + dest.fWidthZ; z++) {
        for (int x = dest.fOrigin.fX; x < dest.fOrigin.fX + dest.fWidthX; x++) {
          dest.setUnchecked({x, y, z}, src[{x, y, z}]);
        }
      }
    }
  }

public:
  int const fChunkX;
  int const fChunkZ;

private:
  std::shared_ptr<mcfile::Data4b3d> fNorth;
  std::shared_ptr<mcfile::Data4b3d> fEast;
  std::shared_ptr<mcfile::Data4b3d> fSouth;
  std::shared_ptr<mcfile::Data4b3d> fWest;
};

} // namespace je2be::toje::lighting
