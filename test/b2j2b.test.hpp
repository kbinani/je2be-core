static std::shared_ptr<CompoundTag> ReadLevelDatB(fs::path const &p) {
  auto s = make_shared<mcfile::stream::FileInputStream>(p);
  REQUIRE(s->seek(4));     // version
  REQUIRE(s->seek(4 + 4)); // size
  mcfile::stream::InputStreamReader reader(s, mcfile::Endian::Little);
  return mcfile::nbt::CompoundTag::Read(reader);
}

static void CheckLevelDatB(fs::path const &expected, fs::path const &actual) {
  auto e = ReadLevelDatB(expected);
  auto a = ReadLevelDatB(actual);
  CHECK(e);
  CHECK(a);
  if (!e || !a) {
    return;
  }
  DiffCompoundTag(*e, *a);
}

static void TestBedrockToJavaToBedrock(fs::path const &in) {
  auto tmp = mcfile::File::CreateTempDir(fs::temp_directory_path());
  REQUIRE(tmp);
  defer {
    fs::remove_all(*tmp);
  };
  unsigned int concurrency = std::thread::hardware_concurrency();

  // extract mcworld
  auto inB = mcfile::File::CreateTempDir(*tmp);
  Status st = ZipFile::Unzip(in, *inB);
  REQUIRE(st.ok());

  // bedrock -> java
  auto outJ = mcfile::File::CreateTempDir(*tmp);
  REQUIRE(outJ);
  je2be::toje::Options optJ;
  optJ.fTempDirectory = mcfile::File::CreateTempDir(*tmp);
  st = je2be::toje::Converter::Run(*inB, *outJ, optJ, concurrency);
  REQUIRE(st.ok());

  // java -> bedrock
  auto outB = mcfile::File::CreateTempDir(*tmp);
  REQUIRE(outB);
  je2be::tobe::Options optB;
  optB.fTempDirectory = mcfile::File::CreateTempDir(*tmp);
  st = je2be::tobe::Converter::Run(*outJ, *outB, optB, concurrency);
  REQUIRE(st.ok());

  // Compare initial Bedrock input and final Bedrock output.
  CheckLevelDatB(*inB / "level.dat", *outB / "level.dat");
}

TEST_CASE("b2j2b") {
  fs::path thisFile(__FILE__);
  auto dataDir = thisFile.parent_path() / "data";
  auto in = dataDir / "b2j-test.mcworld";
  TestBedrockToJavaToBedrock(in);
}
