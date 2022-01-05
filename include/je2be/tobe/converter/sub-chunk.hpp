#pragma once

namespace je2be::tobe {

class SubChunk {
  SubChunk() = delete;

public:
  [[nodiscard]] static bool Convert(mcfile::je::Chunk const &chunk, mcfile::Dimension dim, int chunkY, ChunkData &cd, ChunkDataPackage &cdp, WorldData &wd) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::je;
    using namespace mcfile::nbt;
    using namespace props;
    using namespace leveldb;

    if (chunk.fChunkX == -32 && chunk.fChunkZ == -32 && chunkY == -4) {
      int a = 0;
    }

    size_t const kNumBlocksInSubChunk = 16 * 16 * 16;

    vector<uint16_t> indices(kNumBlocksInSubChunk);
    vector<bool> waterloggedIndices(kNumBlocksInSubChunk);

    // subchunk format
    // 0: version (0x8)
    // 1: num storage blocks
    // 2~: storage blocks repeated

    // storage block format
    // 0: bbbbbbbv (bit, b = bits per block, v = version(0x0))
    // 1-
    // 1~4: paltte size (uint32 LE)
    //

    je2be::tobe::BlockPalette palette;

    int idx = 0;
    bool empty = true;
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
    if (section != nullptr) {
      for (int x = 0; x < 16; x++) {
        int const bx = chunk.minBlockX() + x;
        for (int z = 0; z < 16; z++) {
          int const bz = chunk.minBlockZ() + z;
          for (int y = 0; y < 16; y++, idx++) {
            int const by = chunkY * 16 + y;
            uint16_t index;
            auto block = section->blockAt(x, y, z);
            if (block && !IsAir(block->fName)) {
              string paletteKey = block->toString();
              auto found = palette.findByBlockState(paletteKey);
              if (found) {
                index = *found;
              } else {
                auto tag = BlockData::From(block);
                index = palette.add(paletteKey, tag);
              }
              empty = false;
              if (index != 0) {
                cdp.updateAltitude(x, by, z);
              }
              static string const nether_portal("minecraft:nether_portal");
              static string const end_portal("minecraft:end_portal");
              if (TileEntity::IsTileEntity(block->fName)) {
                cdp.addTileBlock(bx, by, bz, block);
              } else if (strings::Equal(block->fName, nether_portal)) {
                bool xAxis = block->property("axis", "x") == "x";
                wd.addPortalBlock(bx, by, bz, xAxis);
              }
              if (strings::Equal(block->fName, end_portal) && dim == Dimension::End) {
                wd.addEndPortal(bx, by, bz);
              }
            } else {
              index = 0;
            }

            indices[idx] = index;

            bool waterlogged = block ? IsWaterLogged(*block) : false;
            waterloggedIndices[idx] = waterlogged;
            hasWaterlogged |= waterlogged;
          }
        }
      }
    }

    for (auto it : chunk.fTileEntities) {
      Pos3i pos = it.first;
      shared_ptr<CompoundTag> const &e = it.second;
      if (!TileEntity::IsStandaloneTileEntity(e)) {
        continue;
      }
      auto ret = TileEntity::StandaloneTileEntityBlockdData(pos, e);
      if (!ret) {
        continue;
      }
      auto [tag, paletteKey] = *ret;

      if (pos.fY < chunkY * 16 || chunkY * 16 + 16 <= pos.fY) {
        continue;
      }

      int32_t x = pos.fX - chunk.fChunkX * 16;
      int32_t y = pos.fY - chunkY * 16;
      int32_t z = pos.fZ - chunk.fChunkZ * 16;
      if (x < 0 || 16 <= x || y < 0 || 16 <= y || z < 0 || 16 <= z) {
        continue;
      }
      idx = (x * 16 + z) * 16 + y;

      empty = false;

      indices[idx] = palette.add(paletteKey, tag);
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
      idx = (x * 16 + z) * 16 + y;

      if (indices[idx] != 0) {
        // Not an air block. Avoid replacing current block
        continue;
      }

      empty = false;

      indices[idx] = palette.add(paletteKey, tag);
    }

    if (empty) {
      return true;
    }

    int const numStorageBlocks = hasWaterlogged ? 2 : 1;

