#pragma once

static std::string BlockDataTestVersion() {
  return "1.20";
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

TEST_CASE("block-data-migrate") {
  fs::path thisFile(__FILE__);
  auto dataDir = thisFile.parent_path() / "data";
  auto tmp = mcfile::File::CreateTempDir(fs::temp_directory_path());
  REQUIRE(tmp);
  defer {
    Fs::DeleteAll(*tmp);
  };

  auto before = dataDir / "1.10.1.1-upgrade-test-before.mcworld";
  auto after = dataDir / "1.10.1.1-upgrade-test-after.mcworld";
  auto beforeDir = *tmp / "before";
  auto afterDir = *tmp / "after";
  REQUIRE(Fs::CreateDirectories(beforeDir));
  REQUIRE(Fs::CreateDirectories(afterDir));

  REQUIRE(ZipFile::Unzip(before, beforeDir).ok());
  REQUIRE(ZipFile::Unzip(after, afterDir).ok());
  unique_ptr<leveldb::DB> dbB;
  unique_ptr<leveldb::DB> dbA;
  leveldb::DB *ptr = nullptr;
  REQUIRE(leveldb::DB::Open({}, beforeDir / "db", &ptr).ok());
  dbB.reset(ptr);
  ptr = nullptr;
  REQUIRE(leveldb::DB::Open({}, afterDir / "db", &ptr).ok());
  dbA.reset(ptr);
  for (int cz = 0; cz <= 0; cz++) {
    for (int cx = 0; cx <= 0; cx++) {
      auto chunkB = mcfile::be::Chunk::Load(cx, cz, mcfile::Dimension::Overworld, dbB.get(), mcfile::Endian::Little);
      auto chunkA = mcfile::be::Chunk::Load(cx, cz, mcfile::Dimension::Overworld, dbA.get(), mcfile::Endian::Little);
      for (int z = chunkB->minBlockZ(); z <= chunkB->maxBlockZ(); z++) {
        for (int x = chunkB->minBlockX(); x <= chunkB->maxBlockX(); x++) {
          for (int y = 0; y < 16; y++) {
            auto blockB = chunkB->blockAt(x, y, z);
            auto blockA = chunkA->blockAt(x, y, z);
            REQUIRE(blockB);
            REQUIRE(blockA);
            auto migratedB = std::make_shared<mcfile::be::Block>(*blockB);
            je2be::toje::BlockData::Migrate(*migratedB);
            CHECK(migratedB->fName == blockA->fName);
            if (!blockA->fStates->empty()) {
              REQUIRE(migratedB->fStates);
              auto expected = blockA->fStates->copy();
              auto actual = migratedB->fStates->copy();
              auto id = blockA->fName;
              set<u8string> ignore;
              if (id == u8"minecraft:bedrock") {
                ignore.insert(u8"infiniburn_bit");
              } else if (id == u8"minecraft:cobblestone_wall") {
                ignore.insert(u8"wall_connection_type_east");
                ignore.insert(u8"wall_connection_type_north");
                ignore.insert(u8"wall_connection_type_south");
                ignore.insert(u8"wall_connection_type_west");
                ignore.insert(u8"wall_post_bit");
              } else if (id.ends_with(u8"fence_gate")) {
                ignore.insert(u8"in_wall_bit");
                ignore.insert(u8"open_bit");
              } else if (id.ends_with(u8"stairs")) {
                ignore.insert(u8"upside_down_bit");
              } else if (id.ends_with(u8"_door")) {
                ignore.insert(u8"door_hinge_bit");
                ignore.insert(u8"open_bit");
                ignore.insert(u8"upper_block_bit");
              } else if (id.ends_with(u8"trapdoor")) {
                ignore.insert(u8"open_bit");
              }
              for (auto const &i : ignore) {
                expected->fValue.erase(i);
                actual->fValue.erase(i);
              }
              DiffCompoundTag(*expected, *actual);
            }
          }
        }
      }
    }
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
          auto converted = je2be::tobe::BlockData::From(b, nullptr);
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
