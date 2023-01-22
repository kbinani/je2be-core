#pragma once

#include <minecraft-file.hpp>

#include "_data2d.hpp"
#include "_data3d.hpp"
#include "_optional.hpp"
#include "terraform/_block-property-accessor.hpp"
#include "terraform/java/_block-accessor-java.hpp"
#include "toje/lighting/_chunk-light-cache.hpp"
#include "toje/lighting/_light-cache.hpp"

namespace je2be::toje::lighting {

class Lighting {
  enum Model : uint32_t {
    // https://gyazo.com/a270e426c0eb2b8b53083317b5aab16f
    MODEL_SOLID = 0xffffff,
    MODEL_CLEAR = 0,
    MODEL_HALF_BOTTOM = 0xf3333,
    MODEL_HALF_TOP = 0xf0cccc,
    MODEL_BOTTOM = 0xF0000,
    MODEL_TOP = 0xF00000,

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

    int const minChunkY = minBlockY / 16;

    int const cx = out.fChunkX;
    int const cz = out.fChunkZ;

    Pos3i start(out.minBlockX() - 14, minBlockY, out.minBlockZ() - 14);
    size_t const height = maxBlockY - minBlockY + 1;

    LatticeContainerWrapper<ChunkLightingModel> models(Pos3i((cx - 1) * 16, minBlockY, (cz - 1) * 16), maxBlockY);
    for (int dz = -1; dz <= 1; dz++) {
      for (int dx = -1; dx <= 1; dx++) {
        int x = cx + dx;
        int z = cz + dz;
        auto m = make_shared<ChunkLightingModel>(x, minChunkY, z);
        models.store(x, z, m);
      }
    }

    LightingModel air(CLEAR);
    EnsureLightingModels(cache, models, cx, minChunkY, cz, blockAccessor);

    shared_ptr<Data3dSq<uint8_t, 44>> skyLight;

    if (dim == Dimension::Overworld) {
      skyLight = make_shared<Data3dSq<uint8_t, 44>>(start, height, 0);

      Data2d<optional<Volume>> skyVolumes({cx - 1, cz - 1}, 3, 3, nullopt);
      for (int dz = -1; dz <= 1; dz++) {
        for (int dx = -1; dx <= 1; dx++) {
          if (auto cached = cache.getSkyLight(cx + dx, cz + dz); cached) {
            cached->copyTo(*skyLight);
          } else {
            Pos3i start((cx + dx) * 16, minBlockY, (cz + dz) * 16);
            Pos3i end(start.fX + 15, maxBlockY, start.fZ + 15);
            Volume chunkVolume(start, end);
            if (auto part = Volume::Intersection(skyLight->volume(), chunkVolume); part) {
              skyVolumes[{cx + dx, cz + dz}] = *part;
            }
          }
        }
      }

      InitializeSkyLight(models, *skyLight, skyVolumes);
      DiffuseLight(models, *skyLight, skyVolumes);
      auto skyLightCache = ChunkLightCache::Create(cx, cz, *skyLight);
      cache.setSkyLight(cx, cz, skyLightCache);
    }

    Data3dSq<uint8_t, 44> blockLight(start, height, 0);

    Data2d<optional<Volume>> blockVolumes({cx - 1, cz - 1}, 3, 3, nullopt);
    for (int dz = -1; dz <= 1; dz++) {
      for (int dx = -1; dx <= 1; dx++) {
        if (auto cached = cache.getBlockLight(cx + dx, cz + dz); cached) {
          cached->copyTo(blockLight);
        } else {
          Pos3i start((cx + dx) * 16, minBlockY, (cz + dz) * 16);
          Pos3i end(start.fX + 15, maxBlockY, start.fZ + 15);
          Volume chunkVolume(start, end);
          if (auto part = Volume::Intersection(blockLight.volume(), chunkVolume); part) {
            blockVolumes[{cx + dx, cz + dz}] = *part;
          }
        }
      }
    }