    auto stream = make_shared<mcfile::stream::ByteStream>();
    mcfile::stream::OutputStreamWriter w(stream, {.fLittleEndian = true});

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
      for (size_t i = 0; i < kNumBlocksInSubChunk; i += blocksPerWord) {
        uint32_t v = 0;
        for (size_t j = 0; j < blocksPerWord && i + j < kNumBlocksInSubChunk; j++) {
          uint32_t const index = (uint32_t)indices[i + j];
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
        shared_ptr<CompoundTag> const &tag = palette[i];
        if (!w.write(static_cast<uint8_t>(Tag::Type::Compound))) {
          return false;
        }
        if (!w.write(std::string())) {
          return false;
        }
        if (!tag->write(w)) {
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
      for (size_t i = 0; i < kNumBlocksInSubChunk; i += blocksPerWord) {
        uint32_t v = 0;
        for (size_t j = 0; j < blocksPerWord && i + j < kNumBlocksInSubChunk; j++) {
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
      if (!w.write(static_cast<uint8_t>(Tag::Type::Compound))) {
        return false;
      }
      if (!w.write(string())) {
        return false;
      }
      if (!air->write(w)) {
        return false;
      }

      auto water = BlockData::Make("water");
      auto states = make_shared<CompoundTag>();
      states->set("liquid_depth", Int(0));
      water->set("states", states);
      if (!w.write(static_cast<uint8_t>(Tag::Type::Compound))) {
        return false;
      }
      if (!w.write(string())) {
        return false;
      }
      if (!water->write(w)) {
        return false;
      }
    }

    stream->drain(cd.fSubChunks[chunkY]);

    int64_t currentTick = chunk.fLastUpdate;
    for (int i = 0; i < chunk.fTileTicks.size(); i++) {
      TickingBlock tb = chunk.fTileTicks[i];
      int x = tb.fX - chunk.fChunkX * 16;
      int y = tb.fY - chunkY * 16;
      int z = tb.fZ - chunk.fChunkZ * 16;
      if (x < 0 || 16 <= x || y < 0 || 16 <= y || z < 0 || 16 <= z) {
        continue;
      }
      int64_t time = currentTick + tb.fT;
      int localIndex = (x * 16 + z) * 16 + y;
      int paletteIndex = indices[localIndex];
      auto block = palette[paletteIndex];

      auto tick = make_shared<CompoundTag>();
      tick->set("blockState", block);
      tick->set("time", Long(time));
      tick->set("x", Int(tb.fX));
      tick->set("y", Int(tb.fY));
      tick->set("z", Int(tb.fZ));
      cdp.addPendingTick(i, tick);
    }

    for (int i = 0; i < chunk.fLiquidTicks.size(); i++) {
      TickingBlock tb = chunk.fLiquidTicks[i];
      int x = tb.fX - chunk.fChunkX * 16;
      int y = tb.fY - chunkY * 16;
      int z = tb.fZ - chunk.fChunkZ * 16;
      if (x < 0 || 16 <= x || y < 0 || 16 <= y || z < 0 || 16 <= z) {
        continue;
      }
      int64_t time = currentTick + tb.fT;
      int localIndex = (x * 16 + z) * 16 + y;
      int paletteIndex = indices[localIndex];
      auto block = palette[paletteIndex];

      auto tick = make_shared<CompoundTag>();
      tick->set("blockState", block);
      tick->set("time", Long(time));
      tick->set("x", Int(tb.fX));
      tick->set("y", Int(tb.fY));
      tick->set("z", Int(tb.fZ));
      cdp.addPendingTick(i, tick);
    }

    return true;
  }

private:
  static bool IsAir(std::string const &name) {
    static std::string const air("minecraft:air");
    static std::string const cave_air("minecraft:cave_air");
    static std::string const void_air("minecraft:void_air");
    return strings::Equal(name, air) || strings::Equal(name, cave_air) || strings::Equal(name, void_air);
  }

  static bool IsWaterLogged(mcfile::je::Block const &block) {
    using namespace std;
    static string const seagrass("minecraft:seagrass");
    static string const tall_seagrass("minecraft:tall_seagrass");
    static string const kelp("minecraft:kelp");
    static string const kelp_plant("minecraft:kelp_plant");
    static string const bubble_column("minecraft:bubble_column");
    if (!block.fProperties.empty()) {
      auto waterlogged = block.property("waterlogged", "");
      if (strings::Equal(waterlogged, "true")) {
        return true;
      }
    }
    auto const &name = block.fName;
    return strings::Equal(name, seagrass) || strings::Equal(name, tall_seagrass) || strings::Equal(name, kelp) || strings::Equal(name, kelp_plant) || strings::Equal(name, bubble_column);
  }
};

} // namespace je2be::tobe