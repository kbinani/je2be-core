#pragma once

static std::optional<fs::path> JavaToBedrock(fs::path const &java) {
  auto output = File::CreateTempDir(fs::temp_directory_path());
  if (!output) {
    return nullopt;
  }
  je2be::java::Options o;
  o.fDimensionFilter.insert(mcfile::Dimension::Overworld);
  o.fChunkFilter.insert(Pos2i(0, 0));
  if (auto st = je2be::java::Converter::Run(java, *output, o, thread::hardware_concurrency()); !st.ok()) {
    return nullopt;
  }
  return *output;
}

static std::optional<fs::path> BedrockToJava(fs::path const &bedrock) {
  auto output = File::CreateTempDir(fs::temp_directory_path());
  if (!output) {
    return nullopt;
  }
  je2be::bedrock::Options o;
  o.fDimensionFilter.insert(mcfile::Dimension::Overworld);
  o.fChunkFilter.insert(Pos2i(0, 0));
  if (auto st = je2be::bedrock::Converter::Run(bedrock, *output, o, thread::hardware_concurrency()); !st.ok()) {
    return nullopt;
  }
  return *output;
}

static void CheckBedrockBlock(mcfile::be::Block const &a, mcfile::be::Block const &b) {
  CHECK(a.fName == b.fName);
  bool aEmpty = a.fStates ? a.fStates->empty() : true;
  bool bEmpty = b.fStates ? b.fStates->empty() : true;
  CHECK(aEmpty == bEmpty);
  if (aEmpty != bEmpty) {
    return;
  }
  if (aEmpty) {
    return;
  }
  ostringstream as;
  mcfile::nbt::PrintAsJson(as, *a.fStates, {.fTypeHint = true});
  ostringstream bs;
  mcfile::nbt::PrintAsJson(bs, *b.fStates, {.fTypeHint = true});
  CHECK(as.str() == bs.str());
}

static void DrainAttachedBlocks(CompoundTag &tag, unordered_set<Pos3i, Pos3iHasher> &buffer) {
  auto blocks = tag.listTag(u8"AttachedBlocks");
  if (!blocks) {
    return;
  }
  REQUIRE(blocks->size() % 3 == 0);
  for (int i = 0; i < blocks->size(); i += 3) {
    auto x = blocks->at(i)->asInt();
    auto y = blocks->at(i + 1)->asInt();
    auto z = blocks->at(i + 2)->asInt();
    REQUIRE(x);
    REQUIRE(y);
    REQUIRE(z);
    Pos3i pos(x->fValue, y->fValue, z->fValue);
    buffer.insert(pos);
  }
  tag.erase(u8"AttachedBlocks");
}

static void CheckMovingPistonTileEntity(CompoundTag const &e, CompoundTag const &a, i8 state) {
  auto copyE = e.copy();
  auto copyA = a.copy();

  if (e.string(u8"id") == u8"PistonArm") {
    i8 eState = e.byte(u8"State", -1);
    REQUIRE(eState == state);
    i8 aState = a.byte(u8"State", -1);
    CHECK(aState == state);
  }

  unordered_set<Pos3i, Pos3iHasher> attachedBlocksE;
  unordered_set<Pos3i, Pos3iHasher> attachedBlocksA;
  DrainAttachedBlocks(*copyE, attachedBlocksE);
  DrainAttachedBlocks(*copyA, attachedBlocksA);
  for (u8string const &tag : {u8"movingBlock/version", u8"movingBlockExtra/version"}) {
    Erase(copyE, tag);
    Erase(copyA, tag);
  }
  DiffCompoundTag(*copyE, *copyA);
  CHECK(attachedBlocksE.size() == attachedBlocksA.size());
  if (attachedBlocksE.size() != attachedBlocksA.size()) {
    return;
  }
  for (Pos3i const &pos : attachedBlocksE) {
    CHECK(attachedBlocksA.find(pos) != attachedBlocksA.end());
  }
}

