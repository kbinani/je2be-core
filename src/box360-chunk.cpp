#include "box360/_chunk.hpp"

#include <je2be/nbt.hpp>

#include "_data2d.hpp"
#include "_data3d.hpp"
#include "_mem.hpp"
#include "_props.hpp"
#include "box360/_biome.hpp"
#include "box360/_block-data.hpp"
#include "box360/_entity.hpp"
#include "box360/_grid.hpp"
#include "box360/_savegame.hpp"
#include "box360/_tile-entity.hpp"

#include <bitset>

namespace je2be::box360 {

class Chunk::Impl {
  Impl() = delete;

public:
  static Status Convert(mcfile::Dimension dimension,
                        std::filesystem::path const &region,
                        int cx,
                        int cz,
                        std::shared_ptr<mcfile::je::WritableChunk> &result,
                        Context const &ctx,
                        Options const &options) {
    using namespace std;

    int rx = mcfile::Coordinate::RegionFromChunk(cx);
    int rz = mcfile::Coordinate::RegionFromBlock(cz);
    int localCx = cx - rx * 32;
    int localCz = cz - rz * 32;

    auto f = make_shared<mcfile::stream::FileInputStream>(region);

    vector<u8> buffer;
    if (!Savegame::ExtractRawChunkFromRegionFile(*f, localCx, localCz, buffer)) {
      return JE2BE_ERROR;
    }
    if (buffer.empty()) {
      return Status::Ok();
    }
    if (!Savegame::DecompressRawChunk(buffer)) {
      return JE2BE_ERROR;
    }

    Savegame::DecodeDecompressedChunk(buffer);

    if (buffer.size() < 2) {
      return JE2BE_ERROR;
    }
    if (buffer[0] == 0xa) {
      // TU0
      // TU9 (retail disc)
      // TU10
      // TU11
      // TU12
      // TU13 (max height changed to 255)
      // TU14
      // TU15
      // TU16
      return ConvertV0(dimension, cx, cz, ctx, buffer, result);
    } else if (buffer[0] != 0) {
      return JE2BE_ERROR;
    }

    u8 version = buffer[1];
    if (version == 0x8) {
      // TU17
      // TU18
      return ConvertV8(dimension, cx, cz, ctx, buffer, result);
    } else if (version == 0x9) {
      // TU19
      // TU25
      // TU29
      // TU30
      return ConvertV9(dimension, cx, cz, ctx, buffer, result);
    } else if (version == 0xa) {
      // TU31
      // TU33
      // TU42
      // TU58
      // TU59
      return ConvertV10(dimension, cx, cz, ctx, buffer, result);
    } else if (version == 0xb) {
      // TU60
      // TU63
      // TU67
      return ConvertV11(dimension, cx, cz, ctx, buffer, result);
    } else if (version == 0xc) {
      // TU69
      // TU70
      return ConvertV12(dimension, cx, cz, ctx, buffer, result);
    } else {
      return JE2BE_ERROR;
    }
  }

private:
  struct V8 {
    template <size_t BitPerBlock>
    static Status ParseGrid(
        Pos3i const &origin,
        std::vector<u8> const &palette,
        std::vector<u8> const &buffer,
        int offset,
        Data3dSq<u8, 16> &out) {
      using namespace std;

      if (buffer.size() < offset + BitPerBlock * 8) {
        return JE2BE_ERROR;
      }
      bitset<BitPerBlock * 64> bits;

      for (int i = 0; i < BitPerBlock * 8; i++) {
        u8 v = buffer[offset + i];
        for (int j = 0; j < 8; j++) {
          bits[i * 8 + j] = ((v >> j) & 0x1) == 0x1;
        }
      }
      offset += BitPerBlock * 8;

      for (int x = 0; x < 4; x++) {
        for (int z = 0; z < 4; z++) {
          for (int y = 0; y < 4; y++) {
            int bitOffset = ((x * 4 + z) * 4 + y) * BitPerBlock;
            u8 index = 0;
            for (size_t i = 0; i < BitPerBlock; i++) {
              index = index | (u8(bits[bitOffset + i] ? 1 : 0) << i);
            }
            if (palette.size() <= index) {
              return JE2BE_ERROR;
            }
            u8 blockId = palette[index];
            out[origin + Pos3i(x, y, z)] = blockId;
          }
        }
      }

      return Status::Ok();
    }

