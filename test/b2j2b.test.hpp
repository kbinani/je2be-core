static void CheckEntityB(u8string const &id, CompoundTag const &expected, CompoundTag const &actual);

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
  set<u8string> ignore;
  ignore.insert(u8"hasBeenLoadedInCreative");
  ignore.insert(u8"lastOpenedWithVersion");
  ignore.insert(u8"recipesunlock");
  ignore.insert(u8"serverChunkTickRange");
  ignore.insert(u8"worldStartCount");
  ignore.insert(u8"InventoryVersion");
  ignore.insert(u8"NetworkVersion");
  for (auto const &i : ignore) {
    e->erase(i);
    a->erase(i);
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
      u8"feeling_happy",           // sniffer
      u8"sniffer_search_and_dig",  // sniffer
      u8"stand_up",                // sniffer
      u8"minecraft:camel_sitting", // cannot reconstruct LastPoseTick when B->J conversion.
      u8"minecraft:camel_standing",
      u8"attack_cooldown",
      u8"minecraft:deflate_sensor",        // pufferfish
      u8"minecraft:deflate_sensor_buffer", // pufferfish
      u8"minecraft:start_deflate",         // pufferfish
      u8"minecraft:full_puff",             // pufferfish
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

static void CheckBlockB(CompoundTag const &expected, CompoundTag const &actual) {
  auto nameE = expected.string(u8"name");
  auto nameA = actual.string(u8"name");
  set<u8string> ignore;
  if (nameE == u8"minecraft:brown_mushroom_block") {
    if (auto statesE = expected.compoundTag(u8"states"); statesE) {
      if (statesE->int32(u8"huge_mushroom_bits") == 0) {
        ignore.insert(u8"huge_mushroom_bits");
      }
    }
  }
  CHECK(nameE == nameA);
  auto statesE = expected.compoundTag(u8"states");
  auto statesA = actual.compoundTag(u8"states");
  if (statesE) {
    CHECK(statesA);
    if (statesA) {
      auto e = statesE->copy();
      auto a = statesA->copy();
      for (auto const &i : ignore) {
        e->erase(i);
        a->erase(i);
      }
      DiffCompoundTag(*e, *a);
    }
  }
}

static void CheckItemB(CompoundTag const &expected, CompoundTag const &actual) {
  auto nameE = expected.string(u8"Name");
  auto nameA = actual.string(u8"Name");
  auto e = expected.copy();
  auto a = actual.copy();
  set<u8string> ignore;
  if (nameE == u8"minecraft:field_masoned_banner_pattern" || nameE == u8"minecraft:bordure_indented_banner_pattern") {
    // Doesn't exist in JE
    ignore.insert(u8"Name");
  } else if (nameE == u8"minecraft:empty_map") {
    // Empty Locator Map (Damage=2) doesn't exist in JE
    ignore.insert(u8"Damage");
  } else if (nameE == u8"minecraft:potion" || nameE == u8"minecraft:splash_potion" || nameE == u8"minecraft:lingering_potion") {
    if (expected.int16(u8"Damage") == 2) {
      // Long mundane potion doesn't exist in JE
      ignore.insert(u8"Damage");
      CHECK(actual.int16(u8"Damage") == 4);
    }
  } else if (nameE == u8"minecraft:tropical_fish_bucket" || nameE == u8"minecraft:cod_bucket" || nameE == u8"minecraft:salmon_bucket" || nameE == u8"minecraft:pufferfish_bucket" || nameE == u8"minecraft:axolotl_bucket" || nameE == u8"minecraft:tadpole_bucket") {
    auto tagE = expected.compoundTag(u8"tag");
    auto tagA = actual.compoundTag(u8"tag");
    if (tagE) {
      auto idE = tagE->string(u8"identifier");
      REQUIRE(idE);
      CHECK(tagA);
      if (tagA) {
        auto copyE = tagE->copy();
        auto copyA = tagA->copy();
        for (auto const &p : {u8"Air"}) {
          Erase(copyE, p);
          Erase(copyA, p);
        }
        CheckEntityB(*idE, *copyE, *copyA);
      }
    } else {
      CHECK(!tagA);
    }
    ignore.insert(u8"tag");
  }
  auto blockE = expected.compoundTag(u8"Block");
  auto blockA = actual.compoundTag(u8"Block");
  if (blockE) {
    CHECK(blockA);
    if (blockA) {
      CheckBlockB(*blockE, *blockA);
    }
    ignore.insert(u8"Block");
  }
  for (auto const &i : ignore) {
    Erase(e, i);
    Erase(a, i);
  }
  DiffCompoundTag(*e, *a);
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
  }

  auto poseE = expected.compoundTag(u8"Pose");
  auto poseA = actual.compoundTag(u8"Pose");
  if (poseE) {
    CHECK(poseA);
    if (poseA) {
      CHECK(poseE->int32(u8"PoseIndex") == poseA->int32(u8"PoseIndex"));
    }
  }

  for (u8string const &key : {u8"Inventory", u8"EnderChestInventory"}) {
    auto inventoryE = expected.listTag(key);
    auto inventoryA = actual.listTag(key);
    if (inventoryE) {
      CHECK(inventoryA);
      CHECK(inventoryE->size() == inventoryA->size());
      for (size_t i = 0; i < inventoryE->size(); i++) {
        auto e = inventoryE->at(i)->asCompound();
        REQUIRE(e);
        auto a = inventoryA->at(i)->asCompound();
        CHECK(a);
        if (!a) {
          continue;
        }
        CheckItemB(*e, *a);
      }
    }
  }

  // TODO: check other properties
}

