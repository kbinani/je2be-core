#pragma once

TEST_CASE("toje-legacy-block") {
  fs::path thisFile(__FILE__);
  auto dataDir = thisFile.parent_path() / "data";
  auto tmp = mcfile::File::CreateTempDir(fs::temp_directory_path());
  REQUIRE(tmp);
  defer {
    Fs::DeleteAll(*tmp);
  };

  auto before = dataDir / "1.10.1.1-upgrade-test-before.mcworld";
  auto after = dataDir / "1.10.1.1-upgrade-test-after-1.20.31.mcworld";
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
    for (int cx = 0; cx <= 4; cx++) {
      auto chunkB = mcfile::be::Chunk::Load(cx, cz, mcfile::Dimension::Overworld, dbB.get(), mcfile::Endian::Little);
      auto chunkA = mcfile::be::Chunk::Load(cx, cz, mcfile::Dimension::Overworld, dbA.get(), mcfile::Endian::Little);
      for (int z = chunkB->minBlockZ(); z <= chunkB->maxBlockZ(); z++) {
        for (int x = chunkB->minBlockX(); x <= chunkB->maxBlockX(); x++) {
          for (int y = 0; y < 48; y++) {
            auto blockB = chunkB->blockAt(x, y, z);
            auto blockA = chunkA->blockAt(x, y, z);
            if (!blockB) {
              continue;
            }

            REQUIRE(blockA);
            auto migratedB = std::make_shared<mcfile::be::Block>(*blockB);
            je2be::bedrock::LegacyBlock::Migrate(*migratedB);
            // cout << "name=" << blockB->fName << "(expected: " << blockA->fName << "); val = " << *blockB->fVal << endl;
            static unordered_set<u8string> const sIgnoreName = {
                u8"minecraft:pistonArmCollision",
            };
            if (sIgnoreName.count(migratedB->fName) == 0) {
              CHECK(migratedB->fName == blockA->fName);
            }
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
              } else if (id.ends_with(u8"_door")) {
                ignore.insert(u8"door_hinge_bit");
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
