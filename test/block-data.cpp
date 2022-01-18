#include <doctest/doctest.h>
#include <je2be.hpp>

using namespace std;
using namespace mcfile;
using namespace mcfile::je;
namespace fs = std::filesystem;

#if 1
TEST_CASE("prepare-test-data") {
  fs::path thisFile(__FILE__);
  fs::path dataDir = thisFile.parent_path() / "data";
  auto file = dataDir / "debug-mode" / "1.18.1" / "r.0.0.mca";
  auto region = Region::MakeRegion(file);

  fs::create_directories(dataDir / "block-data" / "1.18.1");

  for (int cz = region->minChunkZ(); cz <= region->maxChunkZ(); cz++) {
    for (int cx = region->minChunkX(); cx <= region->maxChunkX(); cx++) {
      auto chunk = region->chunkAt(cx, cz);
      if (!chunk) {
        continue;
      }
      for (auto const &section : chunk->fSections) {
        section->eachBlockPalette([dataDir](Block const &b) {
          string s = b.toString().substr(string("minecraft:").size());
          fs::path dir = dataDir / "block-data" / "1.18.1" / s.substr(0, 1);
          fs::create_directories(dir);
          fs::path nbt = dir / (s + ".nbt");
          if (fs::exists(nbt)) {
            return true;
          }
          auto converted = je2be::tobe::BlockData::From(make_shared<Block const>(b.fName, b.fProperties));
          CHECK(converted != nullptr);
          auto fos = make_shared<mcfile::stream::FileOutputStream>(nbt);
          mcfile::stream::OutputStreamWriter osw(fos);
          CHECK(converted->writeAsRoot(osw));
          return true;
        });
      }
    }
  }
}
#endif
