#include "toje/_sub-chunk.hpp"

#include "toje/_block-data.hpp"

namespace je2be::toje {

class SubChunk::Impl {
  Impl() = delete;

public:
  static std::shared_ptr<mcfile::je::ChunkSection> Convert(mcfile::be::SubChunk const &sectionB, mcfile::Dimension dim, ChunkConversionMode mode) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::biomes;

    vector<shared_ptr<mcfile::je::Block const>> paletteJ;

    vector<uint16_t> indicesJ(4096, 0);
    if (sectionB.fPaletteIndices.size() != 4096 || sectionB.fPalette.empty()) {
      paletteJ.push_back(make_shared<mcfile::je::Block const>("minecraft:air"));
    } else {
      for (size_t idx = 0; idx < sectionB.fPalette.size(); idx++) {
        auto const &blockB = sectionB.fPalette[idx];
        auto blockJ = BlockData::From(*blockB);
        assert(blockJ);
        paletteJ.push_back(blockJ);
      }
      for (int x = 0, indexB = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
          for (int y = 0; y < 16; y++, indexB++) {
            int indexJ = mcfile::je::chunksection::ChunkSection118::BlockIndex(x, y, z);
            indicesJ[indexJ] = sectionB.fPaletteIndices[indexB];
          }
        }
      }
    }

    // waterlogged=true variant of palette[index] is stored at palette[waterLogged[index]], if waterLogged[index] >= 0.
    vector<int> waterLoggedJ(paletteJ.size(), -1);

    if (sectionB.fWaterPaletteIndices.size() == 4096) {
      vector<bool> isWaterB(sectionB.fWaterPalette.size());
      for (size_t i = 0; i < sectionB.fWaterPalette.size(); i++) {
        isWaterB[i] = sectionB.fWaterPalette[i]->fName == "minecraft:water";
      }
      for (int x = 0, indexB = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
          for (int y = 0; y < 16; y++, indexB++) {
            uint16_t waterPaletteIndexB = sectionB.fWaterPaletteIndices[indexB];
            if (!isWaterB[waterPaletteIndexB]) {
              continue;
            }

            // (x, y, z) block is waterlogged
            int indexJ = mcfile::je::chunksection::ChunkSection118::BlockIndex(x, y, z);

            uint16_t indexDryJ = indicesJ[indexJ];
            auto dryBlockJ = paletteJ[indexDryJ];
            int waterLoggedIndexJ = waterLoggedJ[indexDryJ];
            if (waterLoggedIndexJ < 0) {
              if (dryBlockJ->property("waterlogged") == "") {
                // This block can't be waterlogged in Java.
                waterLoggedIndexJ = indexDryJ;
                waterLoggedJ[indexDryJ] = indexDryJ;
              } else {
                auto waterLoggedBlockJ = dryBlockJ->applying({{"waterlogged", "true"}});
                waterLoggedIndexJ = paletteJ.size();
                paletteJ.push_back(waterLoggedBlockJ);
                waterLoggedJ[indexDryJ] = waterLoggedIndexJ;
              }
            }
            indicesJ[indexJ] = waterLoggedIndexJ;
          }
        }
      }
    }

    shared_ptr<mcfile::je::ChunkSection> sectionJ;
    if (mode == ChunkConversionMode::CavesAndCliffs2) {
      auto section = mcfile::je::chunksection::ChunkSection118::MakeEmpty(sectionB.fChunkY);
      if (!section->fBlocks.reset(paletteJ, indicesJ)) {
        return nullptr;
      }
      sectionJ = section;
    } else {
      auto section = mcfile::je::chunksection::ChunkSection116::MakeEmpty(sectionB.fChunkY);
      if (!section->fBlocks.reset(paletteJ, indicesJ)) {
        return nullptr;
      }
      sectionJ = section;
    }

    return sectionJ;
  }
};

std::shared_ptr<mcfile::je::ChunkSection> SubChunk::Convert(mcfile::be::SubChunk const &sectionB, mcfile::Dimension dim, ChunkConversionMode mode) {
  return Impl::Convert(sectionB, dim, mode);
}

} // namespace je2be::toje
