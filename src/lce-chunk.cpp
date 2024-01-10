#include "lce/_chunk.hpp"

#include <je2be/nbt.hpp>

#include "_data2d.hpp"
#include "_data3d.hpp"
#include "_mem.hpp"
#include "_props.hpp"
#include "lce/_biome.hpp"
#include "lce/_block-data.hpp"
#include "lce/_context.hpp"
#include "lce/_entity.hpp"
#include "lce/_grid.hpp"
#include "lce/_savegame.hpp"
#include "lce/_tile-entity.hpp"

#include <bitset>
#include <mutex>
#include <variant>

namespace je2be::lce {

class Chunk::Impl {
  Impl() = delete;

public:
  static Status Convert(mcfile::Dimension dimension,
                        std::filesystem::path const &region,
                        int cx,
                        int cz,
                        std::shared_ptr<mcfile::je::WritableChunk> &result,
                        ChunkDecompressor const &chunkDecompressor,
                        Context const &ctx,
                        Options const &options) {
    using namespace std;
    auto pos = ChunkLocation(dimension, cx, cz);
    if (holds_alternative<Copy>(pos)) {
      if (auto st = Extract(dimension, region, cx, cz, result, chunkDecompressor, ctx, options); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
      if (!result) {
        return Status::Ok();
      }
      TranslateExitPortal(dimension, *result);
      return Status::Ok();
    } else if (holds_alternative<Move>(pos)) {
      auto mv = get<Move>(pos);
      if (auto st = Extract(dimension, region, cx + mv.fDx, cz + mv.fDz, result, chunkDecompressor, ctx, options); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
      if (!result) {
        return Status::Ok();
      }
      if (auto st = TranslateChunk(*result, Pos2i(-mv.fDx, -mv.fDz)); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
      TranslateExitPortal(dimension, *result);
      return Status::Ok();
    } else if (holds_alternative<Empty>(pos)) {
      auto chunk = CreateEmptyChunk(dimension, cx, cz, ctx.fNewSeaLevel);
      if (chunk) {
        result.swap(chunk);
        return Status::Ok();
      } else {
        return JE2BE_ERROR;
      }
    } else {
      return JE2BE_ERROR;
    }
  }

  static std::shared_ptr<mcfile::je::WritableChunk> CreateEmptyChunk(mcfile::Dimension dim, int cx, int cz, bool newSeaLevel) {
    using namespace std;
    static vector<shared_ptr<mcfile::je::WritableChunk const>> sCache(4);
    enum {
      OverworldOldSeaLevel = 0,
      OverworldNewSeaLevel = 1,
      Nether = 2,
      End = 3,
    };
    int index;
    switch (dim) {
    case mcfile::Dimension::Nether:
      index = Nether;
      break;
    case mcfile::Dimension::End:
      index = End;
      break;
    case mcfile::Dimension::Overworld:
    default:
      if (newSeaLevel) {
        index = OverworldNewSeaLevel;
      } else {
        index = OverworldOldSeaLevel;
      }
      break;
    }
    static mutex sMut;
    std::shared_ptr<mcfile::je::WritableChunk const> cache;
    {
      lock_guard<mutex> lock(sMut);
      if (!sCache[index]) {
        sCache[index] = CreateEmptyChunkImpl(dim, newSeaLevel);
      }
      cache = sCache[index];
    }
    std::shared_ptr<mcfile::je::WritableChunk> chunk(cache->clone());
    chunk->fChunkX = cx;
    chunk->fChunkZ = cz;
    return chunk;
  }

private:
  static std::optional<Pos2i> ChunkTranslation(mcfile::Dimension dim, int cx, int cz) {
    switch (dim) {
    case mcfile::Dimension::Nether:
      return Pos2i(0, 0);
    case mcfile::Dimension::End:
      if (cx <= -7 || 6 <= cx || cz <= 14) {
        return Pos2i(0, 0);
      } else if (cz <= 18) {
        return std::nullopt;
      } else if (cz <= 30) {
        return Pos2i(0, -4);
      } else {
        return Pos2i(0, 0);
      }
    case mcfile::Dimension::Overworld:
    default:
      return Pos2i(0, 0);
    }
  }

  struct Copy {};
  struct Move {
    int fDx;
    int fDz;
    Move(int dx, int dz) : fDx(dx), fDz(dz) {}
  };
  struct Empty {};
  static std::variant<Copy, Move, Empty> ChunkLocation(mcfile::Dimension dim, int cx, int cz) {
    switch (dim) {
    case mcfile::Dimension::Nether:
      if (cx < -9 || 8 < cx || cz < -9 || 8 < cz) {
        return Empty();
      } else {
        return Copy();
      }
    case mcfile::Dimension::End:
      // Mainland: [-6, -6]-[5, 5]
      // EndCity: [-6, 19]-[5, 30]
      if (cx <= -7 || 6 <= cx || cz <= -7) {
        return Empty();
      }
      if (cz <= 5) {
        return Copy();
      } else if (cz <= 14) {
        return Empty();
      } else if (cz <= 26) {
        return Move(0, 4);
      } else {
        return Empty();
      }
    case mcfile::Dimension::Overworld:
    default:
      if (-27 <= cx && cx <= 26 && -27 <= cz && cz <= 26) {
        return Copy();
      } else {
        return Empty();
      }
    }
  }

  static Status TranslateChunk(mcfile::je::WritableChunk &chunk, Pos2i tx) {
    chunk.fChunkX += tx.fX;
    chunk.fChunkZ += tx.fZ;

    int dx = tx.fX * 16;
    int dz = tx.fZ * 16;

    for (auto &it : chunk.fTileEntities) {
      auto tile = it.second;
      auto x = tile->int32(u8"x");
      auto z = tile->int32(u8"z");
      if (x && z) {
        tile->set(u8"x", Int(*x + dx));
        tile->set(u8"z", Int(*z + dz));
      }
    }

    for (auto &entity : chunk.fEntities) {
      for (auto const &p : {u8"Tile", u8"AP"}) {
        auto x = entity->int32(std::u8string(p) + u8"X");
        auto z = entity->int32(std::u8string(p) + u8"Z");
        if (x && z) {
          entity->set(std::u8string(p) + u8"X", Int(*x + dx));
          entity->set(std::u8string(p) + u8"Z", Int(*z + dz));
        }
      }
      auto pos = entity->listTag(u8"Pos");
      if (pos && pos->fType == Tag::Type::Double && pos->fValue.size() == 3) {
        auto x = pos->at(0)->asDouble();
        auto z = pos->at(2)->asDouble();
        if (x && z) {
          pos->fValue[0] = Double(x->fValue + dx);
          pos->fValue[2] = Double(z->fValue + dz);
        }
      }
    }

    return Status::Ok();
  }

  static void TranslateExitPortal(mcfile::Dimension dim, mcfile::je::WritableChunk &chunk) {
    for (auto &tile : chunk.fTileEntities) {
      auto exitPortal = tile.second->compoundTag(u8"ExitPortal");
      if (!exitPortal) {
        continue;
      }
      auto x = exitPortal->int32(u8"X");
      auto z = exitPortal->int32(u8"Z");
      if (!x || !z) {
        continue;
      }
      auto cx = mcfile::Coordinate::ChunkFromBlock(*x);
      auto cz = mcfile::Coordinate::ChunkFromBlock(*z);
      auto pos = ChunkTranslation(dim, cx, cz);
      if (pos && (pos->fX != 0 || pos->fZ != 0)) {
        exitPortal->set(u8"X", Int(*x + 16 * pos->fX));
        exitPortal->set(u8"Z", Int(*z + 16 * pos->fZ));
      }
    }
  }

  static Status Extract(mcfile::Dimension dimension,
                        std::filesystem::path const &region,
                        int cx,
                        int cz,
                        std::shared_ptr<mcfile::je::WritableChunk> &result,
                        ChunkDecompressor const &chunkDecompressor,
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
    if (auto st = chunkDecompressor.decompress(buffer); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }

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
                  return JE2BE_ERROR_PUSH(st);
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
                  return JE2BE_ERROR_PUSH(st);
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
                  return JE2BE_ERROR_PUSH(st);
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

    auto chunk = CreateChunk(cx, cz);

    Data3dSq<u8, 16> blockId({0, 0, 0}, 256, 0);
    Data3dSq<u8, 16> blockData({0, 0, 0}, 256, 0);
    Data3dSq<u8, 16> skyLight({0, 0, 0}, 256, 0);
    Data3dSq<u8, 16> blockLight({0, 0, 0}, 256, 0);

    int offset = startOffset;
    if (auto st = V8::Parse8ChunkSections(buffer, &offset, blockId, 0); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    if (auto st = V8::Parse8ChunkSections(buffer, &offset, blockId, 128); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    if (auto st = V8::Parse4Bit128Table(buffer, &offset, blockData, 0); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    if (auto st = V8::Parse4Bit128Table(buffer, &offset, blockData, 128); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    if (auto st = V8::Parse4Bit128Table(buffer, &offset, skyLight, 0); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    if (auto st = V8::Parse4Bit128Table(buffer, &offset, skyLight, 128); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    if (auto st = V8::Parse4Bit128Table(buffer, &offset, blockLight, 0); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    if (auto st = V8::Parse4Bit128Table(buffer, &offset, blockLight, 128); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
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
    auto entities = tag->listTag(u8"Entities");
    auto tileEntities = tag->listTag(u8"TileEntities");

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
            if (auto lower = mcfile::je::Flatten::Block(lowerId, lowerData, chunk.getDataVersion()); lower) {
              block = lower->applying({{u8"half", u8"upper"}});
            }
          } else if (id == 64) {
            if (y - 1 >= 0) {
              u8 lowerId = blockId[{x, y - 1, z}];
              if (lowerId == 64) {
                u8 lowerData = blockData[{x, y - 1, z}];
                map<u8string, u8string> props;
                mcfile::je::Flatten::Door(lowerData, props);
                mcfile::je::Flatten::Door(data, props);
                block = mcfile::je::Block::FromIdAndProperties(mcfile::blocks::minecraft::oak_door, Chunk::kTargetDataVersion, props);
              }
            }
            if (!block && y + 1 < 256) {
              u8 upperId = blockId[{x, y + 1, z}];
              if (upperId == 64) {
                u8 upperData = blockData[{x, y + 1, z}];
                map<u8string, u8string> props;
                mcfile::je::Flatten::Door(upperData, props);
                mcfile::je::Flatten::Door(data, props);
                block = mcfile::je::Block::FromIdAndProperties(mcfile::blocks::minecraft::oak_door, Chunk::kTargetDataVersion, props);
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
            tag->set(u8"id", u8"minecraft:bed");
            tag->set(u8"x", Int(x));
            tag->set(u8"y", Int(y));
            tag->set(u8"z", Int(z));
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
    auto level = tag->compoundTag(u8"Level");
    if (!level) {
      return JE2BE_ERROR;
    }
    auto xPos = level->int32(u8"xPos");
    auto zPos = level->int32(u8"zPos");
    if (xPos != cx || zPos != cz) {
      return JE2BE_ERROR;
    }

    // auto blockLight = level->byteArrayTag(u8"BlockLight"); // 16384
    auto blocks = level->byteArrayTag(u8"Blocks"); // 32768
    auto data = level->byteArrayTag(u8"Data");     // 16384
    auto entities = level->listTag(u8"Entities");
    // auto heightMap = level->byteArrayTag(u8"HeightMap"); // 256
    // auto lastUpdate = level->longTag(u8"LastUpdate");
    // auto skyLight = level->byteArrayTag(u8"SkyLight"); // 16384
    // auto terrainPopulated = level->byte(u8"TerrainPopulated");
    // auto terrainPopulatedFlags = level->int16(u8"TerrainPopulatedFlags"); // TU13 >=
    auto tileEntities = level->listTag(u8"TileEntities");
    auto biomes = level->byteArrayTag(u8"Biomes"); // 256

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

    auto chunk = CreateChunk(cx, cz);

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

  static mcfile::biomes::BiomeId DefaultBiome(mcfile::Dimension dim) {
    switch (dim) {
    case mcfile::Dimension::End:
      return mcfile::biomes::minecraft::the_end;
    case mcfile::Dimension::Nether:
      return mcfile::biomes::minecraft::nether_wastes;
    default:
      return mcfile::biomes::minecraft::plains;
    }
  }

  static void FillDefaultBiome(mcfile::Dimension dim, mcfile::je::WritableChunk &chunk) {
    auto biome = DefaultBiome(dim);
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

    auto chunk = CreateChunk(cx, cz);

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
    if (auto te = tag->listTag(u8"TileEntities"); te) {
      ParseTileEntities(*te, *chunk, ctx);
    }
    if (auto e = tag->listTag(u8"Entities"); e) {
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

  static std::shared_ptr<mcfile::je::WritableChunk> CreateChunk(int cx, int cz) {
    auto chunk = mcfile::je::WritableChunk::MakeEmpty(cx, 0, cz, kTargetDataVersion);
    chunk->fLegacyBiomes.resize(1024);
    return chunk;
  }

  static std::shared_ptr<mcfile::je::WritableChunk> CreateEmptyChunkImpl(mcfile::Dimension dim, bool newSeaLevel) {
    auto chunk = CreateChunk(0, 0);
    FillDefaultBiome(dim, *chunk);
    chunk->fSections.clear();
    for (int y = 0; y < 256; y++) {
      mcfile::blocks::BlockId blockId = mcfile::blocks::minecraft::air;
      if (dim == mcfile::Dimension::Overworld) {
        if (y == 0) {
          blockId = mcfile::blocks::minecraft::bedrock;
        } else if (y <= 53) {
          blockId = mcfile::blocks::minecraft::stone;
        } else {
          if (newSeaLevel) {
            if (y <= 62) {
              blockId = mcfile::blocks::minecraft::water;
            }
          } else {
            if (y <= 63) {
              blockId = mcfile::blocks::minecraft::water;
            }
          }
        }
      }
      auto block = mcfile::je::Block::FromId(blockId, Chunk::kTargetDataVersion);
      for (int z = 0; z < 16; z++) {
        for (int x = 0; x < 16; x++) {
          chunk->setBlockAt(x, y, z, block);
        }
      }
    }
    return chunk;
  }
};

Status Chunk::Convert(mcfile::Dimension dimension,
                      std::filesystem::path const &region,
                      int cx,
                      int cz,
                      std::shared_ptr<mcfile::je::WritableChunk> &result,
                      ChunkDecompressor const &chunkDecompressor,
                      Context const &ctx,
                      Options const &options) {
  return Impl::Convert(dimension, region, cx, cz, result, chunkDecompressor, ctx, options);
}

std::shared_ptr<mcfile::je::WritableChunk> Chunk::CreateEmptyChunk(mcfile::Dimension dim, int cx, int cz, bool newSeaLevel) {
  return Impl::CreateEmptyChunk(dim, cx, cz, newSeaLevel);
}

} // namespace je2be::lce
