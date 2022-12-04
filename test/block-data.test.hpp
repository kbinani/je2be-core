#pragma once

static std::string BlockDataTestVersion() {
  return "1.19.3";
}

TEST_CASE("block-data") {
  fs::path thisFile(__FILE__);
  fs::path dataDir = thisFile.parent_path() / "data";

  for (auto it : fs::recursive_directory_iterator(dataDir / "block-data" / BlockDataTestVersion())) {
    auto path = it.path();
    if (!fs::is_regular_file(path)) {
      continue;
    }

    fs::path filename = path.filename();
    string javaBlockData = string("minecraft:") + filename.replace_extension().string();

    auto fis = make_shared<mcfile::stream::FileInputStream>(path);
    mcfile::stream::InputStreamReader isr(fis);
    shared_ptr<mcfile::nbt::CompoundTag> bedrockBlockData = CompoundTag::Read(isr);
    CHECK(bedrockBlockData);

    // java -> bedrock
    auto blockJ = mcfile::je::Block::FromBlockData(javaBlockData, 2865);
    CHECK(blockJ);
    auto convertedToBe = je2be::tobe::BlockData::From(blockJ, nullptr);
    CheckTag::Check(convertedToBe.get(), bedrockBlockData.get());

    // bedrock -> java
    shared_ptr<mcfile::be::Block> blockB = mcfile::be::Block::FromCompound(*convertedToBe);
    CHECK(blockB);
    auto convertedJe = je2be::toje::BlockData::From(*blockB);
    CHECK(convertedJe != nullptr);
  }
}

#if 0
TEST_CASE("prepare-test-data") {
  using namespace mcfile::je;
  fs::path thisFile(__FILE__);
  fs::path dataDir = thisFile.parent_path() / "data";
  auto file = dataDir / "debug-mode" / BlockDataTestVersion() / "r.0.0.mca";
  auto region = Region::MakeRegion(file);

  auto root = dataDir / "block-data" / BlockDataTestVersion();
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
          CHECK(CompoundTag::Write(*converted, fos, mcfile::Endian::Big));
          return true;
        });
      }
    }
  }
}
#endif
