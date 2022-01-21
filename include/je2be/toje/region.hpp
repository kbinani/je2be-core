#pragma once

namespace je2be::toje {

class Region {
public:
  std::unordered_set<Pos2i, Pos2iHasher> fChunks;

  static bool Convert(mcfile::Dimension d, std::unordered_set<Pos2i, Pos2iHasher> const &chunks, int rx, int rz, leveldb::DB &db, std::filesystem::path destination) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;
    namespace fs = std::filesystem;

    auto dir = File::CreateTempDir(fs::temp_directory_path());
    if (!dir) {
      return false;
    }

    defer {
      error_code ec;
      fs::remove_all(*dir, ec);
    };

    for (Pos2i chunk : chunks) {
      int cx = chunk.fX;
      int cz = chunk.fZ;
      auto j = mcfile::je::WritableChunk::MakeEmpty(cx, cz);
      auto b = mcfile::be::Chunk::Load(cx, cz, d, &db);
      if (!b) {
        continue;
      }

      for (auto const &sectionB : b->fSubChunks) {
        if (!sectionB) {
          continue;
        }
        auto sectionJ = mcfile::je::chunksection::ChunkSection118::MakeEmpty(sectionB->fChunkY);
        vector<shared_ptr<mcfile::je::Block const>> palette;

        for (size_t idx = 0; idx < sectionB->fPalette.size(); idx++) {
          auto const &blockB = sectionB->fPalette[idx];
          auto blockJ = BlockData::From(*blockB);
          if (!blockJ) {
            blockJ = BlockData::Identity(*blockB);
          }
          palette.push_back(blockJ);
        }

        vector<uint16_t> indices(4096);
        int indexB = 0;
        for (int x = 0; x < 16; x++) {
          for (int z = 0; z < 16; z++) {
            for (int y = 0; y < 16; y++, indexB++) {
              int indexJ = *mcfile::je::chunksection::ChunkSection118::BlockIndex(x, y, z);
              indices[indexJ] = sectionB->fPaletteIndices[indexB];
            }
          }
        }

        if (!sectionJ->fBlocks.reset(palette, indices)) {
          return false;
        }
        int sectionIndex = sectionJ->fY - j->fChunkY;
        if (j->fSections.size() <= sectionIndex) {
          j->fSections.resize(sectionIndex + 1);
        }
        j->fSections[sectionIndex] = sectionJ;
      }

      //TODO: properties of fence
      //TODO: "distance" of leaves
      //TODO: waterlogged property

      auto fos = make_shared<FileOutputStream>(*dir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz));
      if (!fos) {
        return false;
      }
      if (!j->write(*fos)) {
        return false;
      }
    }

    auto mca = destination / mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
    return mcfile::je::Region::ConcatCompressedNbt(rx, rz, *dir, mca);
  }
};

} // namespace je2be::toje
