#pragma once

#include <minecraft-file.hpp>

#include "_data2d.hpp"
#include "_data3d.hpp"
#include "_optional.hpp"
#include "terraform/_block-property-accessor.hpp"
#include "terraform/java/_block-accessor-java.hpp"
#include "toje/terraform/_lighting-property.hpp"

namespace je2be::toje::lighting {

struct LightingModel {
  uint32_t fModel = 0;
  uint8_t fEmission = 0;
  bool fBehaveAsAirWhenOpenUp = false;
  Transparency fTransparency = CLEAR;
};

class LightCache {
public:
  LightCache(int rx, int rz)
      : fRx(rx), fRz(rz), fProperties({rx * 32 - 1, rz * 32 - 1}, 34, 34, nullptr), fModels({rx * 32 - 1, rz * 32 - 1}, 34, 34, nullptr) {}

  std::shared_ptr<Data3dSq<LightingProperty, 16>> getProperty(int cx, int cz) {
    return fProperties[{cx, cz}];
  }

  void setProperty(int cx, int cz, std::shared_ptr<Data3dSq<LightingProperty, 16>> const &data) {
    fProperties[{cx, cz}] = data;
  }

  std::shared_ptr<Data3dSq<LightingModel, 16>> getModel(int cx, int cz) {
    return fModels[{cx, cz}];
  }

  void setModel(int cx, int cz, std::shared_ptr<Data3dSq<LightingModel, 16>> const &data) {
    fModels[{cx, cz}] = data;
  }

private:
  int const fRx;
  int const fRz;
  Data2d<std::shared_ptr<Data3dSq<LightingProperty, 16>>> fProperties;
  Data2d<std::shared_ptr<Data3dSq<LightingModel, 16>>> fModels;
};

class Lighting {
  enum Model : uint32_t {
    // https://gyazo.com/a270e426c0eb2b8b53083317b5aab16f
    MODEL_SOLID = 0xffffff,
    MODEL_CLEAR = 0,
    MODEL_HALF_BOTTOM = 0xf3333,
    MODEL_HALF_TOP = 0xf0cccc,

    MASK_NORTH = 0xf,
    MASK_WEST = 0xf0,
    MASK_EAST = 0xf00,
    MASK_SOUTH = 0xf000,
    MASK_DOWN = 0xf0000,
    MASK_UP = 0xf00000,
  };

public:
  static void Do(mcfile::Dimension dim, mcfile::je::Chunk &out, terraform::java::BlockAccessorJava &blockAccessor, LightCache &cache) {
    using namespace std;
    using namespace mcfile;

    int const minBlockY = dim == mcfile::Dimension::Overworld ? -80 : 0;
    int const maxBlockY = dim == mcfile::Dimension::Overworld ? 319 : 255;

    Pos3i start(out.minBlockX() - 14, minBlockY, out.minBlockZ() - 14);
    size_t const height = maxBlockY - minBlockY + 2;

#if 0
    LightingProperty air(CLEAR);
    Data3dSq<LightingProperty, 44> props(start, height, air);
    EnsureLightingProperties(cache, props, out.fChunkX, out.fChunkZ, blockAccessor);
#else
    LightingModel airModel;
    airModel.fTransparency = CLEAR;
    airModel.fModel = MODEL_CLEAR;
    airModel.fEmission = 0;

    Data3dSq<LightingModel, 44> models(start, height, airModel);
    CopyChunkLightingModel(out, models);
    for (int dz = -1; dz <= 1; dz++) {
      for (int dx = -1; dx <= 1; dx++) {
        if (dx == 0 && dz == 0) {
          continue;
        }
        auto chunk = blockAccessor.chunkAt(out.fChunkX + dx, out.fChunkZ + dz);
        if (chunk) {
          assert(minBlockY <= chunk->minBlockY() && chunk->maxBlockY() <= maxBlockY);
          CopyChunkLightingModel(*chunk, models);
        }
      }
    }

    LightingProperty air(CLEAR);
    Data3dSq<LightingProperty, 44> props(start, height, air);
    CompileLightingProperty(models, props);
#endif

    shared_ptr<Data3dSq<uint8_t, 44>> skyLight;

    if (dim == Dimension::Overworld) {
      skyLight = make_shared<Data3dSq<uint8_t, 44>>(start, height, 0);
      InitializeSkyLight(dim, *skyLight, props);
      DiffuseSkyLight(props, *skyLight);
    }

    Data3dSq<uint8_t, 44> blockLight(start, height, 0);
    InitializeBlockLight(blockLight, props);
    DiffuseBlockLight(props, blockLight);

    for (auto &section : out.fSections) {
      if (!section) {
        continue;
      }
      Pos3i origin(out.minBlockX(), section->y() * 16, out.minBlockZ());

      if (std::any_of(blockLight.cbegin(), blockLight.cend(), [](uint8_t v) { return v > 0; })) {
        section->fBlockLight.resize(2048);
        auto sectionBlockLight = Data4b3dView::Make(origin, 16, 16, 16, &section->fBlockLight);
        if (sectionBlockLight) {
          for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
              for (int x = 0; x < 16; x++) {
                Pos3i v = origin + Pos3i(x, y, z);
                sectionBlockLight->setUnchecked(v, blockLight[v]);
              }
            }
          }
        }
      }

      if (skyLight && std::any_of(skyLight->cbegin(), skyLight->cend(), [](uint8_t v) { return v > 0; })) {
        section->fSkyLight.resize(2048);
        auto sectionSkyLight = Data4b3dView::Make(origin, 16, 16, 16, &section->fSkyLight);
        if (sectionSkyLight) {
          for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
              for (int x = 0; x < 16; x++) {
                Pos3i v = origin + Pos3i(x, y, z);
                uint8_t l = (*skyLight)[v];
                sectionSkyLight->setUnchecked(v, l);
              }
            }
          }
        }
      }
    }

    if (!out.fSections.empty() && out.fSections[0] && out.fSections[0]->y() == out.fChunkY && !out.fSections[0]->fSkyLight.empty() && skyLight->fStart.fY <= 16 * (out.fChunkY - 1)) {
      auto bottom = make_shared<mcfile::je::ChunkSectionEmpty>(out.fChunkY - 1);
      bottom->fBlockLight.clear();
      bottom->fSkyLight.resize(2048);
      Pos3i origin(out.fChunkX * 16, bottom->y() * 16, out.fChunkZ * 16);
      auto sectionSkyLight = Data4b3dView::Make(origin, 16, 16, 16, &bottom->fSkyLight);
      if (sectionSkyLight) {
        for (int y = 0; y < 16; y++) {
          for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
              Pos3i v = origin + Pos3i(x, y, z);
              uint8_t l = (*skyLight)[v];
              sectionSkyLight->setUnchecked(v, l);
            }
          }
        }
        out.fBottomSection = bottom;
      }
    }
  }

