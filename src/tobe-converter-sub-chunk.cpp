#include <je2be/tobe/chunk-data-package.hpp>
#include <je2be/tobe/chunk-data.hpp>
#include <je2be/tobe/converter/block-data.hpp>
#include <je2be/tobe/converter/block-palette.hpp>
#include <je2be/tobe/converter/entity.hpp>
#include <je2be/tobe/converter/sub-chunk.hpp>
#include <je2be/tobe/converter/tile-entity.hpp>
#include <je2be/tobe/world-data.hpp>

namespace je2be::tobe {

class SubChunk::Impl {
  Impl() = delete;

public:
  [[nodiscard]] static bool Convert(mcfile::je::Chunk const &chunk,
                                    mcfile::Dimension dim,
                                    int chunkY,
                                    ChunkData &cd,
                                    ChunkDataPackage &cdp,
                                    WorldData &wd) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::je;
    using namespace leveldb;

    // subchunk format
    // 0: version (0x8)
    // 1: num storage blocks
    // 2~: storage blocks repeated

    // storage block format
    // 0: bbbbbbbv (bit, b = bits per block, v = version(0x0))
    // 1-
    // 1~4: paltte size (uint32 LE)
    //

    bool hasWaterlogged = false;

    shared_ptr<ChunkSection> section;
    for (int i = 0; i < chunk.fSections.size(); i++) {
      auto const &s = chunk.fSections[i];
      if (!s) {
        continue;
      }
      if (s->y() == chunkY) {
        section = s;
        break;
      }
    }

    je2be::tobe::BlockPalette palette;
    vector<bool> waterloggedIndices(4096, 0);
    int airIndex = -1;

