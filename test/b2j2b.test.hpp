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

static void CheckEntityB(u8string const &id, CompoundTag const &expected, CompoundTag const &actual) {
  auto defE = expected.listTag(u8"definitions");
  auto defA = actual.listTag(u8"definitions");
  if (defE) {
    CHECK(defA);
    if (defA) {
      static set<u8string> const sIgnore = {
          u8"+",
          u8"+minecraft:wild_child_ocelot_spawn",
          u8"+look_for_food",
      };
      for (auto const &e : *defE) {
        auto cE = e->asString();
        if (!cE) {
          continue;
        }
        if (sIgnore.count(cE->fValue) > 0) {
          continue;
        }
        bool found = false;
        for (auto const &it : *defA) {
          auto cA = it->asString();
          if (!cA) {
            continue;
          }
          if (cA->fValue == cE->fValue) {
            found = true;
            break;
          }
        }
        if (!found) {
          cerr << cE->fValue << " not found for entity id: " << id << endl;
        }
        CHECK(found);
      }
      for (auto const &a : *defA) {
        auto cA = a->asString();
        if (!cA) {
          continue;
        }
        if (sIgnore.count(cA->fValue) > 0) {
          continue;
        }
        bool found = false;
        for (auto const &it : *defE) {
          auto cE = it->asString();
          if (!cE) {
            continue;
          }
          if (cE->fValue == cA->fValue) {
            found = true;
            break;
          }
        }
        if (!found) {
          cerr << cA->fValue << " is extra definition for entity " << id << endl;
        }
      }
    }
  }

  // TODO: check other properties
}

static void CheckChunkB(mcfile::be::Chunk const &expected, mcfile::be::Chunk const &actual) {
  for (auto const &e : expected.entities()) {
    auto posE = je2be::props::GetPos3f(*e, u8"Pos");
    REQUIRE(posE);
    auto idE = e->string(u8"identifier");
    REQUIRE(idE);
    double minDistanceSq = numeric_limits<double>::max();
    CompoundTagPtr a;
    for (auto const &it : actual.entities()) {
      auto posA = je2be::props::GetPos3f(*it, u8"Pos");
      if (!posA) {
        continue;
      }
      auto idA = it->string(u8"identifier");
      if (idA != *idE) {
        continue;
      }
      double distanceSq = Pos3f::DistanceSquare(*posE, *posA);
      if (distanceSq < minDistanceSq) {
        minDistanceSq = distanceSq;
        a = it;
      }
    }
    if (a) {
      CheckEntityB(*idE, *e, *a);
    } else {
      ostringstream out;
      PrintAsJson(out, *e, {.fTypeHint = true});
      lock_guard<mutex> lock(sMutCerr);
      cerr << out.str();
      CHECK(false);
    }
  }
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
  // CheckLevelDatB(*inB / "level.dat", *outB / "level.dat");
  unique_ptr<leveldb::DB> dbE(OpenF(*inB / "db"));
  unique_ptr<leveldb::DB> dbA(OpenF(*outB / "db"));
  REQUIRE(dbE);
  REQUIRE(dbA);
  for (auto dimension : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
    mcfile::be::Chunk::ForAll(dbE.get(), dimension, [&](int cx, int cz) {
      auto chunkE = mcfile::be::Chunk::Load(cx, cz, dimension, dbE.get(), mcfile::Endian::Little);
      REQUIRE(chunkE);
      auto chunkA = mcfile::be::Chunk::Load(cx, cz, dimension, dbA.get(), mcfile::Endian::Little);
      CHECK(chunkA);
      if (!chunkA) {
        return true;
      }
      CheckChunkB(*chunkE, *chunkA);
      return true;
    });
  }
}

TEST_CASE("b2j2b") {
  fs::path thisFile(__FILE__);
  auto dataDir = thisFile.parent_path() / "data";
  auto in = dataDir / "b2j-test.mcworld";
  TestBedrockToJavaToBedrock(in);
}
