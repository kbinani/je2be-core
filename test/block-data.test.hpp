#pragma once

static std::string BlockDataTestVersion() {
  return "1.21";
}

static int BlockDataTestDataVersion() {
  return 3953;
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
    u8string javaBlockData = u8string(u8"minecraft:") + filename.replace_extension().u8string();

    auto fis = make_shared<mcfile::stream::FileInputStream>(path);
    mcfile::stream::InputStreamReader isr(fis);
    shared_ptr<mcfile::nbt::CompoundTag> bedrockBlockData = CompoundTag::Read(isr);
    CHECK(bedrockBlockData);

    // java -> bedrock
    auto blockJ = mcfile::je::Block::FromBlockData(javaBlockData, BlockDataTestDataVersion());
    CHECK(blockJ);
    auto convertedToBe = je2be::java::BlockData::From(blockJ, nullptr, {});
    convertedToBe->erase(u8"version");
    CheckTag::Check(convertedToBe.get(), bedrockBlockData.get());

    // bedrock -> java
    shared_ptr<mcfile::be::Block> blockB = mcfile::be::Block::FromCompound(*convertedToBe);
    CHECK(blockB);
    auto convertedJe = je2be::bedrock::BlockData::From(*blockB, BlockDataTestDataVersion());
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
        section->eachBlockPalette([root](shared_ptr<Block const> const &b, size_t) {
          u8string s = b->toString().substr(u8string(u8"minecraft:").size());
          fs::path dir = root / s.substr(0, 1);
          fs::create_directories(dir);
          fs::path nbt = dir / (s + u8".nbt");
          auto converted = je2be::java::BlockData::From(b, nullptr, {});
          REQUIRE(converted != nullptr);
          converted->erase(u8"version");
          auto fos = make_shared<mcfile::stream::FileOutputStream>(nbt);
          CHECK(CompoundTag::Write(*converted, fos, mcfile::Encoding::Java));
          return true;
        });
      }
    }
  }
}
#endif