    if (section != nullptr) {
      int const x0 = chunk.minBlockX();
      int const z0 = chunk.minBlockZ();
      int const y0 = chunkY * 16;
      int8_t altitude[16][16];
      for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
          altitude[x][z] = -1;
        }
      }
      section->eachBlockPalette([&palette, &airIndex](shared_ptr<mcfile::je::Block const> const &blockJ, size_t i) {
        auto blockB = BlockData::From(blockJ, nullptr);
        assert(blockB);
        palette.append(blockB);
        if (blockJ->fId == mcfile::blocks::minecraft::air && airIndex < 0) {
          airIndex = i;
        }
        return true;
      });
      int j = 0;
      for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
          for (int y = 0; y < 16; y++, j++) {
            if (auto index = section->blockPaletteIndexAt(x, y, z); index) {
              palette.fIndices[j] = *index;
            }
          }
        }
      }
      section->eachBlockPalette([dim, &palette, &cdp, &wd, &section, x0, y0, z0, &waterloggedIndices, &altitude, &hasWaterlogged](shared_ptr<mcfile::je::Block const> const &blockJ, size_t i) {
        if (TileEntity::IsTileEntity(blockJ->fId)) {
          int j = 0;
          for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
              for (int y = 0; y < 16; y++, j++) {
                if (palette.fIndices[j] == i) {
                  cdp.addTileBlock(x0 + x, y0 + y, z0 + z, blockJ);
                }
              }
            }
          }
        } else if (blockJ->fId == mcfile::blocks::minecraft::nether_portal) {
          bool xAxis = blockJ->property("axis", "x") == "x";
          int j = 0;
          for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
              for (int y = 0; y < 16; y++, j++) {
                if (palette.fIndices[j] == i) {
                  wd.addPortalBlock(x0 + x, y0 + y, z0 + z, xAxis);
                }
              }
            }
          }
        }
        if (blockJ->fId == mcfile::blocks::minecraft::end_portal && dim == Dimension::End) {
          int j = 0;
          for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
              for (int y = 0; y < 16; y++, j++) {
                if (palette.fIndices[j] == i) {
                  wd.addEndPortal(x0 + x, y0 + y, z0 + z);
                }
              }
            }
          }
        }
        if (!IsAir(blockJ->fId)) {
          int j = 0;
          for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
              j += altitude[x][z] + 1;
              for (int y = altitude[x][z] + 1; y < 16; y++, j++) {
                if (palette.fIndices[j] == i) {
                  altitude[x][z] = (std::max)(altitude[x][z], (int8_t)y);
                }
              }
            }
          }
        }
        if (IsWaterLogged(*blockJ)) {
          int j = 0;
          for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
              for (int y = 0; y < 16; y++, j++) {
                if (palette.fIndices[j] == i) {
                  waterloggedIndices[j] = true;
                  hasWaterlogged = true;
                }
              }
            }
          }
        }
        return true;
      });
      for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
          if (altitude[x][z] >= 0) {
            cdp.updateAltitude(x, y0 + altitude[x][z], z);
          }
        }
      }
      if (airIndex < 0) {
        airIndex = palette.size();
        palette.append(BlockData::Air());
      }
      if (airIndex != 0) {
        palette[0].swap(palette[airIndex]);
        for (int i = 0; i < 4096; i++) {
          if (palette.fIndices[i] == 0) {
            palette.fIndices[i] = airIndex;
          } else if (palette.fIndices[i] == airIndex) {
            palette.fIndices[i] = 0;
          }
        }
        airIndex = 0;
      }
    }

    for (auto it : chunk.fTileEntities) {
      Pos3i pos = it.first;
      int32_t x = pos.fX - chunk.fChunkX * 16;
      int32_t y = pos.fY - chunkY * 16;
      int32_t z = pos.fZ - chunk.fChunkZ * 16;
      if (x < 0 || 16 <= x || y < 0 || 16 <= y || z < 0 || 16 <= z) {
        continue;
      }
      int idx = (x * 16 + z) * 16 + y;
      shared_ptr<CompoundTag> const &tile = it.second;

      if (TileEntity::IsStandaloneTileEntity(tile)) {
        auto ret = TileEntity::StandaloneTileEntityBlockdData(pos, tile);
        if (!ret) {
          continue;
        }
        auto [tag, paletteKey] = *ret;
        palette.set(idx, tag);
      } else {
        // Block states may have extra properties from properties in tile entity.
        // Example: last_interacted_slot of chiseled_bookshelf is stored in tile entity on JE, but in block states on BE.
        auto blockJ = section->blockAtUnchecked(x, y, z);
        if (auto tag = BlockData::From(blockJ, tile); tag) {
          palette.set(idx, tag);
        }
      }
    }

    for (auto const &e : chunk.fEntities) {
      if (!Entity::IsTileEntity(*e)) {
        continue;
      }
      auto converted = Entity::ToTileEntityBlock(*e);
      if (!converted) {
        continue;
      }
      auto [pos, tag, paletteKey] = *converted;
      if (pos.fY < chunkY * 16 || chunkY * 16 + 16 <= pos.fY) {
        continue;
      }

      int32_t x = pos.fX - chunk.fChunkX * 16;
      int32_t y = pos.fY - chunkY * 16;
      int32_t z = pos.fZ - chunk.fChunkZ * 16;
      if (x < 0 || 16 <= x || y < 0 || 16 <= y || z < 0 || 16 <= z) {
        continue;
      }
      int idx = (x * 16 + z) * 16 + y;

      if (palette.fIndices[idx] != airIndex) {
        // Not an air block. Avoid replacing current block
        continue;
      }

      palette.set(idx, tag);
    }

    int const numStorageBlocks = hasWaterlogged ? 2 : 1;

    auto stream = make_shared<mcfile::stream::ByteStream>();
    mcfile::stream::OutputStreamWriter w(stream, Endian::Little);

    if (!w.write(kSubChunkBlockStorageVersion)) {
      return false;
    }
    if (!w.write((uint8_t)numStorageBlocks)) {
      return false;
    }
    int8_t cy = Clamp<int8_t>(chunkY);
    if (cy != chunkY) {
      return false;
    }
    if (!w.write(*(uint8_t *)&cy)) {
      return false;
    }

    {
      // layer 0
      int const numPaletteEntries = (int)palette.size();
      uint8_t bitsPerBlock;
      int blocksPerWord;
      if (numPaletteEntries <= 2) {
        bitsPerBlock = 1;
        blocksPerWord = 32;
      } else if (numPaletteEntries <= 4) {
        bitsPerBlock = 2;
        blocksPerWord = 16;
      } else if (numPaletteEntries <= 8) {
        bitsPerBlock = 3;
        blocksPerWord = 10;
      } else if (numPaletteEntries <= 16) {
        bitsPerBlock = 4;
        blocksPerWord = 8;
      } else if (numPaletteEntries <= 32) {
        bitsPerBlock = 5;
        blocksPerWord = 6;
      } else if (numPaletteEntries <= 64) {
        bitsPerBlock = 6;
        blocksPerWord = 5;
      } else if (numPaletteEntries <= 256) {
        bitsPerBlock = 8;
        blocksPerWord = 4;
      } else {
        bitsPerBlock = 16;
        blocksPerWord = 2;
      }

      if (!w.write((uint8_t)(bitsPerBlock * 2))) {
        return false;
      }
      uint32_t const mask = ~((~((uint32_t)0)) << bitsPerBlock);
      for (size_t i = 0; i < 4096; i += blocksPerWord) {
        uint32_t v = 0;
        for (size_t j = 0; j < blocksPerWord && i + j < 4096; j++) {
          uint32_t const index = (uint32_t)palette.fIndices[i + j];
          v = v | ((mask & index) << (j * bitsPerBlock));
        }
        if (!w.write(v)) {
          return false;
        }
      }

      if (!w.write((uint32_t)numPaletteEntries)) {
        return false;
      }

      for (int i = 0; i < palette.size(); i++) {
        auto const &tag = palette[i];
        if (!CompoundTag::Write(*tag, w)) {
          return false;
        }
      }
    }

    if (hasWaterlogged) {
      // layer 1
      int const numPaletteEntries = 2; // air or water
      uint8_t bitsPerBlock = 1;
      int blocksPerWord = 32;

      uint32_t const paletteAir = 0;
      uint32_t const paletteWater = 1;

      if (!w.write((uint8_t)(bitsPerBlock * 2))) {
        return false;
      }
      for (size_t i = 0; i < 4096; i += blocksPerWord) {
        uint32_t v = 0;
        for (size_t j = 0; j < blocksPerWord && i + j < 4096; j++) {
          bool waterlogged = waterloggedIndices[i + j];
          uint32_t const index = waterlogged ? paletteWater : paletteAir;
          v = v | (index << (j * bitsPerBlock));
        }
        if (!w.write(v)) {
          return false;
        }
      }

      if (!w.write((uint32_t)numPaletteEntries)) {
        return false;
      }

      auto air = BlockData::Air();
      if (!CompoundTag::Write(*air, w)) {
        return false;
      }

      auto water = BlockData::Make("water");
      auto states = Compound();
      states->set("liquid_depth", Int(0));
      water->set("states", states);
      if (!CompoundTag::Write(*water, w)) {
        return false;
      }
    }

    stream->drain(cd.fSubChunks[chunkY]);

    return true;
  }

private:
  static bool IsAir(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks;
    return id == minecraft::air || id == minecraft::cave_air || id == minecraft::void_air;
  }

  static bool IsWaterLogged(mcfile::je::Block const &block) {
    using namespace mcfile::blocks;
    if (!block.fData.empty()) {
      auto waterlogged = block.property("waterlogged", "");
      if (waterlogged == "true") {
        return true;
      }
    }
    BlockId id = block.fId;
    return id == minecraft::seagrass || id == minecraft::tall_seagrass || id == minecraft::kelp || id == minecraft::kelp_plant || id == minecraft::bubble_column;
  }
};

bool SubChunk::Convert(mcfile::je::Chunk const &chunk,
                       mcfile::Dimension dim,
                       int chunkY,
                       ChunkData &cd,
                       ChunkDataPackage &cdp,
                       WorldData &wd) {
  return Impl::Convert(chunk, dim, chunkY, cd, cdp, wd);
}

} // namespace je2be::tobe