    static Status Parse8ChunkSections(std::vector<u8> const &buffer,
                                      int *bufferOffset,
                                      Data3dSq<u8, 16> &out,
                                      int y0) {
      using namespace std;
      if (buffer.size() < *bufferOffset + 4 + 1024) {
        return JE2BE_ERROR;
      }
      u16 size = mcfile::U16FromBE(Mem::Read<u16>(buffer, *bufferOffset + 2));
      if (buffer.size() < *bufferOffset + 4 + size) {
        return JE2BE_ERROR;
      }

      int off = *bufferOffset + 4;

      for (int gx = 0; gx < 4; gx++) {
        for (int gz = 0; gz < 4; gz++) {
          for (int gy = 0; gy < 32; gy++) {
            int index = (gx * 4 + gz) * 32 + gy;
            if (buffer.size() < off + index * 2 + 2) {
              return JE2BE_ERROR;
            }
            u8 v1 = buffer[off + index * 2];
            u8 v2 = buffer[off + index * 2 + 1];
            Pos3i origin(gx * 4, y0 + gy * 4, gz * 4);
            if (v1 == 0x7) {
              for (int x = 0; x < 4; x++) {
                for (int y = 0; y < 4; y++) {
                  for (int z = 0; z < 4; z++) {
                    out[origin + Pos3i(x, y, z)] = v2;
                  }
                }
              }
            } else {
              u16 gridOffset = 0x7ffe & (((u16(v2) << 8) | u16(v1)) >> 1);
              int offset = off + 1024 + gridOffset;
              u8 format = v1 & 0x3;
              switch (format) {
              case 0: {
                if (buffer.size() < offset + 2) {
                  return JE2BE_ERROR;
                }
                vector<u8> palette;
                palette.resize(2, 0);
                for (int i = 0; i < 2; i++) {
                  u8 id = buffer[offset + i];
                  if (id == 0xff) {
                    break;
                  }
                  palette[i] = id;
                }
                offset += 2;
                if (auto st = V8::ParseGrid<1>(origin, palette, buffer, offset, out); !st.ok()) {
                  return st;
                }
                break;
              }
              case 1: {
                if (buffer.size() < offset + 4) {
                  return JE2BE_ERROR;
                }
                vector<u8> palette;
                palette.resize(4, 0);
                for (int i = 0; i < 4; i++) {
                  u8 id = buffer[offset + i];
                  if (id == 0xff) {
                    break;
                  }
                  palette[i] = id;
                }
                offset += 4;
                if (auto st = V8::ParseGrid<2>(origin, palette, buffer, offset, out); !st.ok()) {
                  return st;
                }
                break;
              }
              case 2: {
                if (buffer.size() < off + 16) {
                  return JE2BE_ERROR;
                }
                vector<u8> palette;
                palette.resize(16, 0);
                for (int i = 0; i < 16; i++) {
                  u8 id = buffer[offset + i];
                  if (id == 0xff) {
                    break;
                  }
                  palette[i] = id;
                }
                offset += 16;
                if (auto st = V8::ParseGrid<4>(origin, palette, buffer, offset, out); !st.ok()) {
                  return st;
                }
                break;
              }
              case 3: {
                if (buffer.size() < offset + 64) {
                  return JE2BE_ERROR;
                }
                for (int x = 0; x < 4; x++) {
                  for (int z = 0; z < 4; z++) {
                    for (int y = 0; y < 4; y++) {
                      u8 id = buffer[offset + (x * 4 + z) * 4 + y];
                      out[origin + Pos3i(x, y, z)] = id;
                    }
                  }
                }
                break;
              }
              default:
                return JE2BE_ERROR;
              }
            }
          }
        }
      }

      *bufferOffset += 4 + size;

      return Status::Ok();
    }

