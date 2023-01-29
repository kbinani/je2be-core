#include "box360/_chunk.hpp"

#include <je2be/nbt.hpp>

#include "_mem.hpp"
#include "_props.hpp"
#include "box360/_biome.hpp"
#include "box360/_block-data.hpp"
#include "box360/_entity.hpp"
#include "box360/_grid.hpp"
#include "box360/_savegame.hpp"
#include "box360/_tile-entity.hpp"

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
      return ConvertV0(cx, cz, ctx, buffer, result);
    } else if (buffer[0] == 0x0 && buffer[1] == 0x0c) {
      // 0x00. The presence of this tag prevents the file from being parsed as nbt.
      // 0x0c. Legacy parsers that cannot interpret the LongArrayTag will fail here.
      return ConvertLatest(dimension, cx, cz, ctx, buffer, result);
    } else {
      return JE2BE_ERROR;
    }
  }

private:
  static Status ConvertV0(int cx, int cz, Context const &ctx, std::vector<u8> &buffer, std::shared_ptr<mcfile::je::WritableChunk> &result) {
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

    auto blockLight = level->byteArrayTag("BlockLight"); // 16384
    auto blocks = level->byteArrayTag("Blocks");         // 32768
    auto data = level->byteArrayTag("Data");             // 16384
    auto entities = level->listTag("Entities");
    auto heightMap = level->byteArrayTag("HeightMap"); // 256
    auto lastUpdate = level->longTag("LastUpdate");
    auto skyLight = level->byteArrayTag("SkyLight"); // 16384
    auto terrainPopulated = level->byte("TerrainPopulated");
    auto tileEntities = level->listTag("TileEntities");

    if (terrainPopulated != true) {
      return Status::Ok();
    }

    if (!blocks) {
      return JE2BE_ERROR;
    }
    if (blocks->fValue.size() != 32768) {
      return JE2BE_ERROR;
    }
    auto chunk = mcfile::je::WritableChunk::MakeEmpty(cx, 0, cz, kTargetDataVersion);

    for (int x = 0; x < 16; x++) {
      for (int z = 0; z < 16; z++) {
        for (int y = 0; y < 128; y++) {
          int index = (x * 16 + z) * 128 + y;
          auto block = mcfile::je::Flatten::DoFlatten(blocks->fValue[index], 0);
          if (!block) {
            continue;
          }
          chunk->setBlockAt({cx * 16 + x, y, cz * 16 + z}, block);
        }
      }
    }

    if (tileEntities) {
      ParseTileEntities(*tileEntities, *chunk, ctx);
    }
    if (entities) {
      ParseEntities(*entities, *chunk, ctx);
    }

    result.swap(chunk);

    return Status::Ok();
  }

  static Status ConvertLatest(mcfile::Dimension dimension, int cx, int cz, Context const &ctx, std::vector<u8> &buffer, std::shared_ptr<mcfile::je::WritableChunk> &result) {
    using namespace std;

    auto chunk = mcfile::je::WritableChunk::MakeEmpty(cx, 0, cz, kTargetDataVersion);

    // i32 xPos = mcfile::I32FromBE(Mem::Read<i32>(buffer, 0x2));
    // i32 zPos = mcfile::I32FromBE(Mem::Read<i32>(buffer, 0x6));
    // i64 maybeLastUpdate = mcfile::I64FromBE(Mem::Read<i64>(buffer, 0x0a));
    // i64 maybeInhabitedTime = mcfile::I64FromBE(Mem::Read<i64>(buffer, 0x12));

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

    vector<u16> sectionBlocks(4096); // sectionBlocks[(y * 16 + z) * 16 + x]

    for (int section = 0; section < 16; section++) {
      int address = sectionJumpTable[section];

      if (maybeNumBlockPaletteEntriesFor16Sections[section] == 0) {
        continue;
      }
      if (address == maxSectionAddress) {
        break;
      }

      unordered_set<u16> usedBlockData;
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
                  int indexInSection = ((gy * 4 + ly) * 16 + (gz * 4 + lz)) * 16 + gx * 4 + lx;
                  sectionBlocks[indexInSection] = bd;
                  usedBlockData.insert(bd);
                }
              }
            }
          }
        }
      }

      unordered_map<u16, shared_ptr<mcfile::je::Block const>> usedBlocks;
      for (u16 data : usedBlockData) {
        BlockData bd(data);
        if (bd.extendedBlockId() == 175 && bd.data() == 10) {
          // upper half of tall flowers
          continue;
        }
        usedBlocks[data] = bd.toBlock();
      }
      int index = 0;
      for (int y = 0; y < 16; y++) {
        for (int z = 0; z < 16; z++) {
          for (int x = 0; x < 16; x++) {
            u16 data = sectionBlocks[index];
            BlockData bd(data);
            shared_ptr<mcfile::je::Block const> block;
            if (bd.extendedBlockId() == 175 && bd.data() == 10) [[unlikely]] {
              // upper half of tall flowers
              if (y == 0) {
                if (section == 0) [[unlikely]] {
                  block = bd.toBlock();
                } else {
                  if (auto lower = chunk->blockAt(cx * 16 + x, section * 16 + y - 1, cz * 16 + z); lower) {
                    block = lower->applying({{"half", "upper"}});
                  } else {
                    block = bd.toBlock();
                  }
                }
              } else {
                int lowerIndex = ((y - 1) * 16 + z) * 16 + x;
                u16 lowerBlockData = sectionBlocks[lowerIndex];
                auto lower = usedBlocks[lowerBlockData];
                block = lower->applying({{"half", "upper"}});
              }
            } else {
              block = usedBlocks[data];
            }

            int bx = cx * 16 + x;
            int by = section * 16 + y;
            int bz = cz * 16 + z;
            chunk->setBlockAt(bx, by, bz, block);

            index++;
          }
        }
      }
    }

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
