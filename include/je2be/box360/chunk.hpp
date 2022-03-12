#pragma once

namespace je2be::box360 {

class Chunk {
  Chunk() = delete;

public:
  static bool Convert(std::filesystem::path const &region, int cx, int cz, std::shared_ptr<mcfile::je::WritableChunk> &result) {
    using namespace std;

    int rx = mcfile::Coordinate::RegionFromChunk(cx);
    int rz = mcfile::Coordinate::RegionFromBlock(cz);
    int localCx = cx - rx * 32;
    int localCz = cz - rz * 32;

    auto chunk = mcfile::je::WritableChunk::MakeEmpty(cx, 0, cz);
    auto f = make_shared<mcfile::stream::FileInputStream>(region);

    vector<uint8_t> buffer;
    if (!Savegame::ExtractRawChunkFromRegionFile(*f, localCx, localCz, buffer)) {
      return false;
    }
    if (buffer.empty()) {
      return true;
    }
    if (!Savegame::DecompressRawChunk(buffer)) {
      return false;
    }

    Savegame::DecodeDecompressedChunk(buffer);

    uint8_t maybeEndTagMarker = buffer[0];       // 0x00. The presence of this tag prevents the file from being parsed as nbt.
    uint8_t maybeLongArrayTagMarker = buffer[1]; // 0x0c. Legacy parsers that cannot interpret the LongArrayTag will fail here.
    if (maybeEndTagMarker != 0x00 || maybeLongArrayTagMarker != 0x0c) {
      return false;
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

    for (int section = 0; section < 16; section++) {
      int address = sectionJumpTable[section];

      if (address == maxSectionAddress) {
        break;
      }

      vector<uint8_t> gridJumpTable;                                             // "grid" is a cube of 4x4x4 blocks.
      copy_n(buffer.data() + 0x4c + address, 128, back_inserter(gridJumpTable)); // [0x4c, 0xcb]
      for (int gx = 0; gx < 4; gx++) {
        for (int gz = 0; gz < 4; gz++) {
          for (int gy = 0; gy < 4; gy++) {
            int gridIndex = gx * 16 + gz * 4 + gy;
            int bx = cx * 16 + gx * 4;
            int by = section * 16 + gy * 4;
            int bz = cz * 16 + gz * 4;

            uint8_t v1 = gridJumpTable[gridIndex * 2];
            uint8_t v2 = gridJumpTable[gridIndex * 2 + 1];
            uint16_t t1 = v1 >> 4;
            uint16_t t2 = (uint16_t)0xf & v1;
            uint16_t t3 = v2 >> 4;
            uint16_t t4 = (uint16_t)0xf & v2;

            uint16_t offset = (t4 << 8 | t1 << 4 | t2) * 4;
            uint16_t format = t3;

            BlockData grid[64];
            uint16_t gridPosition = 0x4c + address + 0x80 + offset;

            if (format == 0) {
              Grid::ParseFormat0(v1, v2, grid, bx, by, bz);
            } else if (format == 0xf || format == 0xe) {
              if (gridPosition + 128 >= buffer.size()) {
                return false;
              }
              Grid::ParseFormatF(buffer.data() + gridPosition, grid);
            } else if (format == 0x2) { // 1 bit
              // OK
              if (gridPosition + 12 >= buffer.size()) {
                return false;
              }
              if (!Grid::Parse<1>(buffer.data() + gridPosition, grid)) {
                return false;
              }
            } else if (format == 0x3) { // 1 bit + layers
              if (gridPosition + 20 >= buffer.size()) {
                return false;
              }
              if (!Grid::Parse<1>(buffer.data() + gridPosition, grid)) {
                return false;
              }
            } else if (format == 0x4) { // 2 bit
              if (gridPosition + 24 >= buffer.size()) {
                return false;
              }
              if (!Grid::Parse<2>(buffer.data() + gridPosition, grid)) {
                return false;
              }
            } else if (format == 0x5) { // 2 bit + layers
              if (gridPosition + 40 >= buffer.size()) {
                return false;
              }
              if (!Grid::Parse<2>(buffer.data() + gridPosition, grid)) {
                return false;
              }
            } else if (format == 0x6) { // 3 bit
              if (gridPosition + 40 >= buffer.size()) {
                return false;
              }
              if (!Grid::Parse<3>(buffer.data() + gridPosition, grid)) {
                return false;
              }
            } else if (format == 0x7) { // 3 bit + layers
              if (gridPosition + 64 >= buffer.size()) {
                return false;
              }
              if (!Grid::Parse<3>(buffer.data() + gridPosition, grid)) {
                return false;
              }
            } else if (format == 0x8) { // 4 bit
              if (gridPosition + 64 >= buffer.size()) {
                return false;
              }
              if (!Grid::Parse<4>(buffer.data() + gridPosition, grid)) {
                return false;
              }
            } else if (format == 0x9) { // 4bit + layers
              if (gridPosition + 96 >= buffer.size()) {
                return false;
              }
              if (!Grid::Parse<4>(buffer.data() + gridPosition, grid)) {
                return false;
              }
            } else {
              return false;
            }

            for (int lx = 0; lx < 4; lx++) {
              for (int lz = 0; lz < 4; lz++) {
                for (int ly = 0; ly < 4; ly++) {
                  int idx = lx * 16 + lz * 4 + ly;
                  BlockData bd = grid[idx];
                  // TODO: reduce the number of calling toBlock
                  auto block = bd.toBlock();
                  int bx = cx * 16 + gx * 4 + lx;
                  int by = section * 16 + gy * 4 + ly;
                  int bz = cz * 16 + gz * 4 + lz;
                  chunk->setBlockAt(bx, by, bz, block);
                }
              }
            }
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
      auto raw = buffer[buffer.size() - 256 - 1];
      auto biome = mcfile::biomes::FromInt(raw);
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
      return false;
    }
    auto tag = CompoundTag::Read(nbt, endian::big);
    if (!tag) {
      return false;
    }
    // TODO: tag->listTag("Entities") etc.

    result.swap(chunk);

    return true;
  }
};

} // namespace je2be::box360