    static Status Parse4Bit128Table(std::vector<u8> const &buffer,
                                    int *bufferOffset,
                                    Data3dSq<u8, 16> &out,
                                    int y0) {
      using namespace std;
      int offset = *bufferOffset;
      if (buffer.size() < offset + 4 + 128) {
        return JE2BE_ERROR;
      }
      u32 numLayers = mcfile::U32FromBE(mcfile::Mem::Read<u32>(buffer, offset));
      if (buffer.size() < offset + 4 + 128 + 128 * numLayers) {
        return JE2BE_ERROR;
      }
      vector<u8> layerIndices;
      layerIndices.reserve(128);
      copy_n(buffer.begin() + offset + 4, 128, back_inserter(layerIndices));
      for (int y = 0; y < 128; y++) {
        u8 layerIndex = layerIndices[y];
        if (layerIndex == 0x80) {
          for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
              out[{x, y0 + y, z}] = 0;
            }
          }
        } else if (layerIndex == 0x81) {
          for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
              out[{x, y0 + y, z}] = 0xff;
            }
          }
        } else {
          for (int x = 0; x < 16; x++) {
            for (int iz = 0; iz < 8; iz++) {
              int index = offset + 4 + 128 + layerIndex * 128 + x * 8 + iz;
              u8 data = buffer[index];
              int z = iz * 2;
              out[{x, y0 + y, z}] = 0xf & data;
              out[{x, y0 + y, z + 1}] = 0xf & (data >> 4);
            }
          }
        }
      }
      *bufferOffset += 4 + 128 + 128 * numLayers;
      return Status::Ok();
    }
  };

  static Status ConvertV8(mcfile::Dimension dim,
                          int cx,
                          int cz,
                          Context const &ctx,
                          std::vector<u8> &buffer,
                          std::shared_ptr<mcfile::je::WritableChunk> &result) {
    return ConvertV8Impl(dim, cx, cz, ctx, buffer, 0x12, result);
  }

  static Status ConvertV9(mcfile::Dimension dim,
                          int cx,
                          int cz,
                          Context const &ctx,
                          std::vector<u8> &buffer,
                          std::shared_ptr<mcfile::je::WritableChunk> &result) {
    return ConvertV8Impl(dim, cx, cz, ctx, buffer, 0x1a, result);
  }

  static Status ConvertV10(mcfile::Dimension dim,
                           int cx,
                           int cz,
                           Context const &ctx,
                           std::vector<u8> &buffer,
                           std::shared_ptr<mcfile::je::WritableChunk> &result) {
    // Same as v9. Only block id format changed such as upper half of tall flowers.
    return ConvertV8Impl(dim, cx, cz, ctx, buffer, 0x1a, result);
  }

  static Status ConvertV11(mcfile::Dimension dim,
                           int cx,
                           int cz,
                           Context const &ctx,
                           std::vector<u8> &buffer,
                           std::shared_ptr<mcfile::je::WritableChunk> &result) {
    return ConvertV8Impl(dim, cx, cz, ctx, buffer, 0x1a, result);
  }

  static Status ConvertV8Impl(mcfile::Dimension dim,
                              int cx,
                              int cz,
                              Context const &ctx,
                              std::vector<u8> &buffer,
                              int startOffset,
                              std::shared_ptr<mcfile::je::WritableChunk> &result) {
    using namespace std;

    // 1 byte: 0x0 marker
    // 1 byte: 0x9 version
    // 4 bytes: xPos (big endian)
    // 4 bytes: zPos (big endian)
    // 8 bytes: lastUpdate (big endian)
    // 8 bytes: inhabitedTime (big endian, this doen't exist when V8)
    // SubChunk: (y = 0, height = 128)
    // SubChunk: (y = 128, height = 128)
    // 4Bit128Table: (block data, y = 0, height = 128)
    // 4Bit128Table: (block data, y = 128, height = 128)
    // 4Bit128Table: (sky light, y = 0, height = 128)
    // 4Bit128Table: (sky light, y = 128, height = 128)
    // 4Bit128Table: (block light, y = 0, height = 128)
    // 4Bit128Table: (block light, y = 128, height = 128)
    // 256 bytes: height map 16x16
    // 2 bytes: terrain populated flags (big endian)
    // 256 bytes: biome 16x16
    // n bytes: nbt (to the end of file)

    auto chunk = mcfile::je::WritableChunk::MakeEmpty(cx, 0, cz, kTargetDataVersion);

    Data3dSq<u8, 16> blockId({0, 0, 0}, 256, 0);
    Data3dSq<u8, 16> blockData({0, 0, 0}, 256, 0);
    Data3dSq<u8, 16> skyLight({0, 0, 0}, 256, 0);
    Data3dSq<u8, 16> blockLight({0, 0, 0}, 256, 0);

    int offset = startOffset;
    if (auto st = V8::Parse8ChunkSections(buffer, &offset, blockId, 0); !st.ok()) {
      return st;
    }
    if (auto st = V8::Parse8ChunkSections(buffer, &offset, blockId, 128); !st.ok()) {
      return st;
    }
    if (auto st = V8::Parse4Bit128Table(buffer, &offset, blockData, 0); !st.ok()) {
      return st;
    }
    if (auto st = V8::Parse4Bit128Table(buffer, &offset, blockData, 128); !st.ok()) {
      return st;
    }
    if (auto st = V8::Parse4Bit128Table(buffer, &offset, skyLight, 0); !st.ok()) {
      return st;
    }
    if (auto st = V8::Parse4Bit128Table(buffer, &offset, skyLight, 128); !st.ok()) {
      return st;
    }
    if (auto st = V8::Parse4Bit128Table(buffer, &offset, blockLight, 0); !st.ok()) {
      return st;
    }
    if (auto st = V8::Parse4Bit128Table(buffer, &offset, blockLight, 128); !st.ok()) {
      return st;
    }

    PopulateBlocks(blockId, blockData, *chunk);

    if (buffer.size() < offset + 256) {
      return JE2BE_ERROR;
    }
    // skip height map
    offset += 256;

    if (buffer.size() < offset + 2 + 256) {
      return JE2BE_ERROR;
    }

    // u16 terrainPopulatedFlags = mcfile::U16FromBE(Mem::Read<u16>(buffer, offset));
    offset += 2;

    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        u8 b = buffer[offset + z * 16 + x];
        mcfile::biomes::BiomeId biome = Biome::FromUint32(dim, b);
        for (int y = 0; y < 256; y++) {
          chunk->setBiomeAt(cx * 16 + x, y, cz * 16 + z, biome);
        }
      }
    }
    offset += 256;

    if (buffer.size() < offset + 4) {
      return JE2BE_ERROR;
    }
    auto stream = make_shared<mcfile::stream::ByteInputStream>((char *)buffer.data() + offset, buffer.size() - offset);
    auto tag = CompoundTag::Read(stream, mcfile::Endian::Big);
    if (!tag) {
      return JE2BE_ERROR;
    }
    auto entities = tag->listTag("Entities");
    auto tileEntities = tag->listTag("TileEntities");

    if (tileEntities) {
      ParseTileEntities(*tileEntities, *chunk, ctx);
    }
    if (entities) {
      ParseEntities(*entities, *chunk, ctx);
    }

    result.swap(chunk);

    return Status::Ok();
  }

  static void PopulateBlocks(Data3dSq<u8, 16> const &blockId,
                             Data3dSq<u8, 16> const &blockData,
                             mcfile::je::WritableChunk &chunk) {
    using namespace std;
    if (blockId.fStart != blockData.fStart) {
      assert(false);
      return;
    }
    if (blockId.fEnd != blockData.fEnd) {
      assert(false);
      return;
    }
    Pos3i origin(chunk.fChunkX * 16, 0, chunk.fChunkZ * 16);
    for (int y = blockId.fStart.fY; y <= blockId.fEnd.fY; y++) {
      for (int z = blockId.fStart.fZ; z <= blockId.fEnd.fZ; z++) {
        for (int x = blockId.fStart.fX; x <= blockId.fEnd.fX; x++) {
          u8 rawId = blockId[{x, y, z}];
          u8 rawData = blockData[{x, y, z}];
          BlockData bd(rawId, rawData);
          u16 id = bd.id();
          u8 data = bd.data();
          shared_ptr<mcfile::je::Block const> block;
          if (id == 175 && data == 10 && y - 1 >= 0) {
            // upper half of tall flowers
            u8 lowerId = blockId[{x, y - 1, z}];
            u8 lowerData = blockData[{x, y - 1, z}];
            if (auto lower = mcfile::je::Flatten::Block(lowerId, lowerData); lower) {
              block = lower->applying({{"half", "upper"}});
            }
          } else if (id == 64) {
            if (y - 1 >= 0) {
              u8 lowerId = blockId[{x, y - 1, z}];
              if (lowerId == 64) {
                u8 lowerData = blockData[{x, y - 1, z}];
                map<string, string> props;
                mcfile::je::Flatten::Door(lowerData, props);
                mcfile::je::Flatten::Door(data, props);
                block = make_shared<mcfile::je::Block>("minecraft:oak_door", props);
              }
            }
            if (!block && y + 1 < 256) {
              u8 upperId = blockId[{x, y + 1, z}];
              if (upperId == 64) {
                u8 upperData = blockData[{x, y + 1, z}];
                map<string, string> props;
                mcfile::je::Flatten::Door(upperData, props);
                mcfile::je::Flatten::Door(data, props);
                block = make_shared<mcfile::je::Block>("minecraft:oak_door", props);
              }
            }
            if (!block) {
              block = bd.toBlock();
            }
          }
          if (!block) {
            block = bd.toBlock();
          }
          if (block) {
            chunk.setBlockAt(origin + Pos3i{x, y, z}, block);
          }
          if (id == 26) {
            // may be overwritten by ParseTileEntities later
            auto tag = Compound();
            tag->set("id", String("minecraft:bed"));
            tag->set("x", Int(x));
            tag->set("y", Int(y));
            tag->set("z", Int(z));
            chunk.fTileEntities[{x, y, z}] = tag;
          }
        }
      }
    }
  }

  static Status ConvertV0(mcfile::Dimension dim,
                          int cx,
                          int cz,
                          Context const &ctx,
                          std::vector<u8> &buffer,
                          std::shared_ptr<mcfile::je::WritableChunk> &result) {
    using namespace std;

    auto tag = CompoundTag::Read(buffer, mcfile::Endian::Big);
    if (!tag) {
      return JE2BE_ERROR;
    }
    auto level = tag->compoundTag("Level");
    if (!level) {
      return JE2BE_ERROR;
    }
    auto xPos = level->int32("xPos");
    auto zPos = level->int32("zPos");
    if (xPos != cx || zPos != cz) {
      return JE2BE_ERROR;
    }

    // auto blockLight = level->byteArrayTag("BlockLight"); // 16384
    auto blocks = level->byteArrayTag("Blocks"); // 32768
    auto data = level->byteArrayTag("Data");     // 16384
    auto entities = level->listTag("Entities");
    // auto heightMap = level->byteArrayTag("HeightMap"); // 256
    // auto lastUpdate = level->longTag("LastUpdate");
    // auto skyLight = level->byteArrayTag("SkyLight"); // 16384
    // auto terrainPopulated = level->byte("TerrainPopulated");
    // auto terrainPopulatedFlags = level->int16("TerrainPopulatedFlags"); // TU13 >=
    auto tileEntities = level->listTag("TileEntities");
    auto biomes = level->byteArrayTag("Biomes"); // 256

    // terrainPopulatedFlags
    // examples: 16, 20, 36, 52, 60, 62, 102, 126, 244, 254, 450, 486, 894, 970, 1002, 2046

    if (!blocks) {
      return Status::Ok();
    }
    if (blocks->fValue.size() % 256 != 0) {
      return Status::Ok();
    }
    int maxY = blocks->fValue.size() / 256;
    if (maxY != 128 && maxY != 256) {
      return JE2BE_ERROR;
    }
    if (data->fValue.size() != 128 * maxY) {
      return JE2BE_ERROR;
    }

    auto chunk = mcfile::je::WritableChunk::MakeEmpty(cx, 0, cz, kTargetDataVersion);

    Data3dSq<u8, 16> blockId({0, 0, 0}, maxY, 0);
    Data3dSq<u8, 16> blockData({0, 0, 0}, maxY, 0);

    for (int x = 0; x < 16; x++) {
      for (int z = 0; z < 16; z++) {
        for (int y = 0; y < 128; y++) {
          int index = (x * 16 + z) * 128 + y;
          u8 d = data->fValue[index / 2];
          if (index % 2 == 0) {
            d = 0xf & d;
          } else {
            d = 0xf & (d >> 4);
          }
          blockId[{x, y, z}] = blocks->fValue[index];
          blockData[{x, y, z}] = d;
        }
        for (int y = 128; y < maxY; y++) {
          int index = (x * 16 + z) * 128 + (y - 128);
          u8 d = data->fValue[16384 + index / 2];
          if (index % 2 == 0) {
            d = 0xf & d;
          } else {
            d = 0xf & (d >> 4);
          }
          blockId[{x, y, z}] = blocks->fValue[32768 + index];
          blockData[{x, y, z}] = d;
        }
      }
    }

    PopulateBlocks(blockId, blockData, *chunk);

    if (tileEntities) {
      ParseTileEntities(*tileEntities, *chunk, ctx);
    }

    if (entities) {
      ParseEntities(*entities, *chunk, ctx);
    }

    if (biomes && biomes->fValue.size() == 256) {
      for (int z = 0; z < 16; z++) {
        for (int x = 0; x < 16; x++) {
          u8 b = biomes->fValue[z * 16 + x];
          mcfile::biomes::BiomeId biome = Biome::FromUint32(dim, b);
          for (int y = 0; y < 256; y++) {
            chunk->setBiomeAt(cx * 16 + x, y, cz * 16 + z, biome);
          }
        }
      }
    } else {
      FillDefaultBiome(dim, *chunk);
    }

    result.swap(chunk);

    return Status::Ok();
  }

  static void FillDefaultBiome(mcfile::Dimension dim, mcfile::je::WritableChunk &chunk) {
    mcfile::biomes::BiomeId biome;
    switch (dim) {
    case mcfile::Dimension::End:
      biome = mcfile::biomes::minecraft::the_end;
      break;
    case mcfile::Dimension::Nether:
      biome = mcfile::biomes::minecraft::nether_wastes;
      break;
    default:
      biome = mcfile::biomes::minecraft::plains;
      break;
    }
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 256; y++) {
          chunk.setBiomeAt(chunk.fChunkX * 16 + x, y, chunk.fChunkZ * 16 + z, biome);
        }
      }
    }
  }

  static Status ConvertV12(mcfile::Dimension dimension,
                           int cx,
                           int cz,
                           Context const &ctx,
                           std::vector<u8> &buffer,
                           std::shared_ptr<mcfile::je::WritableChunk> &result) {
    using namespace std;

    auto chunk = mcfile::je::WritableChunk::MakeEmpty(cx, 0, cz, kTargetDataVersion);

    // i32 xPos = mcfile::I32FromBE(Mem::Read<i32>(buffer, 0x2));
    // i32 zPos = mcfile::I32FromBE(Mem::Read<i32>(buffer, 0x6));
    // assert(xPos == cx);
    // assert(zPos == cz);
    // i64 lastUpdate = mcfile::I64FromBE(Mem::Read<i64>(buffer, 0x0a));
    // i64 inhabitedTime = mcfile::I64FromBE(Mem::Read<i64>(buffer, 0x12));

    u16 maxSectionAddress = (u16)buffer[0x1b] * 0x100;
    vector<u16> sectionJumpTable;
    for (int section = 0; section < 16; section++) {
      u16 address = mcfile::U16FromBE(Mem::Read<u16>(buffer, 0x1c + section * sizeof(u16)));
      sectionJumpTable.push_back(address);
    }

    vector<u8> maybeNumBlockPaletteEntriesFor16Sections;
    for (int section = 0; section < 16; section++) {
      u8 numBlockPaletteEntries = buffer[0x3c + section];
      maybeNumBlockPaletteEntriesFor16Sections.push_back(numBlockPaletteEntries);
    }

    Data3dSq<u8, 16> blockId({0, 0, 0}, 256, 0);
    Data3dSq<u8, 16> blockData({0, 0, 0}, 256, 0);

    for (int section = 0; section < 16; section++) {
      int address = sectionJumpTable[section];

      if (maybeNumBlockPaletteEntriesFor16Sections[section] == 0) {
        continue;
      }
      if (address == maxSectionAddress) {
        break;
      }

      vector<u8> gridJumpTable;                                                  // "grid" is a cube of 4x4x4 blocks.
      copy_n(buffer.data() + 0x4c + address, 128, back_inserter(gridJumpTable)); // [0x4c, 0xcb]
      for (int gx = 0; gx < 4; gx++) {
        for (int gz = 0; gz < 4; gz++) {
          for (int gy = 0; gy < 4; gy++) {
            int gridIndex = gx * 16 + gz * 4 + gy;

            u8 v1 = gridJumpTable[gridIndex * 2];
            u8 v2 = gridJumpTable[gridIndex * 2 + 1];
            u16 t1 = v1 >> 4;
            u16 t2 = (u16)0xf & v1;
            u16 t3 = v2 >> 4;
            u16 t4 = (u16)0xf & v2;

            u16 offset = (t4 << 8 | t1 << 4 | t2) * 4;
            u16 format = t3;

            u16 grid[64];
            u16 gridPosition = 0x4c + address + 0x80 + offset;

            if (format == 0) {
              Grid::ParseFormat0(v1, v2, grid);
            } else if (format == 0xf || format == 0xe) {
              if (gridPosition + 128 >= buffer.size()) {
                return JE2BE_ERROR;
              }
              Grid::ParseFormatF(buffer.data() + gridPosition, grid);
            } else if (format == 0x2) { // 1 bit
              // OK
              if (gridPosition + 12 >= buffer.size()) {
                return JE2BE_ERROR;
              }
              if (!Grid::Parse<1>(buffer.data() + gridPosition, grid)) {
                return JE2BE_ERROR;
              }
            } else if (format == 0x3) { // 1 bit + layers
              if (gridPosition + 20 >= buffer.size()) {
                return JE2BE_ERROR;
              }
              if (!Grid::Parse<1>(buffer.data() + gridPosition, grid)) {
                return JE2BE_ERROR;
              }
            } else if (format == 0x4) { // 2 bit
              if (gridPosition + 24 >= buffer.size()) {
                return JE2BE_ERROR;
              }
              if (!Grid::Parse<2>(buffer.data() + gridPosition, grid)) {
                return JE2BE_ERROR;
              }
            } else if (format == 0x5) { // 2 bit + layers
              if (gridPosition + 40 >= buffer.size()) {
                return JE2BE_ERROR;
              }
              if (!Grid::Parse<2>(buffer.data() + gridPosition, grid)) {
                return JE2BE_ERROR;
              }
            } else if (format == 0x6) { // 3 bit
              if (gridPosition + 40 >= buffer.size()) {
                return JE2BE_ERROR;
              }
              if (!Grid::Parse<3>(buffer.data() + gridPosition, grid)) {
                return JE2BE_ERROR;
              }
            } else if (format == 0x7) { // 3 bit + layers
              if (gridPosition + 64 >= buffer.size()) {
                return JE2BE_ERROR;
              }
              if (!Grid::Parse<3>(buffer.data() + gridPosition, grid)) {
                return JE2BE_ERROR;
              }
            } else if (format == 0x8) { // 4 bit
              if (gridPosition + 64 >= buffer.size()) {
                return JE2BE_ERROR;
              }
              if (!Grid::Parse<4>(buffer.data() + gridPosition, grid)) {
                return JE2BE_ERROR;
              }
            } else if (format == 0x9) { // 4bit + layers
              if (gridPosition + 96 >= buffer.size()) {
                return JE2BE_ERROR;
              }
              if (!Grid::Parse<4>(buffer.data() + gridPosition, grid)) {
                return JE2BE_ERROR;
              }
            } else {
              return JE2BE_ERROR;
            }

            for (int lx = 0; lx < 4; lx++) {
              for (int lz = 0; lz < 4; lz++) {
                for (int ly = 0; ly < 4; ly++) {
                  int idx = lx * 16 + lz * 4 + ly;
                  u16 bd = grid[idx];
                  int x = gx * 4 + lx;
                  int y = section * 16 + gy * 4 + ly;
                  int z = gz * 4 + lz;
                  blockId[{x, y, z}] = 0xff & (bd >> 8);
                  blockData[{x, y, z}] = 0xff & bd;
                }
              }
            }
          }
        }
      }
    }

    PopulateBlocks(blockId, blockData, *chunk);

    int pos = maxSectionAddress + 0x4c;
    for (int i = 0; i < 4; i++) {
      u32 count = mcfile::U32FromBE(Mem::Read<u32>(buffer, pos));
      pos += 4 + 128 * (count + 1);
    }

    int heightMapStartPos = pos;
    vector<u8> heightMap;
    copy_n(buffer.data() + heightMapStartPos, 256, back_inserter(heightMap)); // When heightMap[x + z * 16] == 0, it means height = 256 at (x, z).

    vector<mcfile::biomes::BiomeId> biomes;
    for (int i = 0; i < 256; i++) {
      auto raw = buffer[heightMapStartPos + 256 + 2 + i];
      auto biome = Biome::FromUint32(dimension, raw);
      biomes.push_back(biome);
    }
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        auto biome = biomes[x + z * 16];
        int bx = cx * 16 + x;
        int bz = cz * 16 + z;
        for (int by = 0; by < 256; by++) {
          chunk->setBiomeAt(bx, by, bz, biome);
        }
      }
    }

    string nbt;
    copy(buffer.begin() + heightMapStartPos + 256 + 2 + 256, buffer.end(), back_inserter(nbt));
    if (!nbt.starts_with(string("\x0a\x00\x00", 3))) {
      return JE2BE_ERROR;
    }
    auto tag = CompoundTag::Read(nbt, mcfile::Endian::Big);
    if (!tag) {
      return JE2BE_ERROR;
    }
    if (auto te = tag->listTag("TileEntities"); te) {
      ParseTileEntities(*te, *chunk, ctx);
    }
    if (auto e = tag->listTag("Entities"); e) {
      ParseEntities(*e, *chunk, ctx);
    }

    result.swap(chunk);

    return Status::Ok();
  }

  static void ParseTileEntities(mcfile::nbt::ListTag const &tiles, mcfile::je::WritableChunk &chunk, Context const &ctx) {
    for (auto &item : tiles) {
      auto c = std::dynamic_pointer_cast<CompoundTag>(item);
      if (!c) {
        continue;
      }
      auto pos = props::GetPos3i<'x', 'y', 'z'>(*c);
      if (!pos) {
        continue;
      }
      auto block = chunk.blockAt(*pos);
      if (!block) {
        continue;
      }
      if (auto converted = TileEntity::Convert(*c, block, *pos, ctx); converted) {
        if (converted->fBlock) {
          chunk.setBlockAt(*pos, converted->fBlock);
        }
        if (converted->fTileEntity) {
          chunk.fTileEntities[*pos] = converted->fTileEntity;
        }
      }
    }
  }

  static void ParseEntities(mcfile::nbt::ListTag const &entities, mcfile::je::WritableChunk &chunk, Context const &ctx) {
    for (auto &item : entities) {
      auto c = item->asCompound();
      if (!c) {
        continue;
      }
      auto converted = Entity::Convert(*c, ctx);
      if (!converted) {
        continue;
      }
      chunk.fEntities.push_back(converted->fEntity);
    }
  }
};

Status Chunk::Convert(mcfile::Dimension dimension,
                      std::filesystem::path const &region,
                      int cx,
                      int cz,
                      std::shared_ptr<mcfile::je::WritableChunk> &result,
                      Context const &ctx,
                      Options const &options) {
  return Impl::Convert(dimension, region, cx, cz, result, ctx, options);
}

} // namespace je2be::box360