static void CheckRotationB(float e, float a) {
  while (e > 180) {
    e -= 360;
  }
  while (e <= -180) {
    e += 360;
  }
  while (a > 180) {
    a -= 360;
  }
  while (a <= -180) {
    a += 360;
  }
  CHECK(e == a);
}

static void CheckBlockEntityB(CompoundTag const &expected, CompoundTag const &actual, std::shared_ptr<mcfile::be::Block const> const &blockE) {
  auto e = expected.copy();
  auto a = actual.copy();
  auto x = expected.int32(u8"x");
  auto y = expected.int32(u8"y");
  auto z = expected.int32(u8"z");
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
    ignore.insert(u8"CustomName");
  } else if (*idE == u8"Bell") {
    ignore.insert(u8"Ticks");
    auto direction = e->int32(u8"Direction");
    if (direction == 255 || (blockE && blockE->fStates->string(u8"attachment") != u8"standing")) {
      ignore.insert(u8"Direction");
    }
  } else if (*idE == u8"Skull") {
    auto rotE = e->float32(u8"Rotation");
    auto rotA = a->float32(u8"Rotation");
    CHECK(rotE);
    CHECK(rotA);
    if (rotE && rotA) {
      // Rotation can be both 180/-180
      CheckRotationB(*rotE, *rotA);
    }
    ignore.insert(u8"Rotation");
    ignore.insert(u8"DoingAnimation");
    ignore.insert(u8"MouthTickCount");
  } else if (*idE == u8"Conduit") {
    ignore.insert(u8"Active");
  } else if (*idE == u8"CalibratedSculkSensor" || *idE == u8"SculkShrieker" || *idE == u8"SculkSensor") {
    ignore.insert(u8"VibrationListener");
  } else if (*idE == u8"Furnace" || *idE == u8"BlastFurnace" || *idE == u8"Smoker") {
    ignore.insert(u8"StoredXPInt");
  } else if (*idE == u8"EnchantTable") {
    ignore.insert(u8"rott");
  } else if (*idE == u8"Beehive") {
    auto occupantsE = e->listTag(u8"Occupants");
    auto occupantsA = a->listTag(u8"Occupants");
    if (occupantsE) {
      CHECK(occupantsA);
      CHECK(occupantsE->size() == occupantsA->size());
      for (size_t i = 0; i < occupantsE->size(); i++) {
        auto occupantE = occupantsE->at(i)->asCompound();
        auto occupantA = occupantsA->at(i)->asCompound();
        CheckEntityB(u8"minecraft:bee", *occupantE, *occupantA);
      }
    }
    ignore.insert(u8"Occupants");
  }
  auto itemE = e->compoundTag(u8"Item");
  auto itemA = a->compoundTag(u8"Item");
  if (itemE) {
    REQUIRE(itemA);
    CheckItemB(*itemE, *itemA);
    ignore.insert(u8"Item");
  }
  for (auto const &key : {u8"Items"}) {
    ignore.insert(key);
    auto listE = e->listTag(key);
    auto listA = a->listTag(key);
    if (listE) {
      CHECK(listA);
      if (listA) {
        CHECK(listE->size() == listA->size());
        for (size_t i = 0; i < listE->size(); i++) {
          auto itemE = listE->at(i)->asCompound();
          auto itemA = listA->at(i)->asCompound();
          REQUIRE(itemE);
          CHECK(itemA);
          if (itemA) {
            CheckItemB(*itemE, *itemA);
          }
        }
      }
    } else {
      CHECK(!listA);
    }
  }
  for (auto const &i : ignore) {
    e->erase(i);
    a->erase(i);
  }
  DiffCompoundTag(*e, *a);
}

