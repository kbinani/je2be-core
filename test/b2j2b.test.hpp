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

static void CheckEntityDefinitionsB(u8string const &id, ListTagPtr const &expected, ListTagPtr const &actual) {
  static set<u8string> const sIgnore = {
      u8"+",
      u8"minecraft:wild_child_ocelot_spawn",
      u8"look_for_food",           // bee
      u8"find_hive",               // bee
      u8"escape_fire",             // bee
      u8"return_to_home",          // bee
      u8"minecraft:fox_with_item", // sometimes this tag exists even when fox does not have any items
      u8"minecraft:docile_fox",
      u8"minecraft:fox_night",
      u8"minecraft:fox_day",
      u8"minecraft:start_suffocating",
      u8"minecraft:detect_suffocating",
      u8"axolotl_on_land",
      u8"axolotl_dried",
      u8"axolotl_in_water",
      u8"-feeling_happy",          // sniffer
      u8"-sniffer_search_and_dig", // sniffer
      u8"-stand_up",               // sniffer
  };

  set<u8string> setE;
  if (expected) {
    for (auto const &it : expected->fValue) {
      auto s = it->asString();
      if (!s) {
        continue;
      }
      if (sIgnore.count(s->fValue) > 0) {
        continue;
      }
      if (s->fValue.starts_with(u8"+") || s->fValue.starts_with(u8"-")) {
        if (sIgnore.count(s->fValue.substr(1)) > 0) {
          continue;
        }
      }
      setE.insert(s->fValue);
    }
  }

  set<u8string> setA;
  if (actual) {
    for (auto const &it : actual->fValue) {
      auto s = it->asString();
      if (!s) {
        continue;
      }
      if (sIgnore.count(s->fValue) > 0) {
        continue;
      }
      if (s->fValue.starts_with(u8"+") || s->fValue.starts_with(u8"-")) {
        if (sIgnore.count(s->fValue.substr(1)) > 0) {
          continue;
        }
      }
      setA.insert(s->fValue);
    }
  }

  set<u8string> common;
  set<u8string> extra = setA;
  set<u8string> missing = setE;
  while (true) {
    bool changed = false;
    for (u8string e : extra) {
      if (missing.count(e) > 0) {
        common.insert(e);
        extra.erase(e);
        missing.erase(e);
        changed = true;
        break;
      }
    }
    if (!changed) {
      break;
    }
  }

  static set<u8string> const sEntitiesSkipDefinitionCheck{
      u8"minecraft:villager_v2",
      u8"minecraft:zombie_villager_v2",
  };
  if (sEntitiesSkipDefinitionCheck.count(id) == 0) {
    for (auto const &m : missing) {
      cerr << m << " not found for entity id: " << id << endl;
    }
    for (auto const &e : extra) {
      cerr << e << " is extra definition for entity " << id << endl;
    }
    CHECK(missing.empty());
    CHECK(extra.empty());
  }
}

static void CheckEntityB(u8string const &id, CompoundTag const &expected, CompoundTag const &actual) {
  auto defE = expected.listTag(u8"definitions");
  auto defA = actual.listTag(u8"definitions");
  CheckEntityDefinitionsB(id, defE, defA);

  if (id == u8"minecraft:falling_block") {
    auto blockE = expected.compoundTag(u8"FallingBlock");
    auto blockA = actual.compoundTag(u8"FallingBlock");
    if (blockE) {
      CHECK(blockA);
      if (blockA) {
        auto copyE = blockE->copy();
        auto copyA = blockA->copy();
        copyE->erase(u8"version");
        copyA->erase(u8"version");
        DiffCompoundTag(*copyE, *copyA);
      }
    } else {
      CHECK(!blockA);
    }

    auto variantE = expected.int32(u8"Variant");
    auto variantA = actual.int32(u8"Variant");
    CHECK(variantE == variantA);
  }
  // TODO: check other properties
}

static void CheckBlockEntityB(CompoundTag const &expected, CompoundTag const &actual) {
  auto e = expected.copy();
  auto a = actual.copy();
  auto idE = e->string(u8"id");
  auto idA = a->string(u8"id");
  REQUIRE(idE);
  REQUIRE(idA);
  CHECK(*idE == *idA);
  set<u8string> ignore;
  if (*idE == u8"CommandBlock") {
    ignore.insert(u8"LPCommandMode");
    ignore.insert(u8"LPCondionalMode");
    ignore.insert(u8"LPRedstoneMode");
    ignore.insert(u8"LastOutput");
    ignore.insert(u8"LastOutputParams");
    ignore.insert(u8"ExecuteOnFirstTick");
    ignore.insert(u8"Version");
  }
  for (auto const &i : ignore) {
    e->erase(i);
    a->erase(i);
  }
  DiffCompoundTag(*e, *a);
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
  auto const &actualBlockEntities = actual.blockEntities();
  for (auto const &e : expected.blockEntities()) {
    auto found = actualBlockEntities.find(e.first);
    REQUIRE(e.second);
    CHECK(found != actualBlockEntities.end());
    if (found == actualBlockEntities.end()) {
      continue;
    }
    REQUIRE(found->second);
    CheckBlockEntityB(*e.second, *found->second);
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

  std::unordered_set<Pos2i, Pos2iHasher> chunkFilter;
#if 0
  chunkFilter.insert({0, 0});
#endif

  // bedrock -> java
  auto outJ = mcfile::File::CreateTempDir(*tmp);
  REQUIRE(outJ);
  je2be::toje::Options optJ;
  optJ.fTempDirectory = mcfile::File::CreateTempDir(*tmp);
  if (!chunkFilter.empty()) {
    optJ.fDimensionFilter.insert(mcfile::Dimension::Overworld);
    for (auto const &ch : chunkFilter) {
      optJ.fChunkFilter.insert(ch);
    }
  }
  st = je2be::toje::Converter::Run(*inB, *outJ, optJ, concurrency);
  REQUIRE(st.ok());

  // java -> bedrock
  auto outB = mcfile::File::CreateTempDir(*tmp);
  REQUIRE(outB);
  je2be::tobe::Options optB;
  optB.fTempDirectory = mcfile::File::CreateTempDir(*tmp);
  if (!chunkFilter.empty()) {
    optB.fDimensionFilter.insert(mcfile::Dimension::Overworld);
    for (auto const &ch : chunkFilter) {
      optB.fChunkFilter.insert(ch);
    }
  }
  st = je2be::tobe::Converter::Run(*outJ, *outB, optB, concurrency);
  REQUIRE(st.ok());

  // Compare initial Bedrock input and final Bedrock output.
  // CheckLevelDatB(*inB / "level.dat", *outB / "level.dat");
  unique_ptr<leveldb::DB> dbE(OpenF(*inB / "db"));
  unique_ptr<leveldb::DB> dbA(OpenF(*outB / "db"));
  REQUIRE(dbE);
  REQUIRE(dbA);
  for (auto dimension : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
    if (!chunkFilter.empty() && dimension != mcfile::Dimension::Overworld) {
      continue;
    }
    mcfile::be::Chunk::ForAll(dbE.get(), dimension, [&](int cx, int cz) {
      if (!chunkFilter.empty() && chunkFilter.count({cx, cz}) == 0) {
        return true;
      }
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