private:
  static void EnsureLightingProperties(
      LightCache &lightCache,
      Data3dSq<LightingProperty, 44> &out_,
      int cx,
      int cz,
      terraform::java::BlockAccessorJava &blockAccessor) {
    using namespace std;

    LightingModel airModel;
    airModel.fTransparency = CLEAR;
    airModel.fModel = MODEL_CLEAR;
    airModel.fEmission = 0;

    int y0 = out_.fStart.fY;
    int y1 = out_.fEnd.fY;
    Pos3i start{(cx - 1) * 16, y0, (cz - 1) * 16};
    int height = y1 - y0 + 1;

    Data3dSq<LightingModel, 48> models(start, height, airModel);
    Data3dSq<LightingProperty, 48> prop(start, height, LightingProperty(CLEAR));

    for (int dz = -1; dz <= 1; dz++) {
      for (int dx = -1; dx <= 1; dx++) {
        if (auto cachedModel = lightCache.getModel(cx + dx, cz + dz); cachedModel) {
          models.copyFrom(*cachedModel);
        } else {
          auto chunk = blockAccessor.chunkAt(cx + dx, cz + dz);
          if (!chunk) {
            continue;
          }
          Pos3i chunkOrigin{chunk->fChunkX * 16, y0, chunk->fChunkZ * 16};
          auto chunkModel = make_shared<Data3dSq<LightingModel, 16>>(chunkOrigin, height, airModel);
          CopyChunkLightingModel(*chunk, *chunkModel);
          lightCache.setModel(cx + dx, cz + dz, chunkModel);
          models.copyFrom(*chunkModel);
        }
      }
    }

    Data2d<bool> cached{Pos2i{-1, -1}, 3, 3, false};
    for (int dz = -1; dz <= 1; dz++) {
      for (int dx = -1; dx <= 1; dx++) {
        if (auto cachedProperty = lightCache.getProperty(cx + dx, cz + dz); cachedProperty) {
          prop.copyFrom(*cachedProperty);
          cached[{dx, dz}] = true;
        } else {
          cached[{dx, dz}] = false;
        }
      }
    }

    for (int dz = -1; dz <= 1; dz++) {
      for (int dx = -1; dx <= 1; dx++) {
        int x0 = std::max((cx + dx) * 16 - 1, (cx - 1) * 16);
        int x1 = std::min((cx + dx) * 16 + 16, (cx + 1) * 16 + 15);
        int z0 = std::max((cz + dz) * 16 - 1, (cz - 1) * 16);
        int z1 = std::min((cz + dz) * 16 + 16, (cz + 1) * 16 + 15);
        // https://gyazo.com/c3d414a01133eb27af7bb3c6d9171c5b
        if (cached[{dx, dz}]) {
          Volume north({x0, y0, z0}, {x1, y1, (cz + dz) * 16});
          CompileLightingProperty(models, prop, north);

          Volume east({(cx + dx) * 16 + 15, y0, z0}, {x1, y1, z1});
          CompileLightingProperty(models, prop, east);

          Volume south({x0, y0, (cz + dz) * 16 + 15}, {x1, y1, z1});
          CompileLightingProperty(models, prop, south);

          Volume west({x0, y0, z0}, {(cx + dx) * 16, y1, z1});
          CompileLightingProperty(models, prop, west);
        } else {
          Volume vol(Pos3i(x0, y0, z0), Pos3i(x1, y1, z1));
          CompileLightingProperty(models, prop, vol);
        }
      }
    }

    for (int dz = -1; dz <= 1; dz++) {
      for (int dx = -1; dx <= 1; dx++) {
        if (!cached[{dx, dz}]) {
          Pos3i chunkOrigin{(cx + dx) * 16, y0, (cz + dz) * 16};
          auto store = make_shared<Data3dSq<LightingProperty, 16>>(chunkOrigin, height, LightingProperty(CLEAR));
          store->copyFrom(prop);
          lightCache.setProperty(cx + dx, cz + dz, store);
        }
      }
    }

    out_.copyFrom(prop);
  }

  static uint32_t MaskFacing6(Facing6 facing) {
    switch (facing) {
    case Facing6::East:
      return MASK_EAST;
    case Facing6::South:
      return MASK_SOUTH;
    case Facing6::West:
      return MASK_WEST;
    case Facing6::Up:
      return MASK_UP;
    case Facing6::Down:
      return MASK_DOWN;
    case Facing6::North:
    default:
      return MASK_NORTH;
    }
  }

  static uint32_t BitSwapped(uint32_t m, int a, int b) {
    return (((m >> a) & uint32_t(1)) << b) | (((m >> b) & uint32_t(1)) << a);
  }

  static uint32_t Invert(uint32_t m) {
    return BitSwapped(m, 0, 13) | BitSwapped(m, 1, 12) | BitSwapped(m, 2, 15) | BitSwapped(m, 3, 14) | BitSwapped(m, 4, 9) | BitSwapped(m, 5, 8) | BitSwapped(m, 6, 11) | BitSwapped(m, 7, 10) | BitSwapped(m, 16, 22) | BitSwapped(m, 17, 23) | BitSwapped(m, 18, 20) | BitSwapped(m, 19, 21);
  }

  template <size_t ModelSize, size_t PropertySize>
  static void CompileLightingProperty(Data3dSq<LightingModel, ModelSize> const &src, Data3dSq<LightingProperty, PropertySize> &out, Volume const &range) {
    using namespace std;

    Data3d<Transparency> up(range.fStart, range.fEnd, CLEAR);
    Data3d<Transparency> down(range.fStart, range.fEnd, CLEAR);
    Data3d<Transparency> north(range.fStart, range.fEnd, CLEAR);
    Data3d<Transparency> east(range.fStart, range.fEnd, CLEAR);
    Data3d<Transparency> south(range.fStart, range.fEnd, CLEAR);
    Data3d<Transparency> west(range.fStart, range.fEnd, CLEAR);

    for (int y = range.fStart.fY; y <= range.fEnd.fY; y++) {
      for (int z = range.fStart.fZ; z <= range.fEnd.fZ; z++) {
        for (int x = range.fStart.fX; x <= range.fEnd.fX; x++) {
          Pos3i p(x, y, z);
          Transparency t = src[p].fTransparency;
          up[p] = t;
          down[p] = t;
          north[p] = t;
          east[p] = t;
          south[p] = t;
          west[p] = t;
        }
      }
    }

    // up, down
    uint32_t mask = MaskFacing6(Facing6::Up);
    for (int z = range.fStart.fZ; z <= range.fEnd.fZ; z++) {
      for (int x = range.fStart.fX; x <= range.fEnd.fX; x++) {
        for (int y = range.fStart.fY; y < range.fEnd.fY; y++) {
          Pos3i pos(x, y, z);
          Pos3i target(x, y + 1, z);

          LightingModel const &m = src[pos];
          LightingModel const &mTarget = src[target];

          if (m.fModel == 0 && mTarget.fModel == 0) {
            continue;
          }

          uint32_t targetInv = ~Invert(mTarget.fModel);
          uint32_t centerInv = ~m.fModel;

          uint32_t test = mask & targetInv & centerInv;

          if (m.fModel != 0) {
            up[pos] = test == 0 ? SOLID : (m.fBehaveAsAirWhenOpenUp ? CLEAR : TRANSLUCENT);
          }
          if (mTarget.fModel != 0) {
            down[target] = test == 0 ? SOLID : TRANSLUCENT;
          }
        }
      }
    }

    // east, west
    mask = MaskFacing6(Facing6::East);
    for (int y = range.fStart.fY; y <= range.fEnd.fY; y++) {
      for (int z = range.fStart.fZ; z <= range.fEnd.fZ; z++) {
        for (int x = range.fStart.fX; x < range.fEnd.fX; x++) {
          Pos3i pos(x, y, z);
          Pos3i target(x + 1, y, z);

          LightingModel const &m = src[pos];
          LightingModel const &mTarget = src[target];
          if (m.fModel == 0 && mTarget.fModel == 0) {
            continue;
          }

          uint32_t targetInv = ~Invert(mTarget.fModel);
          uint32_t centerInv = ~m.fModel;

          uint32_t test = mask & targetInv & centerInv;

          if (m.fModel != 0) {
            east[pos] = test == 0 ? SOLID : TRANSLUCENT;
          }
          if (mTarget.fModel != 0) {
            west[target] = test == 0 ? SOLID : TRANSLUCENT;
          }
        }
      }
    }

    // north, south
    mask = MaskFacing6(Facing6::South);
    for (int y = range.fStart.fY; y <= range.fEnd.fY; y++) {
      for (int x = range.fStart.fX; x <= range.fEnd.fX; x++) {
        for (int z = range.fStart.fZ; z < range.fEnd.fZ; z++) {
          Pos3i pos(x, y, z);
          Pos3i target(x, y, z + 1);

          LightingModel const &m = src[pos];
          LightingModel const &mTarget = src[target];

          if (m.fModel == 0 && mTarget.fModel == 0) {
            continue;
          }

          uint32_t targetInv = ~Invert(mTarget.fModel);
          uint32_t centerInv = ~m.fModel;

          uint32_t test = mask & targetInv & centerInv;

          if (m.fModel != 0) {
            south[pos] = test == 0 ? SOLID : TRANSLUCENT;
          }
          if (mTarget.fModel != 0) {
            north[target] = test == 0 ? SOLID : TRANSLUCENT;
          }
        }
      }
    }

    for (int y = range.fStart.fY; y <= range.fEnd.fY; y++) {
      for (int z = range.fStart.fZ; z <= range.fEnd.fZ; z++) {
        for (int x = range.fStart.fX; x <= range.fEnd.fX; x++) {
          Pos3i pos(x, y, z);
          LightingProperty p(CLEAR);
          p.fUp = up[pos];
          p.fDown = down[pos];
          p.fNorth = north[pos];
          p.fEast = east[pos];
          p.fSouth = south[pos];
          p.fWest = west[pos];
          p.fEmission = src[pos].fEmission;
          out[pos] = p;
        }
      }
    }
  }

  static void CompileLightingProperty(Data3dSq<LightingModel, 44> const &src, Data3dSq<LightingProperty, 44> &out) {
    using namespace std;
    assert(src.fStart == out.fStart);
    assert(src.fEnd == out.fEnd);

    int height = out.fEnd.fY - out.fStart.fY + 1;
    Data3dSq<Transparency, 44> up(out.fStart, height, CLEAR);
    Data3dSq<Transparency, 44> down(out.fStart, height, CLEAR);
    Data3dSq<Transparency, 44> north(out.fStart, height, CLEAR);
    Data3dSq<Transparency, 44> east(out.fStart, height, CLEAR);
    Data3dSq<Transparency, 44> south(out.fStart, height, CLEAR);
    Data3dSq<Transparency, 44> west(out.fStart, height, CLEAR);

    for (int y = src.fStart.fY; y <= src.fEnd.fY; y++) {
      for (int z = src.fStart.fZ; z <= src.fEnd.fZ; z++) {
        for (int x = src.fStart.fX; x <= src.fEnd.fX; x++) {
          Pos3i p(x, y, z);
          Transparency t = src[p].fTransparency;
          up[p] = t;
          down[p] = t;
          north[p] = t;
          east[p] = t;
          south[p] = t;
          west[p] = t;
        }
      }
    }

    // up, down
    uint32_t mask = MaskFacing6(Facing6::Up);
    for (int z = src.fStart.fZ; z <= src.fEnd.fZ; z++) {
      for (int x = src.fStart.fX; x <= src.fEnd.fX; x++) {
        for (int y = src.fStart.fY; y < src.fEnd.fY; y++) {
          Pos3i pos(x, y, z);
          Pos3i target(x, y + 1, z);

          LightingModel const &m = src[pos];
          LightingModel const &mTarget = src[target];

          uint32_t targetInv = ~Invert(mTarget.fModel);
          uint32_t centerInv = ~m.fModel;

          uint32_t test = mask & targetInv & centerInv;

          down[target] = test == 0 ? SOLID : TRANSLUCENT;
          up[pos] = test == 0 ? SOLID : (m.fBehaveAsAirWhenOpenUp ? CLEAR : TRANSLUCENT);
        }
      }
    }

    // east, west
    mask = MaskFacing6(Facing6::East);
    for (int y = src.fStart.fY; y <= src.fEnd.fY; y++) {
      for (int z = src.fStart.fZ; z <= src.fEnd.fZ; z++) {
        for (int x = src.fStart.fX; x < src.fEnd.fX; x++) {
          Pos3i pos(x, y, z);
          Pos3i target(x + 1, y, z);

          LightingModel const &m = src[pos];
          LightingModel const &mTarget = src[target];

          uint32_t targetInv = ~Invert(mTarget.fModel);
          uint32_t centerInv = ~m.fModel;

          uint32_t test = mask & targetInv & centerInv;

          east[pos] = test == 0 ? SOLID : TRANSLUCENT;
          west[target] = test == 0 ? SOLID : TRANSLUCENT;
        }
      }
    }

    // north, south
    mask = MaskFacing6(Facing6::South);
    for (int y = src.fStart.fY; y <= src.fEnd.fY; y++) {
      for (int x = src.fStart.fX; x <= src.fEnd.fX; x++) {
        for (int z = src.fStart.fZ; z < src.fEnd.fZ; z++) {
          Pos3i pos(x, y, z);
          Pos3i target(x, y, z + 1);

          LightingModel const &m = src[pos];
          LightingModel const &mTarget = src[target];

          uint32_t targetInv = ~Invert(mTarget.fModel);
          uint32_t centerInv = ~m.fModel;

          uint32_t test = mask & targetInv & centerInv;

          south[pos] = test == 0 ? SOLID : TRANSLUCENT;
          north[target] = test == 0 ? SOLID : TRANSLUCENT;
        }
      }
    }

    for (int y = src.fStart.fY; y <= src.fEnd.fY; y++) {
      for (int z = src.fStart.fZ; z <= src.fEnd.fZ; z++) {
        for (int x = src.fStart.fX; x <= src.fEnd.fX; x++) {
          Pos3i pos(x, y, z);
          LightingProperty p(CLEAR);
          p.fUp = up[pos];
          p.fDown = down[pos];
          p.fNorth = north[pos];
          p.fEast = east[pos];
          p.fSouth = south[pos];
          p.fWest = west[pos];
          p.fEmission = src[pos].fEmission;
          out[pos] = p;
        }
      }
    }
  }

  static void DiffuseSkyLight(Data3dSq<LightingProperty, 44> const &props, Data3dSq<uint8_t, 44> &out) {
    for (int z = out.fStart.fZ; z <= out.fEnd.fZ; z++) {
      for (int x = out.fStart.fX; x <= out.fEnd.fX; x++) {
        for (int y = out.fEnd.fY - 1; y >= out.fStart.fY; y--) {
          auto p = props.get({x, y, z});
          if (!p) {
            break;
          }
          if (p->fUp == CLEAR) {
            out[{x, y, z}] = 15;
          } else {
            break;
          }
          if (p->fDown != CLEAR) {
            break;
          }
        }
      }
    }
    DiffuseLight(props, out);
  }

  static void DiffuseBlockLight(Data3dSq<LightingProperty, 44> const &props, Data3dSq<uint8_t, 44> &out) {
    DiffuseLight(props, out);
  }

  static void DiffuseLight(Data3dSq<LightingProperty, 44> const &props, Data3dSq<uint8_t, 44> &out) {
    int x0 = out.fStart.fX;
    int x1 = out.fEnd.fX;
    int y0 = out.fStart.fY;
    int y1 = out.fEnd.fY;
    int z0 = out.fStart.fZ;
    int z1 = out.fEnd.fZ;

    for (int v = 14; v >= 1; v--) {
      for (int y = y0; y <= y1; y++) {
        for (int z = z0; z <= z1; z++) {
          for (int x = x0; x <= x1; x++) {
            uint8_t center = out[{x, y, z}];
            if (center >= v) {
              continue;
            }
            auto p = props.get({x, y, z});
            if (!p) {
              continue;
            }
            if (y + 1 <= y1 && p->fUp < SOLID) {
              if (auto pUp = props.get({x, y + 1, z}); pUp && (pUp->fEmission > 0 || pUp->fDown < SOLID)) {
                int up = out[{x, y + 1, z}];
                if (up == v + 1) {
                  out[{x, y, z}] = (uint8_t)v;
                  continue;
                }
              }
            }
            if (y - 1 >= y0 && p->fDown < SOLID) {
              if (auto pDown = props.get({x, y - 1, z}); pDown && (pDown->fEmission > 0 || pDown->fUp < SOLID)) {
                int down = out[{x, y - 1, z}];
                if (down == v + 1) {
                  out[{x, y, z}] = (uint8_t)v;
                  continue;
                }
              }
            }
            if (x + 1 <= x1 && p->fEast < SOLID) {
              if (auto pEast = props.get({x + 1, y, z}); pEast && (pEast->fEmission > 0 || pEast->fWest < SOLID)) {
                int east = out[{x + 1, y, z}];
                if (east == v + 1) {
                  out[{x, y, z}] = (uint8_t)v;
                  continue;
                }
              }
            }
            if (x - 1 >= x0 && p->fWest < SOLID) {
              if (auto pWest = props.get({x - 1, y, z}); pWest && (pWest->fEmission > 0 || pWest->fEast < SOLID)) {
                int west = out[{x - 1, y, z}];
                if (west == v + 1) {
                  out[{x, y, z}] = (uint8_t)v;
                  continue;
                }
              }
            }
            if (z + 1 <= z1 && p->fSouth < SOLID) {
              if (auto pSouth = props.get({x, y, z + 1}); pSouth && (pSouth->fEmission > 0 || pSouth->fNorth < SOLID)) {
                int south = out[{x, y, z + 1}];
                if (south == v + 1) {
                  out[{x, y, z}] = (uint8_t)v;
                  continue;
                }
              }
            }
            if (z - 1 >= z0 && p->fNorth < SOLID) {
              if (auto pNorth = props.get({x, y, z - 1}); pNorth && (pNorth->fEmission > 0 || pNorth->fSouth < SOLID)) {
                int north = out[{x, y, z - 1}];
                if (north == v + 1) {
                  out[{x, y, z}] = (uint8_t)v;
                  continue;
                }
              }
            }
          }
        }
      }
    }
  }

  static void InitializeSkyLight(mcfile::Dimension dim, Data3dSq<uint8_t, 44> &out, Data3dSq<LightingProperty, 44> const &props) {
    assert(out.fStart.fX <= props.fStart.fX && props.fEnd.fX <= out.fEnd.fX);
    assert(out.fStart.fY <= props.fStart.fY && props.fEnd.fY <= out.fEnd.fY);
    assert(out.fStart.fZ <= props.fStart.fZ && props.fEnd.fZ <= out.fEnd.fZ);

    out.fill(0);

    for (int z = props.fStart.fZ; z <= props.fEnd.fZ; z++) {
      for (int x = props.fStart.fX; x <= props.fEnd.fX; x++) {
        out[{x, out.fEnd.fY, z}] = 15;
      }
    }
  }

  static void InitializeBlockLight(Data3dSq<uint8_t, 44> &out, Data3dSq<LightingProperty, 44> const &props) {
    using namespace std;

    assert(out.fStart.fX <= props.fStart.fX && props.fEnd.fX <= out.fEnd.fX);
    assert(out.fStart.fY <= props.fStart.fY && props.fEnd.fY <= out.fEnd.fY);
    assert(out.fStart.fZ <= props.fStart.fZ && props.fEnd.fZ <= out.fEnd.fZ);

    out.fill(0);

    for (int y = out.fStart.fY; y <= out.fEnd.fY; y++) {
      for (int z = out.fStart.fZ; z <= out.fEnd.fZ; z++) {
        for (int x = out.fStart.fX; x <= out.fEnd.fX; x++) {
          if (auto p = props.get({x, y, z}); p) {
            if (p->fEmission > 0) {
              out[{x, y, z}] = p->fEmission;
            }
          }
        }
      }
    }
  }

  template <size_t Size>
  static void CopyChunkLightingModel(mcfile::je::Chunk const &chunk, Data3dSq<LightingModel, Size> &out) {
    using namespace std;
    assert(out.fStart.fY <= chunk.minBlockY() && chunk.maxBlockY() <= out.fEnd.fY);

    int x0 = (std::max)(chunk.minBlockX(), out.fStart.fX);
    int x1 = (std::min)(chunk.maxBlockX(), out.fEnd.fX);
    int z0 = (std::max)(chunk.minBlockZ(), out.fStart.fZ);
    int z1 = (std::min)(chunk.maxBlockZ(), out.fEnd.fZ);

    for (auto const &section : chunk.fSections) {
      if (!section) {
        continue;
      }
      vector<LightingModel> models;
      section->eachBlockPalette([&](shared_ptr<mcfile::je::Block const> const &block, size_t) {
        models.push_back(GetLightingModel(*block));
        return true;
      });
      for (int y = 0; y < 16; y++) {
        for (int bz = z0; bz <= z1; bz++) {
          for (int bx = x0; bx <= x1; bx++) {
            if (auto index = section->blockPaletteIndexAt(bx - chunk.minBlockX(), y, bz - chunk.minBlockZ()); index) {
              LightingModel m = models[*index];
              int by = y + section->y() * 16;
              out.set({bx, by, bz}, m);
            }
          }
        }
      }
    }
  }

  static LightingModel GetLightingModel(mcfile::je::Block const &block) {
    // https://gyazo.com/a270e426c0eb2b8b53083317b5aab16f
    using namespace mcfile::blocks::minecraft;

    if (block.fName.ends_with("_stairs")) {
      auto half = block.property("half");
      auto facing = block.property("facing", "north");
      Facing4 f4 = Facing4FromJavaName(facing);
      auto shape = block.property("shape");
      auto waterlogged = block.property("waterlogged");

      LightingModel m;
      m.fEmission = 0;
      m.fModel = MODEL_CLEAR;
      m.fTransparency = TRANSLUCENT;
      m.fBehaveAsAirWhenOpenUp = waterlogged != "true";

      if (shape == "outer_right") {
        switch (f4) {
        case Facing4::North:
          if (half == "bottom") {
            m.fModel = MODEL_HALF_BOTTOM | uint32_t(0x400408);
          } else {
            m.fModel = MODEL_HALF_TOP | uint32_t(0x10102);
          }
          break;
        case Facing4::East:
          if (half == "bottom") {
            m.fModel = MODEL_HALF_BOTTOM | uint32_t(0x104800);
          } else {
            m.fModel = MODEL_HALF_TOP | uint32_t(0x41200);
          }
          break;
        case Facing4::South:
          if (half == "bottom") {
            m.fModel = MODEL_HALF_BOTTOM | uint32_t(0x208040);
          } else {
            m.fModel = MODEL_HALF_TOP | uint32_t(0x82010);
          }
          break;
        case Facing4::West:
          if (half == "bottom") {
            m.fModel = MODEL_HALF_BOTTOM | uint32_t(0x800084);
          } else {
            m.fModel = MODEL_HALF_TOP | uint32_t(0x20021);
          }
          break;
        }
      } else if (shape == "outer_left") {
        switch (f4) {
        case Facing4::North:
          if (half == "bottom") {
            m.fModel = MODEL_HALF_BOTTOM | uint32_t(0x800084);
          } else {
            m.fModel = MODEL_HALF_TOP | uint32_t(0x20021);
          }
          break;
        case Facing4::East:
          if (half == "bottom") {
            m.fModel = MODEL_HALF_BOTTOM | uint32_t(0x400408);
          } else {
            m.fModel = MODEL_HALF_TOP | uint32_t(0x10102);
          }
          break;
        case Facing4::South:
          if (half == "bottom") {
            m.fModel = MODEL_HALF_BOTTOM | uint32_t(0x104800);
          } else {
            m.fModel = MODEL_HALF_TOP | uint32_t(0x41200);
          }
          break;
        case Facing4::West:
          if (half == "bottom") {
            m.fModel = MODEL_HALF_BOTTOM | uint32_t(0x208040);
          } else {
            m.fModel = MODEL_HALF_TOP | uint32_t(0x82010);
          }
          break;
        }
      } else if (shape.starts_with("inner_right")) {
        switch (f4) {
        case Facing4::North:
          if (half == "bottom") {
            m.fModel = MODEL_SOLID & (~uint32_t(0x208040));
          } else {
            m.fModel = MODEL_SOLID & (~uint32_t(0x82010));
          }
          break;
        case Facing4::East:
          if (half == "bottom") {
            m.fModel = MODEL_SOLID & (~uint32_t(0x800084));
          } else {
            m.fModel = MODEL_SOLID & (~uint32_t(0x20021));
          }
          break;
        case Facing4::South:
          if (half == "bottom") {
            m.fModel = MODEL_SOLID & (~uint32_t(0x400408));
          } else {
            m.fModel = MODEL_SOLID & (~uint32_t(0x10102));
          }
          break;
        case Facing4::West:
          if (half == "bottom") {
            m.fModel = MODEL_SOLID & (~uint32_t(0x104800));
          } else {
            m.fModel = MODEL_SOLID & (~uint32_t(0x41200));
          }
          break;
        }
      } else if (shape.starts_with("inner_left")) {
        switch (f4) {
        case Facing4::North:
          if (half == "bottom") {
            m.fModel = MODEL_SOLID & (~uint32_t(0x104800));
          } else {
            m.fModel = MODEL_SOLID & (~uint32_t(0x41200));
          }
          break;
        case Facing4::East:
          if (half == "bottom") {
            m.fModel = MODEL_SOLID & (~uint32_t(0x208040));
          } else {
            m.fModel = MODEL_SOLID & (~uint32_t(0x82010));
          }
          break;
        case Facing4::South:
          if (half == "bottom") {
            m.fModel = MODEL_SOLID & (~uint32_t(0x800084));
          } else {
            m.fModel = MODEL_SOLID & (~uint32_t(0x20021));
          }
          break;
        case Facing4::West:
          if (half == "bottom") {
            m.fModel = MODEL_SOLID & (~uint32_t(0x400408));
          } else {
            m.fModel = MODEL_SOLID & (~uint32_t(0x10102));
          }
          break;
        }
      } else { // "straight"
        switch (f4) {
        case Facing4::North:
          if (half == "bottom") {
            m.fModel = MODEL_HALF_BOTTOM | uint32_t(0xC0048C);
          } else {
            m.fModel = MODEL_HALF_TOP | uint32_t(0x30123);
          }
          break;
        case Facing4::East:
          if (half == "bottom") {
            m.fModel = MODEL_HALF_BOTTOM | uint32_t(0x504C08);
          } else {
            m.fModel = MODEL_HALF_TOP | uint32_t(0x51302);
          }
          break;
        case Facing4::South:
          if (half == "bottom") {
            m.fModel = MODEL_HALF_BOTTOM | uint32_t(0x30C840);
          } else {
            m.fModel = MODEL_HALF_TOP | uint32_t(0xC3210);
          }
          break;
        case Facing4::West:
          if (half == "bottom") {
            m.fModel = MODEL_HALF_BOTTOM | uint32_t(0xA080C4);
          } else {
            m.fModel = MODEL_HALF_TOP | uint32_t(0xA2031);
          }
          break;
        }
      }
      return m;
    } else if (block.fName.ends_with("slab")) {
      LightingModel m;
      m.fEmission = 0;
      m.fBehaveAsAirWhenOpenUp = block.property("waterlogged") != "true";
      auto type = block.property("type");
      if (type == "double") {
        m.fModel = MODEL_SOLID;
        m.fTransparency = SOLID;
      } else if (type == "top") {
        m.fModel = MODEL_HALF_TOP;
        m.fTransparency = TRANSLUCENT;
      } else { // "bottom"
        m.fModel = MODEL_HALF_BOTTOM;
        m.fTransparency = CLEAR;
      }
      return m;
    } else if (block.fId == piston || block.fId == sticky_piston) {
      if (block.property("extended") == "true") {
        Facing6 f = Facing6FromJavaName(block.property("facing"));
        LightingModel m;
        m.fEmission = 0;
        m.fTransparency = TRANSLUCENT;
        switch (f) {
        case Facing6::Up:
          m.fModel = MODEL_HALF_BOTTOM;
          break;
        case Facing6::Down:
          m.fModel = MODEL_HALF_TOP;
          break;
        case Facing6::North:
          m.fModel = uint32_t(0x3CFA50);
          break;
        case Facing6::East:
          m.fModel = uint32_t(0x6AA0F5);
          break;
        case Facing6::South:
          m.fModel = uint32_t(0xC305AF);
          break;
        case Facing6::West:
          m.fModel = uint32_t(0x555F0A);
          break;
        default:
          break;
        }
        return m;
      } else {
        LightingModel m;
        m.fEmission = 0;
        m.fModel = MODEL_SOLID;
        m.fTransparency = SOLID;
        return m;
      }
    } else if (block.fId == piston_head) {
      Facing6 f = Facing6FromJavaName(block.property("facing"));
      LightingModel m;
      m.fEmission = 0;
      m.fTransparency = TRANSLUCENT;
      switch (f) {
      case Facing6::Up:
        m.fModel = uint32_t(0xF00000);
        break;
      case Facing6::Down:
        m.fModel = uint32_t(0xF0000);
        break;
      case Facing6::North:
        m.fModel = uint32_t(0xf);
        break;
      case Facing6::East:
        m.fModel = uint32_t(0xF00);
        break;
      case Facing6::South:
        m.fModel = uint32_t(0xF000);
        break;
      case Facing6::West:
        m.fModel = uint32_t(0xF0);
        break;
      default:
        break;
      }
      return m;
    }
    int amount = 0;
    switch (block.fId) {
    case bamboo:
    case cactus:
    case chorus_flower:
    case chorus_plant:
    case farmland:
    case fire:
    case twisting_vines_plant:
    case weeping_vines_plant:
    case soul_fire:
    case cave_vines_plant:
      amount = 0;
      break;
    case bubble_column:
    case kelp_plant:
    case water:
    case tall_seagrass:
    case seagrass:
    case kelp:
      amount = 1;
      break;
    case brain_coral_block:
    case bubble_coral_block:
    case fire_coral_block:
    case horn_coral_block:
      amount = 15;
      break;
    default:
      amount = LightAttenuationAmountById(block.fId);
      break;
    }
    if (amount == 0 && block.property("waterlogged") == "true") {
      amount = 1;
    }
    switch (amount) {
    case 0: {
      LightingModel m;
      m.fTransparency = CLEAR;
      m.fEmission = LightEmission(block);
      m.fModel = MODEL_CLEAR;
      return m;
    }
    case 1: {
      LightingModel m;
      m.fTransparency = TRANSLUCENT;
      m.fEmission = LightEmission(block);
      m.fModel = MODEL_CLEAR;
      return m;
    }
    case 15:
    default: {
      LightingModel m;
      m.fTransparency = SOLID;
      m.fEmission = LightEmission(block);
      m.fModel = MODEL_SOLID;
      return m;
    }
    }
  }

  static uint8_t LightEmission(mcfile::je::Block const &block) {
    using namespace mcfile::blocks::minecraft;
    switch (block.fId) {
    case dirt_path:
    case big_dripleaf_stem:
      return 0;
    case glow_lichen:
      // glow_lichen does not emit light when placed by setblock
      return 7;
    case soul_fire:
      return 10;
    case fire:
      return 15;
    }
    if (block.fId == cave_vines_plant) {
      if (block.property("berries") == "true") {
        return 14;
      } else {
        return 0;
      }
    } else if (block.fId == furnace || block.fId == smoker || block.fId == blast_furnace) {
      if (block.property("lit") == "true") {
        return 14;
      } else {
        return 0;
      }
    } else if (block.fId == campfire) {
      if (block.property("lit") == "true") {
        return 15;
      } else {
        return 0;
      }
    } else if (block.fId == soul_campfire) {
      if (block.property("lit") == "true") {
        return 10;
      } else {
        return 0;
      }
    } else if (block.fId == redstone_torch || block.fId == redstone_wall_torch) {
      if (block.property("lit") == "true") {
        return 7;
      } else {
        return 0;
      }
    } else if (block.fName.ends_with("candle_cake")) {
      if (block.property("lit") == "true") {
        return 3;
      } else {
        return 0;
      }
    } else if (block.fName.ends_with("candle")) {
      if (block.property("lit") == "true") {
        auto candles = Wrap(strings::Toi(block.property("candles", "1")), 1);
        return candles * 3;
      } else {
        return 0;
      }
    } else if (block.fId == lava_cauldron) {
      return 15;
    } else if (block.fId == redstone_lamp) {
      if (block.property("lit") == "true") {
        return 15;
      } else {
        return 0;
      }
    }
    return LightEmissionById(block.fId);
  }

  static uint8_t LightAttenuationAmountById(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks::minecraft;
    switch (id) {
    case acacia_button:
    case acacia_door:
    case acacia_fence:
    case acacia_fence_gate:
    case acacia_pressure_plate:
    case acacia_sapling:
    case acacia_sign:
    case acacia_trapdoor:
    case acacia_wall_sign:
    case activator_rail:
    case air:
    case allium:
    case andesite_wall:
    case anvil:
    case attached_melon_stem:
    case attached_pumpkin_stem:
    case azure_bluet:
    case bamboo_sapling:
    case barrier:
    case beetroots:
    case bell:
    case birch_button:
    case birch_door:
    case birch_fence:
    case birch_fence_gate:
    case birch_pressure_plate:
    case birch_sapling:
    case birch_sign:
    case birch_trapdoor:
    case birch_wall_sign:
    case black_banner:
    case black_bed:
    case black_carpet:
    case black_stained_glass:
    case black_stained_glass_pane:
    case black_wall_banner:
    case blue_banner:
    case blue_bed:
    case blue_carpet:
    case blue_orchid:
    case blue_stained_glass:
    case blue_stained_glass_pane:
    case blue_wall_banner:
    case brain_coral:
    case brain_coral_fan:
    case brewing_stand:
    case brick_wall:
    case brown_banner:
    case brown_bed:
    case brown_carpet:
    case brown_mushroom:
    case brown_stained_glass:
    case brown_stained_glass_pane:
    case brown_wall_banner:
    case cake:
    case campfire:
    case carrots:
    case cauldron:
    case cave_air:
    case chest:
    case chipped_anvil:
    case cobblestone_wall:
    case cocoa:
    case composter:
    case conduit:
    case cornflower:
    case creeper_head:
    case creeper_wall_head:
    case cyan_banner:
    case cyan_bed:
    case cyan_carpet:
    case cyan_stained_glass:
    case cyan_stained_glass_pane:
    case cyan_wall_banner:
    case damaged_anvil:
    case dandelion:
    case dark_oak_button:
    case dark_oak_door:
    case dark_oak_fence:
    case dark_oak_fence_gate:
    case dark_oak_pressure_plate:
    case dark_oak_sapling:
    case dark_oak_sign:
    case dark_oak_trapdoor:
    case dark_oak_wall_sign:
    case daylight_detector:
    case dead_brain_coral:
    case dead_brain_coral_fan:
    case dead_brain_coral_wall_fan:
    case dead_bubble_coral:
    case dead_bubble_coral_fan:
    case dead_bubble_coral_wall_fan:
    case dead_bush:
    case dead_fire_coral:
    case dead_fire_coral_fan:
    case dead_fire_coral_wall_fan:
    case dead_horn_coral:
    case dead_horn_coral_fan:
    case dead_horn_coral_wall_fan:
    case dead_tube_coral:
    case dead_tube_coral_fan:
    case dead_tube_coral_wall_fan:
    case detector_rail:
    case diorite_wall:
    case dragon_egg:
    case dragon_head:
    case dragon_wall_head:
    case enchanting_table:
    case end_portal:
    case end_portal_frame:
    case end_rod:
    case end_stone_brick_wall:
    case ender_chest:
    case farmland:
    case fern:
    case fire:
    case fire_coral:
    case fire_coral_fan:
    case flower_pot:
    case glass:
    case glass_pane:
    case granite_wall:
    case grass:
    case gray_banner:
    case gray_bed:
    case gray_carpet:
    case gray_stained_glass:
    case gray_stained_glass_pane:
    case gray_wall_banner:
    case green_banner:
    case green_bed:
    case green_carpet:
    case green_stained_glass:
    case green_stained_glass_pane:
    case green_wall_banner:
    case grindstone:
    case heavy_weighted_pressure_plate:
    case hopper:
    case iron_bars:
    case iron_door:
    case iron_trapdoor:
    case jungle_button:
    case jungle_door:
    case jungle_fence:
    case jungle_fence_gate:
    case jungle_pressure_plate:
    case jungle_sapling:
    case jungle_sign:
    case jungle_trapdoor:
    case jungle_wall_sign:
    case ladder:
    case lantern:
    case large_fern:
    case lectern:
    case lever:
    case light_blue_banner:
    case light_blue_bed:
    case light_blue_carpet:
    case light_blue_stained_glass:
    case light_blue_stained_glass_pane:
    case light_blue_wall_banner:
    case light_gray_banner:
    case light_gray_bed:
    case light_gray_carpet:
    case light_gray_stained_glass:
    case light_gray_stained_glass_pane:
    case light_gray_wall_banner:
    case light_weighted_pressure_plate:
    case lilac:
    case lily_of_the_valley:
    case lily_pad:
    case lime_banner:
    case lime_bed:
    case lime_carpet:
    case lime_stained_glass:
    case lime_stained_glass_pane:
    case lime_wall_banner:
    case magenta_banner:
    case magenta_bed:
    case magenta_carpet:
    case magenta_stained_glass:
    case magenta_stained_glass_pane:
    case magenta_wall_banner:
    case melon_stem:
    case mossy_cobblestone_wall:
    case mossy_stone_brick_wall:
    case moving_piston:
    case nether_brick_fence:
    case nether_brick_wall:
    case nether_portal:
    case nether_wart:
    case oak_button:
    case oak_door:
    case oak_fence:
    case oak_fence_gate:
    case oak_pressure_plate:
    case oak_sapling:
    case oak_sign:
    case oak_trapdoor:
    case oak_wall_sign:
    case orange_banner:
    case orange_bed:
    case orange_carpet:
    case orange_stained_glass:
    case orange_stained_glass_pane:
    case orange_tulip:
    case orange_wall_banner:
    case oxeye_daisy:
    case peony:
    case pink_banner:
    case pink_bed:
    case pink_carpet:
    case pink_stained_glass:
    case pink_stained_glass_pane:
    case pink_tulip:
    case pink_wall_banner:
    case piston_head:
    case player_head:
    case player_wall_head:
    case poppy:
    case potatoes:
    case potted_acacia_sapling:
    case potted_allium:
    case potted_azure_bluet:
    case potted_bamboo:
    case potted_birch_sapling:
    case potted_blue_orchid:
    case potted_brown_mushroom:
    case potted_cactus:
    case potted_cornflower:
    case potted_dandelion:
    case potted_dark_oak_sapling:
    case potted_dead_bush:
    case potted_fern:
    case potted_jungle_sapling:
    case potted_lily_of_the_valley:
    case potted_oak_sapling:
    case potted_orange_tulip:
    case potted_oxeye_daisy:
    case potted_pink_tulip:
    case potted_poppy:
    case potted_red_mushroom:
    case potted_red_tulip:
    case potted_spruce_sapling:
    case potted_white_tulip:
    case potted_wither_rose:
    case powered_rail:
    case prismarine_wall:
    case pumpkin_stem:
    case purple_banner:
    case purple_bed:
    case purple_carpet:
    case purple_stained_glass:
    case purple_stained_glass_pane:
    case purple_wall_banner:
    case rail:
    case red_banner:
    case red_bed:
    case red_carpet:
    case red_mushroom:
    case red_nether_brick_wall:
    case red_sandstone_wall:
    case red_stained_glass:
    case red_stained_glass_pane:
    case red_tulip:
    case red_wall_banner:
    case comparator:
    case redstone_wire:
    case repeater:
    case redstone_torch:
    case redstone_wall_torch:
    case rose_bush:
    case sandstone_wall:
    case scaffolding:
    case sea_pickle:
    case skeleton_skull:
    case skeleton_wall_skull:
    case snow:
    case spruce_button:
    case spruce_door:
    case spruce_fence:
    case spruce_fence_gate:
    case spruce_pressure_plate:
    case spruce_sapling:
    case spruce_sign:
    case spruce_trapdoor:
    case spruce_wall_sign:
    case stone_brick_wall:
    case stone_button:
    case stone_pressure_plate:
    case stonecutter:
    case structure_void:
    case sunflower:
    case sweet_berry_bush:
    case tall_grass:
    case torch:
    case trapped_chest:
    case tripwire:
    case tripwire_hook:
    case turtle_egg:
    case vine:
    case void_air:
    case wall_torch:
    case wheat:
    case white_banner:
    case white_bed:
    case white_carpet:
    case white_stained_glass:
    case white_stained_glass_pane:
    case white_tulip:
    case white_wall_banner:
    case wither_rose:
    case wither_skeleton_skull:
    case wither_skeleton_wall_skull:
    case yellow_banner:
    case yellow_bed:
    case yellow_carpet:
    case yellow_stained_glass:
    case yellow_stained_glass_pane:
    case yellow_wall_banner:
    case zombie_head:
    case zombie_wall_head:
    case crimson_fungus:
    case warped_fungus:
    case crimson_roots:
    case warped_roots:
    case nether_sprouts:
    case weeping_vines:
    case twisting_vines:
    case crimson_fence:
    case warped_fence:
    case soul_torch:
    case chain:
    case blackstone_wall:
    case polished_blackstone_wall:
    case polished_blackstone_brick_wall:
    case soul_lantern:
    case soul_campfire:
    case crimson_pressure_plate:
    case warped_pressure_plate:
    case crimson_trapdoor:
    case warped_trapdoor:
    case crimson_fence_gate:
    case warped_fence_gate:
    case crimson_button:
    case warped_button:
    case crimson_door:
    case warped_door:
    case crimson_sign:
    case warped_sign:
    case polished_blackstone_pressure_plate:
    case polished_blackstone_button:
    case crimson_wall_sign:
    case warped_wall_sign:
    case soul_wall_torch:
    case azalea:
    case flowering_azalea:
    case spore_blossom:
    case moss_carpet:
    case hanging_roots:
    case potted_azalea_bush:
    case potted_flowering_azalea_bush:
    case water_cauldron:
    case lava_cauldron:
    case powder_snow_cauldron:
    case big_dripleaf:
    case small_dripleaf:
    case candle:
    case white_candle:
    case orange_candle:
    case magenta_candle:
    case light_blue_candle:
    case yellow_candle:
    case lime_candle:
    case pink_candle:
    case gray_candle:
    case light_gray_candle:
    case cyan_candle:
    case purple_candle:
    case blue_candle:
    case brown_candle:
    case green_candle:
    case red_candle:
    case black_candle:
    case candle_cake:
    case white_candle_cake:
    case orange_candle_cake:
    case magenta_candle_cake:
    case light_blue_candle_cake:
    case yellow_candle_cake:
    case lime_candle_cake:
    case pink_candle_cake:
    case gray_candle_cake:
    case light_gray_candle_cake:
    case cyan_candle_cake:
    case purple_candle_cake:
    case blue_candle_cake:
    case brown_candle_cake:
    case green_candle_cake:
    case red_candle_cake:
    case black_candle_cake:
    case cobbled_deepslate_wall:
    case polished_deepslate_wall:
    case deepslate_brick_wall:
    case deepslate_tile_wall:
    case lightning_rod:
    case small_amethyst_bud:
    case medium_amethyst_bud:
    case large_amethyst_bud:
    case amethyst_cluster:
    case pointed_dripstone:
    case light:
    case cave_vines:
    case glow_lichen:
    case potted_crimson_fungus:
    case potted_warped_fungus:
    case potted_crimson_roots:
    case potted_warped_roots:
    case sculk_sensor:
    case mangrove_fence:
    case mangrove_fence_gate:
    case mud_brick_wall:
    case mangrove_sign:
    case mangrove_wall_sign:
    case mangrove_propagule:
    case mangrove_button:
    case mangrove_pressure_plate:
    case mangrove_door:
    case mangrove_trapdoor:
    case frogspawn:
    case potted_mangrove_propagule:
    case acacia_hanging_sign:
    case acacia_wall_hanging_sign:
    case bamboo_button:
    case bamboo_door:
    case bamboo_fence:
    case bamboo_fence_gate:
    case bamboo_hanging_sign:
    case bamboo_pressure_plate:
    case bamboo_sign:
    case bamboo_trapdoor:
    case bamboo_wall_hanging_sign:
    case bamboo_wall_sign:
    case birch_hanging_sign:
    case birch_wall_hanging_sign:
    case crimson_hanging_sign:
    case crimson_wall_hanging_sign:
    case dark_oak_hanging_sign:
    case dark_oak_wall_hanging_sign:
    case jungle_hanging_sign:
    case jungle_wall_hanging_sign:
    case mangrove_hanging_sign:
    case mangrove_wall_hanging_sign:
    case oak_hanging_sign:
    case oak_wall_hanging_sign:
    case piglin_head:
    case piglin_wall_head:
    case spruce_hanging_sign:
    case spruce_wall_hanging_sign:
    case warped_hanging_sign:
    case warped_wall_hanging_sign:
      return 0;
    case acacia_leaves:
    case beacon:
    case birch_leaves:
    case black_shulker_box:
    case blue_shulker_box:
    case brown_shulker_box:
    case cobweb:
    case cyan_shulker_box:
    case dark_oak_leaves:
    case end_gateway:
    case frosted_ice:
    case gray_shulker_box:
    case green_shulker_box:
    case ice:
    case jungle_leaves:
    case lava:
    case light_blue_shulker_box:
    case light_gray_shulker_box:
    case lime_shulker_box:
    case magenta_shulker_box:
    case oak_leaves:
    case orange_shulker_box:
    case pink_shulker_box:
    case purple_shulker_box:
    case red_shulker_box:
    case shulker_box:
    case slime_block:
    case spawner:
    case spruce_leaves:
    case white_shulker_box:
    case yellow_shulker_box:
    case honey_block:
    case azalea_leaves:
    case powder_snow:
    case flowering_azalea_leaves:
    case sculk_shrieker:
    case sculk_vein:
    case mangrove_roots:
    case mangrove_leaves:
      return 1;
    default:
      return 15;
    }
  }

  static uint8_t LightEmissionById(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks::minecraft;
    switch (id) {
    case brewing_stand:
    case brown_mushroom:
    case dragon_egg:
    case end_portal_frame:
    case small_amethyst_bud:
    case sculk_sensor:
      return 1;
    case medium_amethyst_bud:
      return 2;
    case magma_block:
      return 3;
    case large_amethyst_bud:
      return 4;
    case amethyst_cluster:
      return 5;
    case sea_pickle:
    case smooth_quartz:
    case sponge:
    case spruce_pressure_plate:
    case sculk_catalyst:
      return 6;
    case enchanting_table:
    case ender_chest:
    case redstone_torch:
    case redstone_wall_torch:
      return 7;
    case crying_obsidian:
    case soul_torch:
    case soul_lantern:
    case soul_campfire:
    case soul_wall_torch:
    case deepslate_iron_ore:
      return 10;
    case nether_portal:
    case oak_fence:
      return 11;
    case end_rod:
    case torch:
    case wall_torch:
      return 14;
    case beacon:
    case campfire:
    case conduit:
    case end_gateway:
    case end_portal:
    case fire:
    case glowstone:
    case jack_o_lantern:
    case lantern:
    case lava:
    case sea_lantern:
    case smoker:
    case spawner:
    case spruce_planks:
    case shroomlight:
    case lava_cauldron:
    case candle:
    case light:
    case ochre_froglight:
    case verdant_froglight:
    case pearlescent_froglight:
    case mangrove_trapdoor:
    case frogspawn:
    case potted_mangrove_propagule:
    case bamboo_door:
    case bamboo_fence:
    case bamboo_fence_gate:
    case bamboo_trapdoor:
    case bamboo_wall_hanging_sign:
    case bamboo_wall_sign:
    case dark_oak_hanging_sign:
    case dark_oak_wall_hanging_sign:
    case jungle_hanging_sign:
    case piglin_head:
    case piglin_wall_head:
    case spruce_hanging_sign:
      return 15;
    default:
      return 0;
    }
  }
};

} // namespace je2be::toje::lighting
