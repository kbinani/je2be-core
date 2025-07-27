#pragma once

static std::string BlockDataTestVersion() {
  return "1.21.8";
}

static int BlockDataTestDataVersion() {
  return 4440;
}

TEST_CASE("block-data") {
  fs::path thisFile(__FILE__);
  fs::path dataDir = thisFile.parent_path() / "data";
  fs::path root = dataDir / "block-data" / BlockDataTestVersion();

  auto fis = make_shared<mcfile::stream::FileInputStream>(root / "data.deflate.nbt");
  REQUIRE(fis->valid());
  auto expected = CompoundTag::ReadDeflateCompressed(*fis, mcfile::Encoding::Java);
  REQUIRE(expected);

  for (auto it : *expected) {
    auto name = it.first;
    u8string javaBlockData = Namespace::Add(name);
    auto bedrockBlockData = dynamic_pointer_cast<CompoundTag>(it.second);
    REQUIRE(bedrockBlockData);

    // java -> bedrock
    auto blockJ = mcfile::je::Block::FromBlockData(javaBlockData, BlockDataTestDataVersion());
    CHECK(blockJ);
    DataVersion dataVersion(BlockDataTestDataVersion(), BlockDataTestDataVersion());
    auto convertedToBe = je2be::java::BlockData::From(blockJ, nullptr, dataVersion, {});
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

  DataVersion const dv(BlockDataTestDataVersion(), BlockDataTestDataVersion());
  auto out = Compound();

  for (int cz = region->minChunkZ(); cz <= region->maxChunkZ(); cz++) {
    for (int cx = region->minChunkX(); cx <= region->maxChunkX(); cx++) {
      auto chunk = region->chunkAt(cx, cz);
      if (!chunk) {
        continue;
      }
      for (auto const &section : chunk->fSections) {
        section->eachBlockPalette([root, dv, &out](shared_ptr<Block const> const &b, size_t) {
          u8string s = Namespace::Remove(b->toString());
          auto converted = je2be::java::BlockData::From(b, nullptr, dv, {});
          REQUIRE(converted != nullptr);
          converted->erase(u8"version");
          out->set(s, converted);
          return true;
        });
      }
    }
  }

  auto ok = CompoundTag::WriteDeflateCompressed(*out, root / "data.deflate.nbt", mcfile::Encoding::Java);
  CHECK(ok);
}
#endif
