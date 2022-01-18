#include <doctest/doctest.h>
#include <je2be.hpp>

#include <iostream>

#include "CheckTag.hpp"

using namespace std;
using namespace mcfile;
using namespace mcfile::je;
namespace fs = std::filesystem;

TEST_CASE("block-data") {
  fs::path thisFile(__FILE__);
  fs::path dataDir = thisFile.parent_path() / "data";
  int ok = 0;
  int total = 0;
  for (auto it : fs::recursive_directory_iterator(dataDir / "block-data" / "1.18.1")) {
    auto path = it.path();
    if (!fs::is_regular_file(path)) {
      continue;
    }

    total++;

    fs::path filename = path.filename();
    string javaBlockData = string("minecraft:") + filename.replace_extension().string();

    auto fis = make_shared<mcfile::stream::FileInputStream>(path);
    mcfile::stream::InputStreamReader isr(fis);
    auto tag = make_shared<mcfile::nbt::CompoundTag>();
    CHECK(tag->read(isr));
    shared_ptr<mcfile::nbt::CompoundTag> bedrockBlockData = tag->compoundTag("");
    CHECK(bedrockBlockData);

    // java -> bedrock
    auto block = mcfile::je::Block::FromBlockData(javaBlockData, 2865);
    CHECK(block);
    auto convertedToBe = je2be::tobe::BlockData::From(block);
    CheckTag::Check(convertedToBe.get(), bedrockBlockData.get());

    // bedrock -> java
    auto convertedToJe = je2be::toje::BlockData::From(*bedrockBlockData);
    if (convertedToJe && convertedToJe->toString() == javaBlockData) {
      ok++;
    } else {
      cout << "-------------------------------------------------------------------------------" << endl;
      cout << "input=" << endl;
      mcfile::nbt::PrintAsJson(cout, *bedrockBlockData, {.fTypeHint = true});
      cout << "expected=" << javaBlockData << endl;
    }
    //TODO: CHECK(convertedToJe);
    //TODO: CHECK(convertedToJe->toString() == javaBlockData);
  }
  cout << ok << "/" << total << " (" << ((float)ok / (float)total * 100.0f) << "%)" << endl;
}

#if 0
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
