#include <doctest/doctest.h>
#include <je2be.hpp>

using namespace std;
using namespace je2be;
namespace fs = std::filesystem;

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
                CHECK(blockA);
                CHECK(blockE);
                CHECK(blockA->fName == blockE->fName);
              }
            }
          }
        }
      }
    }
  }
}
