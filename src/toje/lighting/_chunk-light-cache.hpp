#pragma once

namespace je2be::toje::lighting {

class ChunkLightCache {
public:
  ChunkLightCache(mcfile::Dimension dim, int cx, int cz, mcfile::Data4b3dView const &src) : fDim(dim), fChunkX(cx), fChunkZ(cz) {
    using namespace std;
    int y0 = 0;
    int height = 256;
    if (dim == mcfile::Dimension::Overworld) {
      y0 = -64;
      height = 384;
    }
    int x0 = cx * 16;
    int z0 = cz * 16;
    fNorth = std::make_shared<mcfile::Data4b3d>(Pos3i(x0, y0, z0), 16, height, 1);
    fSouth = std::make_shared<mcfile::Data4b3d>(Pos3i(x0, y0, z0 + 15), 16, height, 1);
    fWest = std::make_shared<mcfile::Data4b3d>(Pos3i(x0, y0, z0 + 1), 1, height, 14);
    fEast = std::make_shared<mcfile::Data4b3d>(Pos3i(x0 + 15, y0, z0 + 1), 1, height, 14);
    Copy(src, *fNorth);
    Copy(src, *fSouth);
    Copy(src, *fWest);
    Copy(src, *fEast);
  }

  uint8_t get(Pos3i const &p) const {
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

  void copyTo(mcfile::Data4b3dView &dest) const {
    CopyAvailable(*fNorth, dest);
    CopyAvailable(*fEast, dest);
    CopyAvailable(*fSouth, dest);
    CopyAvailable(*fWest, dest);
  }

private:
  static void CopyAvailable(mcfile::Data4b3dView const &src, mcfile::Data4b3dView &dest) {
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
          dest.setUnchecked({x, y, z}, src.getUnchecked({x, y, z}));
        }
      }
    }
  }

  static std::pair<int, int> XRange(mcfile::Data4b3dView const &data) {
    return std::make_pair(data.fOrigin.fX, data.fOrigin.fX + data.fWidthX - 1);
  }

  static std::pair<int, int> YRange(mcfile::Data4b3dView const &data) {
    return std::make_pair(data.fOrigin.fY, data.fOrigin.fY + data.fHeight - 1);
  }

  static std::pair<int, int> ZRange(mcfile::Data4b3dView const &data) {
    return std::make_pair(data.fOrigin.fZ, data.fOrigin.fZ + data.fWidthZ - 1);
  }

  static std::optional<std::pair<int, int>> Intersection(std::pair<int, int> const &a, std::pair<int, int> const &b) {
    int max1 = a.second;
    int max2 = b.second;
    int minend = std::min(a.second, b.second);
    if (b.first <= a.first && a.first <= b.second) {
      return std::make_pair(a.first, minend);
    } else if (a.first <= b.first && b.first <= a.second) {
      return std::make_pair(b.first, minend);
    } else {
      return std::nullopt;
    }
  }

  static bool Contains(mcfile::Data4b3dView const &data, Pos3i const &p) {
    return data.fOrigin.fX <= p.fX && p.fX < data.fOrigin.fX + data.fWidthX &&
           data.fOrigin.fY <= p.fY && p.fY < data.fOrigin.fY + data.fHeight &&
           data.fOrigin.fZ <= p.fZ && p.fZ < data.fOrigin.fZ + data.fWidthZ;
  }

  static void Copy(mcfile::Data4b3dView const &src, mcfile::Data4b3dView &dest) {
    for (int y = dest.fOrigin.fY; y < dest.fOrigin.fY + dest.fHeight; y++) {
      for (int z = dest.fOrigin.fZ; z < dest.fOrigin.fZ + dest.fWidthZ; z++) {
        for (int x = dest.fOrigin.fX; x < dest.fOrigin.fX + dest.fWidthX; x++) {
          dest.setUnchecked({x, y, z}, src.getUnchecked({x, y, z}));
        }
      }
    }
  }

public:
  mcfile::Dimension const fDim;
  int const fChunkX;
  int const fChunkZ;

private:
  std::shared_ptr<mcfile::Data4b3d> fNorth;
  std::shared_ptr<mcfile::Data4b3d> fEast;
  std::shared_ptr<mcfile::Data4b3d> fSouth;
  std::shared_ptr<mcfile::Data4b3d> fWest;
};

} // namespace je2be::toje::lighting
