#pragma once

namespace je2be::box360 {

class Chunk {
  Chunk() = delete;

public:
  enum {
    kTargetDataVersion = 2865,
  };
  static std::string TargetVersionString() {
    return "1.18.1";
  }

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

    auto chunk = mcfile::je::WritableChunk::MakeEmpty(cx, 0, cz, kTargetDataVersion);
    auto f = make_shared<mcfile::stream::FileInputStream>(region);

    vector<uint8_t> buffer;
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

    uint8_t maybeEndTagMarker = buffer[0];       // 0x00. The presence of this tag prevents the file from being parsed as nbt.
    uint8_t maybeLongArrayTagMarker = buffer[1]; // 0x0c. Legacy parsers that cannot interpret the LongArrayTag will fail here.
    if (maybeEndTagMarker != 0x00 || maybeLongArrayTagMarker != 0x0c) {
      return JE2BE_ERROR;
    }
    int32_t xPos = mcfile::I32FromBE(*(int32_t *)(buffer.data() + 0x2));
    int32_t zPos = mcfile::I32FromBE(*(int32_t *)(buffer.data() + 0x6));
    int64_t maybeLastUpdate = mcfile::I64FromBE(*(int64_t *)(buffer.data() + 0x0a));
    int64_t maybeInhabitedTime = mcfile::I64FromBE(*(int64_t *)(buffer.data() + 0x12));

    uint16_t maxSectionAddress = (uint16_t)buffer[0x1b] * 0x100;
    vector<uint16_t> sectionJumpTable;
    for (int section = 0; section < 16; section++) {
      uint16_t address = mcfile::U16FromBE(*(uint16_t *)(buffer.data() + 0x1c + section * sizeof(uint16_t)));
      sectionJumpTable.push_back(address);
    }

    vector<uint8_t> maybeNumBlockPaletteEntriesFor16Sections;
    for (int section = 0; section < 16; section++) {
      uint8_t numBlockPaletteEntries = buffer[0x3c + section];
      maybeNumBlockPaletteEntriesFor16Sections.push_back(numBlockPaletteEntries);
    }

    vector<uint16_t> sectionBlocks(4096); // sectionBlocks[(y * 16 + z) * 16 + x]

    for (int section = 0; section < 16; section++) {
      int address = sectionJumpTable[section];

      if (maybeNumBlockPaletteEntriesFor16Sections[section] == 0) {
        continue;
      }
      if (address == maxSectionAddress) {
        break;
      }

      unordered_set<uint16_t> usedBlockData;
      vector<uint8_t> gridJumpTable;                                             // "grid" is a cube of 4x4x4 blocks.
      copy_n(buffer.data() + 0x4c + address, 128, back_inserter(gridJumpTable)); // [0x4c, 0xcb]
      for (int gx = 0; gx < 4; gx++) {
        for (int gz = 0; gz < 4; gz++) {
          for (int gy = 0; gy < 4; gy++) {
            int gridIndex = gx * 16 + gz * 4 + gy;

            uint8_t v1 = gridJumpTable[gridIndex * 2];
            uint8_t v2 = gridJumpTable[gridIndex * 2 + 1];
            uint16_t t1 = v1 >> 4;
            uint16_t t2 = (uint16_t)0xf & v1;
            uint16_t t3 = v2 >> 4;
            uint16_t t4 = (uint16_t)0xf & v2;

            uint16_t offset = (t4 << 8 | t1 << 4 | t2) * 4;
            uint16_t format = t3;

            uint16_t grid[64];
            uint16_t gridPosition = 0x4c + address + 0x80 + offset;

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
                  uint16_t bd = grid[idx];
                  int indexInSection = ((gy * 4 + ly) * 16 + (gz * 4 + lz)) * 16 + gx * 4 + lx;
                  sectionBlocks[indexInSection] = bd;
                  usedBlockData.insert(bd);
                }
              }
            }
          }
        }
      }

      unordered_map<uint16_t, shared_ptr<mcfile::je::Block const>> usedBlocks;
      for (uint16_t data : usedBlockData) {
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
            uint16_t data = sectionBlocks[index];
            BlockData bd(data);
            shared_ptr<mcfile::je::Block const> block;
            if (bd.extendedBlockId() == 175 && bd.data() == 10) [[unlikely]] {
              // upper half of tall flowers
              if (y == 0) {
                if (section == 0) [[unlikely]] {
                  block = bd.toBlock();
                } else {
                  if (auto lower = chunk->blockAt(cx * 16 + x, section * 16 + y - 1, cz * 16 + z); lower) {
                    map<string, string> props(lower->fProperties);
                    props["half"] = "upper";
                    block = make_shared<mcfile::je::Block const>(lower->fName, props);
                  } else {
                    block = bd.toBlock();
                  }
                }
              } else {
                int lowerIndex = ((y - 1) * 16 + z) * 16 + x;
                uint16_t lowerBlockData = sectionBlocks[lowerIndex];
                auto lower = usedBlocks[lowerBlockData];
                map<string, string> props(lower->fProperties);
                props["half"] = "upper";
                block = make_shared<mcfile::je::Block const>(lower->fName, props);
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
      uint32_t count = mcfile::U32FromBE(*(uint32_t *)(buffer.data() + pos));
      pos += 4 + 128 * (count + 1);
    }

    int heightMapStartPos = pos;
    vector<uint8_t> heightMap;
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
      for (auto &item : *te) {
        auto c = std::dynamic_pointer_cast<CompoundTag>(item);
        if (!c) {
          continue;
        }
        auto pos = props::GetPos3i<'x', 'y', 'z'>(*c);
        if (!pos) {
          continue;
        }
        auto block = chunk->blockAt(*pos);
        if (!block) {
          continue;
        }
        if (auto converted = TileEntity::Convert(*c, block, *pos, ctx); converted) {
          if (converted->fBlock) {
            chunk->setBlockAt(*pos, converted->fBlock);
          }
          if (converted->fTileEntity) {
            chunk->fTileEntities[*pos] = converted->fTileEntity;
          }
        }
      }
    }
    if (auto e = tag->listTag("Entities"); e) {
      for (auto &item : *e) {
        auto c = item->asCompound();
        if (!c) {
          continue;
        }
        auto converted = Entity::Convert(*c, ctx);
        if (!converted) {
          continue;
        }
        chunk->fEntities.push_back(converted->fEntity);
      }
    }

    result.swap(chunk);

    return Status::Ok();
  }
};

} // namespace je2be::box360
