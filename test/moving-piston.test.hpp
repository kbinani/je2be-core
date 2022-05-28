#pragma once

static std::optional<fs::path> JavaToBedrock(fs::path const &java) {
  auto output = File::CreateTempDir(fs::temp_directory_path());
  if (!output) {
    return nullopt;
  }
  je2be::tobe::Options o;
  o.fDimensionFilter.insert(mcfile::Dimension::Overworld);
  o.fChunkFilter.insert(Pos2i(0, 0));
  je2be::tobe::Converter converter(java, *output, o);
  if (!converter.run(thread::hardware_concurrency())) {
    return nullopt;
  }
  return *output;
}

static std::optional<fs::path> BedrockToJava(fs::path const &bedrock) {
  auto output = File::CreateTempDir(fs::temp_directory_path());
  if (!output) {
    return nullopt;
  }
  je2be::toje::Options o;
  o.fDimensionFilter.insert(mcfile::Dimension::Overworld);
  o.fChunkFilter.insert(Pos2i(0, 0));
  je2be::toje::Converter converter(bedrock, *output, o);
  if (auto st = converter.run(thread::hardware_concurrency()); !st.ok()) {
    return nullopt;
  }
  return *output;
}

static void CheckBedrockBlock(mcfile::be::Block const &a, mcfile::be::Block const &b) {
  CHECK(a.fName == b.fName);
  CHECK(a.fVersion == b.fVersion);
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

static void CheckMovingPiston(fs::path const &java, fs::path const &bedrock) {
  auto javaReferenceDir = File::CreateTempDir(fs::temp_directory_path());
  REQUIRE(javaReferenceDir);
  REQUIRE(ZipFile::Unzip(java, *javaReferenceDir));

  auto bedrockReferenceDir = File::CreateTempDir(fs::temp_directory_path());
  REQUIRE(bedrockReferenceDir);
  REQUIRE(ZipFile::Unzip(bedrock, *bedrockReferenceDir));

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
          DiffCompoundTag(*expectedTile, *actualTile);
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
      CheckBlock(expectedBlock, actualBlock, mcfile::Dimension::Overworld, 0, y, 0);

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
  fs::path const dataDirectory = thisFile.parent_path() / "data" / "piston";

  SUBCASE("state=0") {
    string bedrock = "1.19piston_arm_bedrock_-8076300039614213879.mcworld";
    string java = "1.19piston_arm_java_-8541049737591066119.zip";
    CheckMovingPiston(dataDirectory / java, dataDirectory / bedrock);
  }

  SUBCASE("state=1") {
    string bedrock = "1.19piston_arm_bedrock_6291263495557308048.mcworld";
    string java = "1.19piston_arm_java_1094285069167139155.zip";
    CheckMovingPiston(dataDirectory / java, dataDirectory / bedrock);
  }

  SUBCASE("state=2") {
    string bedrock = "1.19piston_arm_bedrock_4449454955840640434.mcworld";
    string java = "1.19piston_arm_java_2666496645811299860.zip";
    CheckMovingPiston(dataDirectory / java, dataDirectory / bedrock);
  }

  SUBCASE("state=3") {
    string bedrock = "1.19piston_arm_bedrock_1911327770476968939.mcworld";
    string java = "1.19piston_arm_java_3720894831070949398.zip";
    CheckMovingPiston(dataDirectory / java, dataDirectory / bedrock);
  }
}