static void CheckChunkB(mcfile::be::Chunk const &chunkE, mcfile::be::Chunk const &chunkA) {
  for (int y = chunkE.minBlockY(); y <= chunkE.maxBlockY(); y++) {
    for (int z = chunkE.minBlockZ(); z <= chunkE.maxBlockZ(); z++) {
      for (int x = chunkE.minBlockX(); x <= chunkE.maxBlockX(); x++) {
        auto e = chunkE.blockAt(x, y, z);
        auto a = chunkA.blockAt(x, y, z);
        if (e) {
          REQUIRE(a);
          if (e->fName == u8"minecraft:water" || e->fName == u8"minecraft:flowing_water" || e->fName == u8"minecraft:lava" || e->fName == u8"minecraft:flowing_lava") {
            // TODO:
            continue;
          }
          if (e->fName == u8"minecraft:brown_mushroom_block" || e->fName == u8"minecraft:red_mushroom_block") {
            // Java has only "mushroom_stem" for stem block.
            // Bedrock's "huge_mushroom_bits" property cannot completely represents Java's up/down/north/east/south/west properties.
            continue;
          }
          CHECK(e->fName == a->fName);
          auto tagE = e->fStates->copy();
          auto tagA = a->fStates->copy();
          set<u8string> ignore;
          if (e->fName == u8"minecraft:weeping_vines") {
            ignore.insert(u8"weeping_vines_age");
          } else if (e->fName == u8"minecraft:twisting_vines") {
            ignore.insert(u8"twisting_vines_age");
          } else if (e->fName == u8"minecraft:cave_vines") {
            ignore.insert(u8"growing_plant_age");
          } else if (e->fName == u8"minecraft:bamboo_sapling") {
            // Bamboo sapling doesn't have "stage" property in Java
            ignore.insert(u8"age_bit");
          } else if (e->fName == u8"minecraft:soul_fire") {
            ignore.insert(u8"age");
          }
          for (auto const &i : ignore) {
            tagE->erase(i);
            tagA->erase(i);
          }
          DiffCompoundTag(*tagE, *tagA);
        } else {
          if (a) {
            CHECK(a->fName == u8"minecraft:air");
          }
        }
      }
    }
  }
  for (auto const &e : chunkE.entities()) {
    auto posE = je2be::props::GetPos3f(*e, u8"Pos");
    REQUIRE(posE);
    auto idE = e->string(u8"identifier");
    REQUIRE(idE);
    double minDistanceSq = numeric_limits<double>::max();
    CompoundTagPtr a;
    for (auto const &it : chunkA.entities()) {
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
  auto const &actualBlockEntities = chunkA.blockEntities();
  for (auto const &e : chunkE.blockEntities()) {
    auto found = actualBlockEntities.find(e.first);
    REQUIRE(e.second);
    CHECK(found != actualBlockEntities.end());
    if (found == actualBlockEntities.end()) {
      continue;
    }
    REQUIRE(found->second);
    auto blockE = chunkE.blockAt(e.first);
    CheckBlockEntityB(*e.second, *found->second, blockE);
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
  je2be::bedrock::Options optJ;
  optJ.fTempDirectory = mcfile::File::CreateTempDir(*tmp);
  std::unordered_set<Pos2i, Pos2iHasher> chunks;
  std::vector<mcfile::Dimension> dimensions = {mcfile::Dimension::Overworld,
                                               mcfile::Dimension::Nether,
                                               mcfile::Dimension::End};
  bool checkLevelDat = true;
#if 1
  Pos2i center(0, 0);
  int radius = 3;
  for (int x = -radius; x <= radius; x++) {
    for (int y = -radius; y <= radius; y++) {
      chunks.insert({center.fX + x, center.fZ + y});
    }
  }
#else
  chunks.insert({2, 1});
  dimensions.clear();
  dimensions = {mcfile::Dimension::Overworld};
  checkLevelDat = false;
#endif
  if (!dimensions.empty()) {
    for (auto const &d : dimensions) {
      optJ.fDimensionFilter.insert(d);
    }
  }
  if (!chunks.empty()) {
    for (auto const &ch : chunks) {
      for (int dx = -1; dx <= 1; dx++) {
        for (int dz = -1; dz <= 1; dz++) {
          optJ.fChunkFilter.insert({ch.fX + dx, ch.fZ + dz});
        }
      }
    }
  }
  st = je2be::bedrock::Converter::Run(*inB, *outJ, optJ, concurrency);
  REQUIRE(st.ok());

  // java -> bedrock
  auto outB = mcfile::File::CreateTempDir(*tmp);
  REQUIRE(outB);
  je2be::java::Options optB;
  optB.fTempDirectory = mcfile::File::CreateTempDir(*tmp);
  if (!dimensions.empty()) {
    for (auto const &d : dimensions) {
      optB.fDimensionFilter.insert(d);
    }
  }
  if (!chunks.empty()) {
    for (auto const &ch : chunks) {
      for (int dx = -1; dx <= 1; dx++) {
        for (int dz = -1; dz <= 1; dz++) {
          optB.fChunkFilter.insert({ch.fX + dx, ch.fZ + dz});
        }
      }
    }
  }
  st = je2be::java::Converter::Run(*outJ, *outB, optB, concurrency);
  REQUIRE(st.ok());

  // Compare initial Bedrock input and final Bedrock output.
  if (checkLevelDat) {
    CheckLevelDatB(*inB / "level.dat", *outB / "level.dat");
  }
  unique_ptr<leveldb::DB> dbE(OpenF(*inB / "db"));
  unique_ptr<leveldb::DB> dbA(OpenF(*outB / "db"));
  REQUIRE(dbE);
  REQUIRE(dbA);

  for (auto dimension : dimensions) {
    mcfile::be::Chunk::ForAll(dbE.get(), dimension, [&](int cx, int cz) {
      if (!chunks.empty() && chunks.count({cx, cz}) == 0) {
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

  {
    string localPlayerE;
    REQUIRE(dbE->Get({}, mcfile::be::DbKey::LocalPlayer(), &localPlayerE).ok());
    string localPlayerA;
    CHECK(dbA->Get({}, mcfile::be::DbKey::LocalPlayer(), &localPlayerA).ok());
    auto componentE = CompoundTag::Read(localPlayerE, mcfile::Endian::Little);
    REQUIRE(componentE);
    auto componentA = CompoundTag::Read(localPlayerA, mcfile::Endian::Little);
    CHECK(componentA);
    if (componentA) {
      CheckEntityB(u8"minecraft:player", *componentE, *componentA);
    }
  }
}

TEST_CASE("b2j2b") {
  fs::path thisFile(__FILE__);
  auto dataDir = thisFile.parent_path() / "data";
  auto in = dataDir / "b2j-test.mcworld";
  TestBedrockToJavaToBedrock(in);
}