static void CheckMovingPiston(fs::path const &java, fs::path const &bedrock, i8 state) {
  auto javaReferenceDir = File::CreateTempDir(fs::temp_directory_path());
  REQUIRE(javaReferenceDir);
  REQUIRE(ZipFile::Unzip(java, *javaReferenceDir).ok());

  auto bedrockReferenceDir = File::CreateTempDir(fs::temp_directory_path());
  REQUIRE(bedrockReferenceDir);
  REQUIRE(ZipFile::Unzip(bedrock, *bedrockReferenceDir).ok());

  { // Java to Bedrock
    auto output = JavaToBedrock(*javaReferenceDir);
    REQUIRE(output);

    unique_ptr<leveldb::DB> actualDb(OpenF(*output / "db"));
    REQUIRE(actualDb);
    unique_ptr<leveldb::DB> expectedDb(OpenF(*bedrockReferenceDir / "db"));
    REQUIRE(expectedDb);

    auto actual = mcfile::be::Chunk::Load(0, 0, mcfile::Dimension::Overworld, actualDb.get(), mcfile::Endian::Little);
    REQUIRE(actual);
    auto expected = mcfile::be::Chunk::Load(0, 0, mcfile::Dimension::Overworld, expectedDb.get(), mcfile::Endian::Little);
    REQUIRE(expected);

    for (int y = 0; y < 4; y++) {
      auto actualBlock = actual->blockAt(0, y, 0);
      auto expectedBlock = expected->blockAt(0, y, 0);
      REQUIRE(actualBlock);
      REQUIRE(expectedBlock);
      CheckBedrockBlock(*expectedBlock, *actualBlock);

      auto actualTile = actual->blockEntityAt(0, y, 0);
      auto expectedTile = expected->blockEntityAt(0, y, 0);
      if (expectedTile == nullptr) {
        CHECK(actualTile == nullptr);
      } else {
        CHECK(actualTile != nullptr);
        if (actualTile) {
          CheckMovingPistonTileEntity(*expectedTile, *actualTile, state);
        }
      }
    }
  }

  { // Bedrock to Java
    auto output = BedrockToJava(*bedrockReferenceDir);
    REQUIRE(output);

    mcfile::je::World actualWorld(*output);
    mcfile::je::World expectedWorld(*javaReferenceDir);

    auto actual = actualWorld.chunkAt(0, 0);
    REQUIRE(actual);
    auto expected = expectedWorld.chunkAt(0, 0);
    REQUIRE(expected);

    for (int y = 0; y < 4; y++) {
      auto actualBlock = actual->blockAt(0, y, 0);
      auto expectedBlock = expected->blockAt(0, y, 0);
      REQUIRE(actualBlock);
      REQUIRE(expectedBlock);
      CheckBlockJ(expectedBlock, actualBlock, mcfile::Dimension::Overworld, 0, y, 0);

      auto actualTile = actual->tileEntityAt(0, y, 0);
      auto expectedTile = expected->tileEntityAt(0, y, 0);
      if (expectedTile == nullptr) {
        CHECK(actualTile == nullptr);
      } else {
        CHECK(actualTile != nullptr);
        if (actualTile) {
          DiffCompoundTag(*expectedTile, *actualTile);
        }
      }
    }
  }

  fs::remove_all(*javaReferenceDir);
  fs::remove_all(*bedrockReferenceDir);
}

TEST_CASE("moving-piston") {
  // https://gyazo.com/84feadea4a96ecb63dedb9ec958e584a
  fs::path const thisFile(__FILE__);
  fs::path const projectRoot = fs::absolute(thisFile.parent_path().parent_path()).lexically_normal();
  fs::path const cwd = fs::absolute(fs::current_path()).lexically_normal();
  auto rel = fs::relative(projectRoot, cwd);
  fs::path const dataDirectory = rel / "test" / "data" / "piston";

  SUBCASE("state=0") {
    string bedrock = "1.20.51piston_arm_bedrock_state0.mcworld";
    string java = "1.20.4piston_arm_java_state0.zip";
    CheckMovingPiston(dataDirectory / java, dataDirectory / bedrock, 0);
  }

  SUBCASE("state=1") {
    string bedrock = "1.20.51piston_arm_bedrock_state1.mcworld";
    string java = "1.20.4piston_arm_java_state1.zip";
    CheckMovingPiston(dataDirectory / java, dataDirectory / bedrock, 1);
  }

  SUBCASE("state=2") {
    string bedrock = "1.20.51piston_arm_bedrock_state2.mcworld";
    string java = "1.20.4piston_arm_java_state2.zip";
    CheckMovingPiston(dataDirectory / java, dataDirectory / bedrock, 2);
  }

  SUBCASE("state=3") {
    string bedrock = "1.20.51piston_arm_bedrock_state3.mcworld";
    string java = "1.20.4piston_arm_java_state3.zip";
    CheckMovingPiston(dataDirectory / java, dataDirectory / bedrock, 3);
  }
}
