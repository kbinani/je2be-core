#include "java/_sub-chunk.hpp"

#include "_data-version.hpp"
#include "java/_block-data.hpp"
#include "java/_block-palette.hpp"
#include "java/_chunk-data-package.hpp"
#include "java/_chunk-data.hpp"
#include "java/_entity.hpp"
#include "java/_tile-entity.hpp"
#include "java/_world-data.hpp"

namespace je2be::java {

class SubChunk::Impl {
  Impl() = delete;

public:
  [[nodiscard]] static Status Convert(mcfile::je::Chunk const &chunk,
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
    DataVersion dataVersion = cd.fDataVersion;

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

    je2be::java::BlockPalette palette;
    vector<bool> waterloggedIndices(4096, 0);
    int airIndex = -1;

    if (section != nullptr) {
      int const x0 = chunk.minBlockX();
      int const z0 = chunk.minBlockZ();
      int const y0 = chunkY * 16;
      i8 altitude[16][16];
      for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
          altitude[x][z] = -1;
        }
      }
      section->eachBlockPalette([&palette, &wd, dataVersion](shared_ptr<mcfile::je::Block const> const &blockJ, size_t i) {
        using namespace mcfile::blocks::minecraft;
        auto blockB = BlockData::From(blockJ, nullptr, dataVersion, {});
        assert(blockB);
        palette.append(blockB);
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
      section->eachBlockPalette([dim, &palette, &cdp, &wd, x0, y0, z0, &waterloggedIndices, &altitude, &hasWaterlogged](shared_ptr<mcfile::je::Block const> const &blockJ, size_t i) {
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
          bool xAxis = blockJ->property(u8"axis", u8"x") == u8"x";
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
                  altitude[x][z] = (std::max)(altitude[x][z], (i8)y);
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
      palette.resolveDuplication();
      for (size_t i = 0; i < palette.fPalette.size(); i++) {
        if (palette.fPalette[i]->string(u8"name") == u8"minecraft:air") {
          airIndex = (int)i;
          break;
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
      i32 x = pos.fX - chunk.fChunkX * 16;
      i32 y = pos.fY - chunkY * 16;
      i32 z = pos.fZ - chunk.fChunkZ * 16;
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
        if (auto tag = BlockData::From(blockJ, tile, dataVersion, {}); tag) {
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
      auto [pos, tag] = *converted;
      if (pos.fY < chunkY * 16 || chunkY * 16 + 16 <= pos.fY) {
        continue;
      }

      i32 x = pos.fX - chunk.fChunkX * 16;
      i32 y = pos.fY - chunkY * 16;
      i32 z = pos.fZ - chunk.fChunkZ * 16;
      if (x < 0 || 16 <= x || z < 0 || 16 <= z) {
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
    mcfile::stream::OutputStreamWriter w(stream, Encoding::LittleEndian);

    if (!w.write(kSubChunkBlockStorageVersion)) {
      return JE2BE_ERROR;
    }
    if (!w.write((u8)numStorageBlocks)) {
      return JE2BE_ERROR;
    }
    i8 cy = Clamp<i8>(chunkY);
    if (cy != chunkY) {
      return JE2BE_ERROR;
    }
    if (!w.write(*(u8 *)&cy)) {
      return JE2BE_ERROR;
    }

    {
      // layer 0
      int const numPaletteEntries = (int)palette.size();
      u8 bitsPerBlock;
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

      if (!w.write((u8)(bitsPerBlock * 2))) {
        return JE2BE_ERROR;
      }
      u32 const mask = ~((~((u32)0)) << bitsPerBlock);
      for (size_t i = 0; i < 4096; i += blocksPerWord) {
        u32 v = 0;
        for (size_t j = 0; j < blocksPerWord && i + j < 4096; j++) {
          u32 const index = (u32)palette.fIndices[i + j];
          v = v | ((mask & index) << (j * bitsPerBlock));
        }
        if (!w.write(v)) {
          return JE2BE_ERROR;
        }
      }

      if (!w.write((u32)numPaletteEntries)) {
        return JE2BE_ERROR;
      }

      for (int i = 0; i < palette.size(); i++) {
        auto const &tag = palette[i];
        if (!CompoundTag::Write(*tag, w)) {
          return JE2BE_ERROR;
        }
      }
    }

    if (hasWaterlogged) {
      // layer 1
      int const numPaletteEntries = 2; // air or water
      u8 bitsPerBlock = 1;
      int blocksPerWord = 32;

      u32 const paletteAir = 0;
      u32 const paletteWater = 1;

      if (!w.write((u8)(bitsPerBlock * 2))) {
        return JE2BE_ERROR;
      }
      for (size_t i = 0; i < 4096; i += blocksPerWord) {
        u32 v = 0;
        for (size_t j = 0; j < blocksPerWord && i + j < 4096; j++) {
          bool waterlogged = waterloggedIndices[i + j];
          u32 const index = waterlogged ? paletteWater : paletteAir;
          v = v | (index << (j * bitsPerBlock));
        }
        if (!w.write(v)) {
          return JE2BE_ERROR;
        }
      }

      if (!w.write((u32)numPaletteEntries)) {
        return JE2BE_ERROR;
      }

      auto air = BlockData::Air();
      if (!CompoundTag::Write(*air, w)) {
        return JE2BE_ERROR;
      }

      auto water = BlockData::Make(u8"water");
      auto states = Compound();
      states->set(u8"liquid_depth", Int(0));
      water->set(u8"states", states);
      if (!CompoundTag::Write(*water, w)) {
        return JE2BE_ERROR;
      }
    }

    stream->drain(cd.fSubChunks[chunkY]);

    return Status::Ok();
  }

private:
  static bool IsAir(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks;
    return id == minecraft::air || id == minecraft::cave_air || id == minecraft::void_air;
  }

  static bool IsWaterLogged(mcfile::je::Block const &block) {
    using namespace mcfile::blocks;
    if (!block.fData.empty()) {
      auto waterlogged = block.property(u8"waterlogged", u8"");
      if (waterlogged == u8"true") {
        return true;
      }
    }
    BlockId id = block.fId;
    return id == minecraft::seagrass || id == minecraft::tall_seagrass || id == minecraft::kelp || id == minecraft::kelp_plant || id == minecraft::bubble_column;
  }
};

Status SubChunk::Convert(mcfile::je::Chunk const &chunk,
                         mcfile::Dimension dim,
                         int chunkY,
                         ChunkData &cd,
                         ChunkDataPackage &cdp,
                         WorldData &wd) {
  return Impl::Convert(chunk, dim, chunkY, cd, cdp, wd);
}

} // namespace je2be::java