    InitializeBlockLight(models, blockLight, blockVolumes);
    DiffuseLight(models, blockLight, blockVolumes);
    auto blockLightCache = ChunkLightCache::Create(cx, cz, blockLight);
    cache.setBlockLight(cx, cz, blockLightCache);

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
      Pos3i origin(cx * 16, bottom->y() * 16, cz * 16);
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
  static void EnsureLightingModels(
      LightCache &lightCache,
      LatticeContainerWrapper<ChunkLightingModel> &out,
      int cx,
      int cy,
      int cz,
      terraform::java::BlockAccessorJava &blockAccessor) {
    using namespace std;

    for (int dz = -1; dz <= 1; dz++) {
      for (int dx = -1; dx <= 1; dx++) {
        if (auto cachedModel = lightCache.getModel(cx + dx, cz + dz); cachedModel) {
          out.store(cx + dx, cz + dz, cachedModel);
        } else {
          auto chunk = blockAccessor.chunkAt(cx + dx, cz + dz);
          if (!chunk) {
            continue;
          }
          auto chunkModel = CopyChunkLightingModel(*chunk, cy);
          out.store(cx + dx, cz + dz, chunkModel);
          lightCache.setModel(cx + dx, cz + dz, chunkModel);
        }
      }
    }
  }

  static constexpr uint32_t MaskFacing6(Facing6 facing) {
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

  template <Facing6 Face>
  static bool CanLightPassthrough(LightingModel const &model, LightingModel const &targetModel) {
    constexpr uint32_t mask = MaskFacing6(Face);
    return (mask & ((~model.fModel) & ~Invert(targetModel.fModel))) != 0;
  }

  template <Facing6 Face>
  static bool IsFaceOpened(LightingModel const &model) {
    constexpr uint32_t mask = MaskFacing6(Face);
    return ((~model.fModel) & mask) != 0;
  }

  static void DiffuseLight(LatticeContainerWrapper<ChunkLightingModel> const &models, Data3dSq<uint8_t, 44> &out, Data2d<std::optional<Volume>> const &volumes) {
    using namespace std;

    int x0 = out.fStart.fX;
    int x1 = out.fEnd.fX;
    int y0 = out.fStart.fY;
    int y1 = out.fEnd.fY;
    int z0 = out.fStart.fZ;
    int z1 = out.fEnd.fZ;

    while (true) {
      int changed = 0;
      // up -> down
      for (int zz = volumes.fStart.fZ; zz <= volumes.fEnd.fZ; zz++) {
        for (int xx = volumes.fStart.fX; xx <= volumes.fEnd.fX; xx++) {
          auto v = volumes[{xx, zz}];
          if (!v) {
            continue;
          }
          for (int z = v->fStart.fZ; z <= v->fEnd.fZ; z++) {
            for (int x = v->fStart.fX; x <= v->fEnd.fX; x++) {
              for (int y = y1; y > y0; y--) {
                Pos3i p(x, y, z);
                Pos3i target(x, y - 1, z);
                uint8_t center = out[p];
                uint8_t down = out[target];
                if (center > down + 1 && CanLightPassthrough<Facing6::Down>(models[p], models[target])) {
                  out[target] = center - 1;
                  changed++;
                }
              }
            }
          }
        }
      }

      // down -> up
      for (int zz = volumes.fStart.fZ; zz <= volumes.fEnd.fZ; zz++) {
        for (int xx = volumes.fStart.fX; xx <= volumes.fEnd.fX; xx++) {
          auto v = volumes[{xx, zz}];
          if (!v) {
            continue;
          }
          for (int z = v->fStart.fZ; z <= v->fEnd.fZ; z++) {
            for (int x = v->fStart.fX; x <= v->fEnd.fX; x++) {
              for (int y = y0; y < y1; y++) {
                Pos3i p(x, y, z);
                Pos3i target(x, y + 1, z);
                uint8_t center = out[p];
                uint8_t up = out[target];
                if (center > up + 1 && CanLightPassthrough<Facing6::Up>(models[p], models[target])) {
                  out[target] = center - 1;
                  changed++;
                }
              }
            }
          }
        }
      }

      // west -> east
      for (int y = y0; y <= y1; y++) {
        for (int zz = volumes.fStart.fZ; zz <= volumes.fEnd.fZ; zz++) {
          for (int xx = volumes.fStart.fX; xx <= volumes.fEnd.fX; xx++) {
            auto v = volumes[{xx, zz}];
            if (!v) {
              continue;
            }
            if (auto xIntersection = ChunkLightCache::Intersection(make_pair(v->fStart.fX, v->fEnd.fX), make_pair(x0 + 1, x1)); xIntersection) {
              auto [xStart, xEnd] = *xIntersection;
              for (int z = v->fStart.fZ; z <= v->fEnd.fZ; z++) {
                for (int x = xStart; x <= xEnd; x++) {
                  Pos3i p(x - 1, y, z);
                  Pos3i target(x, y, z);
                  uint8_t center = out[p];
                  uint8_t east = out[target];
                  if (center > east + 1 && CanLightPassthrough<Facing6::East>(models[p], models[target])) {
                    out[target] = center - 1;
                    changed++;
                  }
                }
              }
            }
          }
        }
      }

      // east -> west
      for (int y = y0; y <= y1; y++) {
        for (int zz = volumes.fStart.fZ; zz <= volumes.fEnd.fZ; zz++) {
          for (int xx = volumes.fStart.fX; xx <= volumes.fEnd.fX; xx++) {
            auto v = volumes[{xx, zz}];
            if (!v) {
              continue;
            }
            if (auto xIntersection = ChunkLightCache::Intersection(make_pair(v->fStart.fX, v->fEnd.fX), make_pair(x0, x1 - 1)); xIntersection) {
              auto [xStart, xEnd] = *xIntersection;
              for (int z = v->fStart.fZ; z <= v->fEnd.fZ; z++) {
                for (int x = xEnd; x >= xStart; x--) {
                  Pos3i p(x + 1, y, z);
                  Pos3i target(x, y, z);
                  uint8_t center = out[p];
                  uint8_t west = out[target];
                  if (center > west + 1 && CanLightPassthrough<Facing6::West>(models[p], models[target])) {
                    out[target] = center - 1;
                    changed++;
                  }
                }
              }
            }
          }
        }
      }

      // north -> south
      for (int y = y0; y <= y1; y++) {
        for (int xx = volumes.fStart.fX; xx <= volumes.fEnd.fX; xx++) {
          for (int zz = volumes.fStart.fZ; zz <= volumes.fEnd.fZ; zz++) {
            auto v = volumes[{xx, zz}];
            if (!v) {
              continue;
            }
            if (auto zIntersection = ChunkLightCache::Intersection(make_pair(v->fStart.fZ, v->fEnd.fZ), make_pair(z0 + 1, z1)); zIntersection) {
              auto [zStart, zEnd] = *zIntersection;
              for (int x = v->fStart.fX; x <= v->fEnd.fX; x++) {
                for (int z = zStart; z <= zEnd; z++) {
                  Pos3i p(x, y, z - 1);
                  Pos3i target(x, y, z);
                  uint8_t center = out[p];
                  uint8_t south = out[target];
                  if (center > south + 1 && CanLightPassthrough<Facing6::South>(models[p], models[target])) {
                    out[target] = center - 1;
                    changed++;
                  }
                }
              }
            }
          }
        }
      }

      // south -> north
      for (int y = y0; y <= y1; y++) {
        for (int xx = volumes.fStart.fX; xx <= volumes.fEnd.fX; xx++) {
          for (int zz = volumes.fStart.fZ; zz <= volumes.fEnd.fZ; zz++) {
            auto v = volumes[{xx, zz}];
            if (!v) {
              continue;
            }
            if (auto zIntersection = ChunkLightCache::Intersection(make_pair(v->fStart.fZ, v->fEnd.fZ), make_pair(z0, z1 - 1)); zIntersection) {
              auto [zStart, zEnd] = *zIntersection;
              for (int x = v->fStart.fX; x <= v->fEnd.fX; x++) {
                for (int z = zEnd; z >= zStart; z--) {
                  Pos3i p(x, y, z + 1);
                  Pos3i target(x, y, z);
                  uint8_t center = out[p];
                  uint8_t north = out[target];
                  if (center > north + 1 && CanLightPassthrough<Facing6::North>(models[p], models[target])) {
                    out[target] = center - 1;
                    changed++;
                  }
                }
              }
            }
          }
        }
      }

      if (changed == 0) {
        break;
      }
    }
  }

  static void InitializeSkyLight(LatticeContainerWrapper<ChunkLightingModel> const &models, Data3dSq<uint8_t, 44> &out, Data2d<std::optional<Volume>> const &volumes) {
    for (int z = 0; z < volumes.fHeight; z++) {
      for (int x = 0; x < volumes.fWidth; x++) {
        int cx = x + volumes.fOrigin.fX;
        int cz = z + volumes.fOrigin.fZ;
        if (auto v = volumes[{cx, cz}]; v) {
          for (int bz = v->fStart.fZ; bz <= v->fEnd.fZ; bz++) {
            for (int bx = v->fStart.fX; bx <= v->fEnd.fX; bx++) {
              int by = v->fEnd.fY;
              for (; by >= v->fStart.fY; by--) {
                auto p = models[{bx, by, bz}];
                if (p.fTransparency == CLEAR) {
                  out[{bx, by, bz}] = 15;
                } else if (p.fTransparency == TRANSLUCENT && p.fBehaveAsAirWhenOpenUp && IsFaceOpened<Facing6::Up>(p)) {
                  out[{bx, by, bz}] = 15;
                } else {
                  break;
                }
                if (!IsFaceOpened<Facing6::Down>(p)) {
                  break;
                }
              }
            }
          }
        }
      }
    }
  }

  static void InitializeBlockLight(LatticeContainerWrapper<ChunkLightingModel> const &models, Data3dSq<uint8_t, 44> &out, Data2d<std::optional<Volume>> const &volumes) {
    using namespace std;

    assert(models.fStart.fX <= out.fStart.fX && out.fEnd.fX <= models.fEnd.fX);
    assert(models.fStart.fY <= out.fStart.fY && out.fEnd.fY <= models.fEnd.fY);
    assert(models.fStart.fZ <= out.fStart.fZ && out.fEnd.fZ <= models.fEnd.fZ);

    for (int j = volumes.fOrigin.fZ; j <= volumes.fEnd.fZ; j++) {
      for (int i = volumes.fOrigin.fX; i <= volumes.fEnd.fX; i++) {
        auto v = volumes[{i, j}];
        if (!v) {
          continue;
        }
        for (int y = v->fStart.fY; y <= v->fEnd.fY; y++) {
          for (int z = v->fStart.fZ - 1; z <= v->fEnd.fZ + 1; z++) {
            for (int x = v->fStart.fX - 1; x <= v->fEnd.fX + 1; x++) {
              Pos3i pos{x, y, z};
              if (!models.contains(pos)) {
                continue;
              }
              auto p = models[pos];
              if (p.fEmission == 0) {
                continue;
              }
              if (v->contains(pos) && out[pos] < p.fEmission) {
                out[pos] = p.fEmission;
              }

              Pos3i target;

              target = pos + Pos3iFromFacing6(Facing6::Up);
              if (v->contains(target)) {
                LightingModel mTarget = models[target];
                if (IsFaceOpened<Facing6::Down>(mTarget) && out[target] < p.fEmission) {
                  out[target] = p.fEmission - 1;
                }
              }

              target = pos + Pos3iFromFacing6(Facing6::Down);
              if (v->contains(target)) {
                LightingModel mTarget = models[target];
                if (IsFaceOpened<Facing6::Up>(mTarget) && out[target] < p.fEmission) {
                  out[target] = p.fEmission - 1;
                }
              }

              target = pos + Pos3iFromFacing6(Facing6::North);
              if (v->contains(target)) {
                LightingModel mTarget = models[target];
                if (IsFaceOpened<Facing6::South>(mTarget) && out[target] < p.fEmission) {
                  out[target] = p.fEmission - 1;
                }
              }

              target = pos + Pos3iFromFacing6(Facing6::East);
              if (v->contains(target)) {
                LightingModel mTarget = models[target];
                if (IsFaceOpened<Facing6::West>(mTarget) && out[target] < p.fEmission) {
                  out[target] = p.fEmission - 1;
                }
              }

              target = pos + Pos3iFromFacing6(Facing6::South);
              if (v->contains(target)) {
                LightingModel mTarget = models[target];
                if (IsFaceOpened<Facing6::North>(mTarget) && out[target] < p.fEmission) {
                  out[target] = p.fEmission - 1;
                }
              }

              target = pos + Pos3iFromFacing6(Facing6::West);
              if (v->contains(target)) {
                LightingModel mTarget = models[target];
                if (IsFaceOpened<Facing6::East>(mTarget) && out[target] < p.fEmission) {
                  out[target] = p.fEmission - 1;
                }
              }
            }
          }
        }
      }
    }
  }

  static std::shared_ptr<ChunkLightingModel> CopyChunkLightingModel(mcfile::je::Chunk const &chunk, int minChunkY) {
    using namespace std;

    auto ret = make_shared<ChunkLightingModel>(chunk.fChunkX, minChunkY, chunk.fChunkZ);

    for (auto const &section : chunk.fSections) {
      if (!section) {
        continue;
      }
      auto s = make_shared<ChunkLightingModel::Section>();
      vector<LightingModel> palette;
      section->eachBlockPalette([&](shared_ptr<mcfile::je::Block const> const &block, size_t) {
        palette.push_back(GetLightingModel(*block));
        return true;
      });
      vector<uint16_t> index;
      index.resize(4096);
      for (int y = 0; y < 16; y++) {
        for (int z = 0; z < 16; z++) {
          for (int x = 0; x < 16; x++) {
            if (auto i = section->blockPaletteIndexAt(x, y, z); i) {
              index[(y * 16 + z) * 16 + x] = *i;
            }
          }
        }
      }
      s->reset(palette, index);
      ret->setSection(section->y(), s);
    }

    return ret;
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
        m.fTransparency = TRANSLUCENT;
      }
      return m;
    } else if (block.fId == piston || block.fId == sticky_piston) {
      if (block.property("extended") == "true") {
        Facing6 f = Facing6FromJavaName(block.property("facing"));
        LightingModel m;
        m.fEmission = 0;
        switch (f) {
        case Facing6::Up:
          m.fModel = MODEL_HALF_BOTTOM;
          m.fTransparency = TRANSLUCENT;
          break;
        case Facing6::Down:
          m.fModel = MODEL_HALF_TOP;
          m.fTransparency = TRANSLUCENT;
          break;
        case Facing6::North:
          m.fModel = uint32_t(0x3CFA50);
          m.fTransparency = CLEAR;
          break;
        case Facing6::East:
          m.fModel = uint32_t(0x6AA0F5);
          m.fTransparency = CLEAR;
          break;
        case Facing6::South:
          m.fModel = uint32_t(0xC305AF);
          m.fTransparency = CLEAR;
          break;
        case Facing6::West:
          m.fModel = uint32_t(0x555F0A);
          m.fTransparency = CLEAR;
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
      switch (f) {
      case Facing6::Up:
        m.fModel = uint32_t(0xF00000);
        m.fTransparency = TRANSLUCENT;
        break;
      case Facing6::Down:
        m.fModel = uint32_t(0xF0000);
        m.fTransparency = TRANSLUCENT;
        break;
      case Facing6::North:
        m.fModel = uint32_t(0xf);
        m.fTransparency = CLEAR;
        break;
      case Facing6::East:
        m.fModel = uint32_t(0xF00);
        m.fTransparency = CLEAR;
        break;
      case Facing6::South:
        m.fModel = uint32_t(0xF000);
        m.fTransparency = CLEAR;
        break;
      case Facing6::West:
        m.fModel = uint32_t(0xF0);
        m.fTransparency = CLEAR;
        break;
      default:
        break;
      }
      return m;
    } else if (block.fId == farmland || block.fId == dirt_path) {
      LightingModel m;
      m.fEmission = 0;
      m.fTransparency = TRANSLUCENT;
      m.fModel = MODEL_HALF_BOTTOM;
      m.fBehaveAsAirWhenOpenUp = true;
      return m;
    } else if (block.fId == snow) {
      auto layers = Wrap(strings::Toi(block.property("layers", "1")), 1);
      if (layers == 8) {
        LightingModel m;
        m.fEmission = 0;
        m.fTransparency = SOLID;
        m.fModel = MODEL_SOLID;
        return m;
      } else if (layers >= 4) {
        LightingModel m;
        m.fEmission = 0;
        m.fTransparency = TRANSLUCENT;
        m.fModel = MODEL_HALF_BOTTOM;
        m.fBehaveAsAirWhenOpenUp = true;
        return m;
      } else {
        LightingModel m;
        m.fEmission = 0;
        m.fTransparency = TRANSLUCENT;
        m.fModel = MODEL_BOTTOM;
        m.fBehaveAsAirWhenOpenUp = true;
        return m;
      }
    } else if (block.fId == stonecutter || block.fId == end_portal_frame) {
      LightingModel m;
      m.fEmission = 0;
      m.fTransparency = TRANSLUCENT;
      m.fModel = MODEL_HALF_BOTTOM;
      m.fBehaveAsAirWhenOpenUp = true;
      return m;
    } else if (block.fId == lectern) {
      LightingModel m;
      m.fEmission = 0;
      m.fTransparency = TRANSLUCENT;
      m.fModel = MODEL_BOTTOM;
      m.fBehaveAsAirWhenOpenUp = true;
      return m;
    } else if (block.fId == sculk_shrieker) {
      LightingModel m;
      m.fEmission = 0;
      m.fTransparency = TRANSLUCENT;
      m.fModel = MODEL_HALF_BOTTOM;
      return m;
    } else if (block.fId == sculk_sensor) {
      LightingModel m;
      m.fEmission = 1;
      m.fTransparency = TRANSLUCENT;
      m.fModel = MODEL_HALF_BOTTOM;
      m.fBehaveAsAirWhenOpenUp = true;
      return m;
    }
    int amount = 0;
    switch (block.fId) {
    case bamboo:
    case cactus:
    case fire:
    case twisting_vines_plant:
    case weeping_vines_plant:
    case soul_fire:
    case cave_vines_plant:
    case sugar_cane:
    case fire_coral:
    case fire_coral_fan:
    case fire_coral_wall_fan:
    case tube_coral:
    case tube_coral_fan:
    case tube_coral_wall_fan:
    case brain_coral:
    case brain_coral_fan:
    case brain_coral_wall_fan:
    case bubble_coral:
    case bubble_coral_fan:
    case bubble_coral_wall_fan:
    case horn_coral:
    case horn_coral_fan:
    case horn_coral_wall_fan:
    case big_dripleaf_stem:
    case sculk_vein:
      amount = 0;
      break;
    case bubble_column:
    case kelp_plant:
    case water:
    case tall_seagrass:
    case seagrass:
    case kelp:
    case chorus_flower:
    case chorus_plant:
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
    if (block.fId == cave_vines_plant || block.fId == cave_vines) {
      if (block.property("berries") == "true") {
        return 14;
      } else {
        return 0;
      }
    } else if (block.fId == furnace || block.fId == smoker || block.fId == blast_furnace) {
      if (block.property("lit") == "true") {
        return 13;
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
    } else if (block.fId == sea_pickle) {
      if (block.property("waterlogged") == "true") {
        auto pickles = Wrap(strings::Toi(block.property("pickles", "1")), 1);
        return (pickles + 1) * 3;
      } else {
        return 0;
      }
    } else if (block.fId == light) {
      return Wrap(strings::Toi(block.property("level", "1")), 1);
    } else if (block.fId == redstone_ore || block.fId == deepslate_redstone_ore) {
      if (block.property("lit") == "true") {
        return 9;
      } else {
        return 0;
      }
    } else if (block.fId == respawn_anchor) {
      auto charges = Wrap(strings::Toi(block.property("charges", "0")), 0);
      if (charges <= 0) {
        return 0;
      } else {
        return charges * 4 - 1;
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
    case fern:
    case fire:
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
    case horn_coral:
    case horn_coral_fan:
    case horn_coral_wall_fan:
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
    case chorus_flower:
    case chorus_plant:
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
    case sculk_catalyst:
      return 6;
    case enchanting_table:
    case ender_chest:
    case redstone_torch:
    case redstone_wall_torch:
    case glow_lichen:
      return 7;
    case crying_obsidian:
    case soul_torch:
    case soul_lantern:
    case soul_campfire:
    case soul_fire:
    case soul_wall_torch:
      return 10;
    case nether_portal:
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
    case shroomlight:
    case light:
    case ochre_froglight:
    case verdant_froglight:
    case pearlescent_froglight:
      return 15;
    default:
      return 0;
    }
  }
};

} // namespace je2be::toje::lighting
