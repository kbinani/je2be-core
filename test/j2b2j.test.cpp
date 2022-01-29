#include <doctest/doctest.h>
#include <je2be.hpp>

using namespace std;
using namespace je2be;
namespace fs = std::filesystem;

static void CheckBlock(mcfile::je::Block const &e, mcfile::je::Block const &a, std::initializer_list<std::string> ignore = {}) {
  using namespace std;
  CHECK(e.fName == a.fName);
  map<string, string> propsE(e.fProperties);
  map<string, string> propsA(a.fProperties);
  for (std::string const &p : ignore) {
    propsE.erase(p);
    propsA.erase(p);
  }
  CHECK(propsE.size() == propsA.size());
  for (auto it : propsE) {
    auto found = propsA.find(it.first);
    CHECK(found != propsA.end());
    CHECK(found->second == it.second);
  }
}

TEST_CASE("j2b2j") {
  fs::path thisFile(__FILE__);
  auto dataDir = thisFile.parent_path() / "data";
  auto in = dataDir / "je2be-test";
  auto tmp = mcfile::File::CreateTempDir(fs::temp_directory_path());
  CHECK(tmp);
  defer {
    fs::remove_all(*tmp);
  };

  // java -> bedrock
  auto outB = mcfile::File::CreateTempDir(*tmp);
  CHECK(outB);
  je2be::tobe::InputOption io;
  for (int cx = 0; cx < 18; cx++) {
    io.fChunkFilter.insert(Pos2i(cx, 0));
  }
  io.fDimensionFilter.insert(mcfile::Dimension::Overworld);
  je2be::tobe::OutputOption oo;
  je2be::tobe::Converter tobe(in, io, *outB, oo);
  CHECK(tobe.run(thread::hardware_concurrency()));

  // bedrock -> java
  auto outJ = mcfile::File::CreateTempDir(*tmp);
  CHECK(outJ);
  je2be::toje::Converter toje(*outB, *outJ);
  CHECK(toje.run(thread::hardware_concurrency()));

  // Compare initial Java input and final Java output.

  unordered_map<string, string> fallbackJtoB;
  fallbackJtoB["minecraft:petrified_oak_slab"] = "minecraft:oak_slab"; // does not exist in BE. should be replaced to oak_slab when java -> bedrock.
  fallbackJtoB["minecraft:cave_air"] = "minecraft:air";

  unordered_map<string, string> fallbackBtoJ;
  fallbackBtoJ["minecraft:frame"] = "minecraft:air";      // frame should be converted as an entity.
  fallbackBtoJ["minecraft:glow_frame"] = "minecraft:air"; // frame should be converted as an entity.

  for (auto dim : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
    if (!io.fDimensionFilter.empty()) {
      if (io.fDimensionFilter.find(dim) == io.fDimensionFilter.end()) {
        continue;
      }
    }
    auto regionDirA = io.getWorldDirectory(*outJ, dim) / "region";
    auto regionDirE = io.getWorldDirectory(in, dim) / "region";
    for (auto it : fs::directory_iterator(regionDirA)) {
      if (!it.is_regular_file()) {
        continue;
      }
      auto fileA = it.path();
      if (fileA.extension() != ".mca") {
        continue;
      }
      auto regionA = mcfile::je::Region::MakeRegion(fileA);
      CHECK(regionA);

      auto fileE = regionDirE / fileA.filename();
      auto regionE = mcfile::je::Region::MakeRegion(fileE);
      CHECK(regionE);

      for (int cz = regionA->minChunkZ(); cz <= regionA->maxChunkZ(); cz++) {
        for (int cx = regionA->minChunkX(); cx <= regionA->maxChunkX(); cx++) {
          if (!io.fChunkFilter.empty()) {
            if (io.fChunkFilter.find(Pos2i(cx, cz)) == io.fChunkFilter.end()) {
              continue;
            }
          }
          auto chunkA = regionA->chunkAt(cx, cz);
          if (!chunkA) {
            continue;
          }
          auto chunkE = regionE->chunkAt(cx, cz);
          CHECK(chunkE);

          CHECK(chunkA->minBlockY() == chunkE->minBlockY());
          CHECK(chunkA->maxBlockY() == chunkE->maxBlockY());

          for (int y = chunkE->minBlockY(); y <= chunkE->maxBlockY(); y++) {
            for (int z = chunkE->minBlockZ(); z <= chunkE->maxBlockZ(); z++) {
              for (int x = chunkE->minBlockX(); x <= chunkE->maxBlockX(); x++) {
                auto blockA = chunkA->blockAt(x, y, z);
                auto blockE = chunkE->blockAt(x, y, z);
                if (blockA && blockE) {
                  if (blockE->fName == "minecraft:piston_head" || blockE->fName == "minecraft:piston" || blockE->fName == "minecraft:sticky_piston") {
                    continue; //TODO: remove this
                  }
                  if (blockE->fName == "minecraft:chest" || blockE->fName == "minecraft:trapped_chest") {
                    continue; //TODO: remove this
                  }
                  if (blockA->fName.ends_with("shulker_box")) {
                    continue; //TODO: remove this
                  }
                  if (blockE->fName == "minecraft:note_block") {
                    continue; //TODO: pickup "note" from block entity
                  }
                  if (blockE->fName == "minecraft:lectern") {
                    continue; //TODO: pickup "has_book" from block entity
                  }
                  auto foundJtoB = fallbackJtoB.find(blockE->fName);
                  if (foundJtoB == fallbackJtoB.end()) {
                    auto foundBtoJ = fallbackBtoJ.find(blockA->fName);
                    if (foundBtoJ == fallbackBtoJ.end()) {
                      if (blockE->fName.ends_with("_leaves")) {
                        CheckBlock(*blockE, *blockA, {"distance"});
                      } else if (blockE->fName == "minecraft:redstone_wall_torch" || blockE->fName == "minecraft:redstone_torch") {
                        CheckBlock(*blockE, *blockA, {"lit"});
                      } else if (blockE->fName == "minecraft:red_mushroom_block" || blockE->fName == "minecraft:brown_mushroom_block" || blockE->fName == "minecraft:sculk_sensor") {
                        CHECK(blockE->fName == blockA->fName);
                      } else if (blockE->fName == "minecraft:scaffolding") {
                        CheckBlock(*blockE, *blockA, {"distance"});
                      } else if (blockE->fName == "minecraft:repeater") {
                        CheckBlock(*blockE, *blockA, {"locked"});
                      } else if (blockE->fName == "minecraft:note_block" || blockE->fName.ends_with("_trapdoor") || blockE->fName.ends_with("_fence_gate") || blockE->fName == "minecraft:lectern" || blockE->fName.ends_with("_door") || blockE->fName == "minecraft:lightning_rod") {
                        CheckBlock(*blockE, *blockA, {"powered"});
                      } else if (blockE->fName.ends_with("_button")) {
                        CheckBlock(*blockE, *blockA, {"facing", "powered"});
                      } else {
                        CHECK(blockA->toString() == blockE->toString());
                      }
                    } else {
                      CHECK(foundBtoJ->second == blockE->fName);
                    }
                  } else {
                    CHECK(blockA->fName == foundJtoB->second);
                  }
                } else if (blockA) {
                  CHECK(blockA->fName == "minecraft:air");
                } else if (blockE) {
                  CHECK(blockE->fName == "minecraft:air");
                }
              }
            }
          }

          for (auto const &it : chunkE->fTileEntities) {
            Pos3i pos = it.first;
            shared_ptr<mcfile::nbt::CompoundTag> const &tileE = it.second;
            auto id = tileE->string("id");
            static unordered_set<string> const sWhitelist{
                "minecraft:banner", "minecraft:skull", "minecraft:bed", "minecraft:jukebox"};
            if (sWhitelist.find(*id) == sWhitelist.end()) {
              continue; //TODO: remove this
            }
            auto found = chunkA->fTileEntities.find(pos);
            CHECK(found != chunkA->fTileEntities.end());
            auto tileA = found->second;
            ostringstream e;
            mcfile::nbt::PrintAsJson(e, *tileE, {.fTypeHint = true});
            ostringstream a;
            mcfile::nbt::PrintAsJson(a, *tileA, {.fTypeHint = true});
            string es = e.str();
            string as = a.str();
            if (es == as) {
              CHECK(true);
            } else {
              cerr << "actual:" << endl;
              cerr << as << endl;
              cerr << "expected:" << endl;
              cerr << es << endl;
              vector<string> linesE = mcfile::String::Split(es, '\n');
              vector<string> linesA = mcfile::String::Split(as, '\n');
              for (int i = 0; i < std::min(linesE.size(), linesA.size()); i++) {
                string lineE = linesE[i];
                string lineA = linesA[i];
                string prefix = "#" + to_string(i);
                CHECK(prefix + lineA == prefix + lineE);
              }
            }
          }
        }
      }
    }
  }
}
