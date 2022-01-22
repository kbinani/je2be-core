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

  int total = 0;
  for (auto it : fs::recursive_directory_iterator(dataDir / "block-data" / "1.18.1")) {
    auto path = it.path();
    if (!fs::is_regular_file(path)) {
      continue;
    }
    total++;
  }

  int ok = 0;
  int ng = 0;
  for (auto it : fs::recursive_directory_iterator(dataDir / "block-data" / "1.18.1")) {
    auto path = it.path();
    if (!fs::is_regular_file(path)) {
      continue;
    }

    fs::path filename = path.filename();
    string javaBlockData = string("minecraft:") + filename.replace_extension().string();

    auto fis = make_shared<mcfile::stream::FileInputStream>(path);
    mcfile::stream::InputStreamReader isr(fis);
    auto tag = make_shared<mcfile::nbt::CompoundTag>();
    CHECK(tag->read(isr));
    shared_ptr<mcfile::nbt::CompoundTag> bedrockBlockData = tag->compoundTag("");
    CHECK(bedrockBlockData);

    // java -> bedrock
    auto blockJ = mcfile::je::Block::FromBlockData(javaBlockData, 2865);
    CHECK(blockJ);
    auto convertedToBe = je2be::tobe::BlockData::From(blockJ);
    CheckTag::Check(convertedToBe.get(), bedrockBlockData.get());

    // bedrock -> java
    shared_ptr<mcfile::be::Block> blockB = mcfile::be::Block::FromCompound(*convertedToBe);
    CHECK(blockB);
    auto convertedJe = je2be::toje::BlockData::From(*blockB);
    if (convertedJe) {
      ok++;
    } else {
      cerr << "-------------------------------------------------------------------------------" << endl;
      cerr << "input=" << endl;
      mcfile::nbt::PrintAsJson(cerr, *convertedToBe, {.fTypeHint = true});
      cerr << "java=" << javaBlockData << endl;
      ng++;
      if (ng >= 10) {
        break;
      }
    }
  }
  cout << ok << "/" << total << " (" << ((float)ok / (float)total * 100.0f) << "%)" << endl;
  CHECK(ok == total);
}

#if 0
TEST_CASE("prepare-test-data") {
  fs::path thisFile(__FILE__);
  fs::path dataDir = thisFile.parent_path() / "data";
  auto file = dataDir / "debug-mode" / "1.18.1" / "r.0.0.mca";
  auto region = Region::MakeRegion(file);

  auto root = dataDir / "block-data" / "1.18.1";
  fs::remove_all(root);
  fs::create_directories(root);

  for (int cz = region->minChunkZ(); cz <= region->maxChunkZ(); cz++) {
    for (int cx = region->minChunkX(); cx <= region->maxChunkX(); cx++) {
      auto chunk = region->chunkAt(cx, cz);
      if (!chunk) {
        continue;
      }
      for (auto const &section : chunk->fSections) {
        section->eachBlockPalette([root](Block const &b) {
          string s = b.toString().substr(string("minecraft:").size());
          fs::path dir = root / s.substr(0, 1);
          fs::create_directories(dir);
          fs::path nbt = dir / (s + ".nbt");
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
