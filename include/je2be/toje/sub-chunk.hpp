#pragma once

namespace je2be::toje {

class SubChunk {
  SubChunk() = delete;

public:
  static std::shared_ptr<mcfile::je::chunksection::ChunkSection118> Convert(mcfile::be::SubChunk const &sectionB) {
    using namespace std;
    auto sectionJ = mcfile::je::chunksection::ChunkSection118::MakeEmpty(sectionB.fChunkY);
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
            int indexJ = *mcfile::je::chunksection::ChunkSection118::BlockIndex(x, y, z);
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
            int indexJ = *mcfile::je::chunksection::ChunkSection118::BlockIndex(x, y, z);

            uint16_t indexDryJ = indicesJ[indexJ];
            auto dryBlockJ = paletteJ[indexDryJ];
            int waterLoggedIndexJ = waterLoggedJ[indexDryJ];
            if (waterLoggedIndexJ < 0) {
              if (dryBlockJ->fProperties.find("waterlogged") == dryBlockJ->fProperties.end()) {
                // This block can't be waterlogged in Java.
                waterLoggedIndexJ = indexDryJ;
                waterLoggedJ[indexDryJ] = indexDryJ;
              } else {
                map<string, string> props(dryBlockJ->fProperties);
                props["waterlogged"] = "true";
                auto waterLoggedBlockJ = make_shared<mcfile::je::Block const>(dryBlockJ->fName, props);
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

    if (!sectionJ->fBlocks.reset(paletteJ, indicesJ)) {
      return nullptr;
    }

    return sectionJ;
  }
};

} // namespace je2be::toje
