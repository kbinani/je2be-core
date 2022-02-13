#pragma once

static void CheckBlockWithIgnore(mcfile::je::Block const &e, mcfile::je::Block const &a, std::initializer_list<std::string> ignore) {
  CHECK(e.fName == a.fName);
  map<string, string> propsE(e.fProperties);
  map<string, string> propsA(a.fProperties);
  for (std::string const &p : ignore) {
    propsE.erase(p);
    propsA.erase(p);
  }
  auto blockE = make_shared<mcfile::je::Block>(e.fName, propsE);
  auto blockA = make_shared<mcfile::je::Block>(a.fName, propsA);
  CHECK(blockA->toString() == blockE->toString());
}

static void CheckBlock(shared_ptr<mcfile::je::Block const> const &blockE, shared_ptr<mcfile::je::Block const> const &blockA, Dimension dim, int x, int y, int z) {
  unordered_map<string, string> fallbackJtoB;
  fallbackJtoB["minecraft:petrified_oak_slab"] = "minecraft:oak_slab"; // does not exist in BE. should be replaced to oak_slab when java -> bedrock.
  fallbackJtoB["minecraft:cave_air"] = "minecraft:air";

  unordered_map<string, string> fallbackBtoJ;
  fallbackBtoJ["minecraft:frame"] = "minecraft:air";      // frame should be converted as an entity.
  fallbackBtoJ["minecraft:glow_frame"] = "minecraft:air"; // frame should be converted as an entity.

  if (blockA && blockE) {
    auto foundJtoB = fallbackJtoB.find(blockE->fName);
    if (foundJtoB == fallbackJtoB.end()) {
      auto foundBtoJ = fallbackBtoJ.find(blockA->fName);
      if (foundBtoJ == fallbackBtoJ.end()) {
        if (blockE->fName.ends_with("_leaves")) {
          CheckBlockWithIgnore(*blockE, *blockA, {"distance"});
        } else if (blockE->fName == "minecraft:redstone_wall_torch" || blockE->fName == "minecraft:redstone_torch") {
          CheckBlockWithIgnore(*blockE, *blockA, {"lit"});
        } else if (blockE->fName == "minecraft:red_mushroom_block" || blockE->fName == "minecraft:brown_mushroom_block" || blockE->fName == "minecraft:sculk_sensor") {
          CHECK(blockE->fName == blockA->fName);
        } else if (blockE->fName == "minecraft:scaffolding") {
          CheckBlockWithIgnore(*blockE, *blockA, {"distance"});
        } else if (blockE->fName == "minecraft:repeater") {
          CheckBlockWithIgnore(*blockE, *blockA, {"locked"});
        } else if (blockE->fName == "minecraft:note_block" || blockE->fName.ends_with("_trapdoor") || blockE->fName.ends_with("_fence_gate") || blockE->fName == "minecraft:lectern" || blockE->fName.ends_with("_door") || blockE->fName == "minecraft:lightning_rod") {
          CheckBlockWithIgnore(*blockE, *blockA, {"powered"});
        } else if (blockE->fName.ends_with("_button")) {
          CheckBlockWithIgnore(*blockE, *blockA, {"facing", "powered"});
        } else if (blockE->fName == "minecraft:fire") {
          CheckBlockWithIgnore(*blockE, *blockA, {"east", "north", "south", "west", "up"});
        } else if (blockE->fName == "minecraft:weeping_vines_plant") {
          if (blockA->toString() == "minecraft:weeping_vines[age=25]") {
            // JE: weeping_vines -> tip
            //     weeping_vines_plant -> body
            // BE: weeping_vines -> tip / body

            // "weeping_vines_plant without tip" sometimes seen in vanilla JE. tobe::Converter recognize this as a tip of vines
            CHECK(true);
          } else {
            CHECK(blockA->toString() == blockE->toString());
          }
        } else if (blockE->fName == "minecraft:twisting_vines_plant") {
          if (blockA->toString() == "minecraft:twisting_vines[age=25]") {
            CHECK(true);
          } else {
            CHECK(blockA->toString() == blockE->toString());
          }
        } else if (blockE->fName == "minecraft:vine") {
          CheckBlockWithIgnore(*blockE, *blockA, {"up"});
        } else {
          if (blockA->toString() != blockE->toString()) {
            cout << "[" << x << ", " << y << ", " << z << "] " << JavaStringFromDimension(dim) << endl;
          }
          CHECK(blockA->toString() == blockE->toString());
        }
      } else {
        CHECK(foundBtoJ->second == blockE->fName);
      }
    } else {
      CHECK(blockA->fName == foundJtoB->second);
    }
  } else if (blockA) {
    CHECK(blockA->fName == "minecraft:air");
  } else if (blockE) {
    CHECK(blockE->fName == "minecraft:air");
  }
}

static void Erase(shared_ptr<CompoundTag> t, string path) {
  auto keys = mcfile::String::Split(path, '/');
  if (keys.size() == 1) {
    t->erase(keys[0]);
    return;
  }
  auto index = strings::Toi(keys[1]);
  if (keys[1] == "*") {
    assert(keys.size() >= 3);
    auto next = t->listTag(keys[0]);
    if (!next) {
      return;
    }
    string nextPath = keys[2];
    for (int i = 3; i < keys.size(); i++) {
      nextPath += "/" + keys[i];
    }
    for (auto const &it : *next) {
      if (it->type() != Tag::Type::Compound) {
        continue;
      }
      shared_ptr<CompoundTag> c = dynamic_pointer_cast<CompoundTag>(it);
      if (!c) {
        continue;
      }
      Erase(c, nextPath);
    }
  } else if (index) {
    assert(keys.size() == 2);
    auto next = t->listTag(keys[0]);
    if (!next) {
      return;
    }
    if (0 <= *index && *index < next->size()) {
      next->fValue.erase(next->fValue.begin() + *index);
    }
  } else {
    t = t->compoundTag(keys[0]);
    if (!t) {
      return;
    }
    string nextPath = keys[1];
    for (int i = 2; i < keys.size(); i++) {
      nextPath += "/" + keys[i];
    }
    Erase(t, nextPath);
  }
}

static void DiffCompoundTag(CompoundTag const &e, CompoundTag const &a) {
  ostringstream streamE;
  PrintAsJson(streamE, e, {.fTypeHint = true});
  ostringstream streamA;
  PrintAsJson(streamA, a, {.fTypeHint = true});
  string jsonE = streamE.str();
  string jsonA = streamA.str();
  if (jsonE == jsonA) {
    CHECK(true);
  } else {
    vector<string> linesE = mcfile::String::Split(jsonE, '\n');
    vector<string> linesA = mcfile::String::Split(jsonA, '\n');
    int lines = (std::min)(linesE.size(), linesA.size());
    cerr << "actual:" << endl;
    for (int i = 0; i < lines; i++) {
      if (i < linesE.size() && linesA[i] != linesE[i]) {
        cerr << ">> ";
      } else {
        cerr << "   ";
      }
      cerr << i << ":\t" << linesA[i] << endl;
    }
    cerr << "expected:" << endl;
    for (int i = 0; i < lines; i++) {
      if (i < linesA.size() && linesA[i] != linesE[i]) {
        cerr << ">> ";
      } else {
        cerr << "   ";
      }
      cerr << i << ":\t" << linesE[i] << endl;
    }
    CHECK(false);
  }
}

static void CheckTileEntity(CompoundTag const &expected, CompoundTag const &actual) {
  auto copyE = expected.copy();
  auto copyA = actual.copy();

  static unordered_set<string> sTagBlacklist = {
      "LootTableSeed",           // chest in dungeon etc.
      "RecipesUsed",             // furnace, blast_furnace, and smoker
      "LastOutput",              // command_block
      "author",                  // structure_block
      "Book/tag/resolved",       // written_book
      "Book/tag/filtered_title", // written_book
      "Items/*/tag/map",
      "Levels",          // beacon. Sometimes reset to 0 in JE
      "SpawnPotentials", // mob_spawner, SpawnPotentials sometimes doesn't contained in JE
  };
  for (string b : sTagBlacklist) {
    Erase(copyE, b);
    Erase(copyA, b);
  }
  DiffCompoundTag(*copyE, *copyA);
}

static void CheckEntity(std::string const &id, CompoundTag const &entityE, CompoundTag const &entityA) {
  auto copyE = entityE.copy();
  auto copyA = entityA.copy();

  CHECK(copyE->intArrayTag("UUID"));
  CHECK(copyA->intArrayTag("UUID"));

  auto posE = props::GetPos3d(*copyE, "Pos");
  auto posA = props::GetPos3d(*copyA, "Pos");
  CHECK(posE);
  CHECK(posA);
  CHECK(fabs(posE->fX - posA->fX) <= 0.001);
  if (id == "minecraft:armor_stand") {
    // y is aligned 0.5 block in BE
  } else if (id == "minecraft:chest_minecart") {
    // y of chest_minecart in abandoned_mineshaft has usually conversion failure because OnGround=false but not on rail
  } else {
    CHECK(fabs(posE->fY - posA->fY) <= 0.001);
  }
  CHECK(fabs(posE->fZ - posA->fZ) <= 0.001);

  unordered_set<string> blacklist = {
      "UUID",
      "Pos",
      "Attributes",
      "Motion",
      "LeftHanded", // left handed skeleton does not exist in BE
      "ForcedAge",
      "Item/tag/map",
      "Owner",
      "HurtByTimestamp",
      "NoAI",
      "Fire",
      "Offers/Recipes/*/sell/tag/Effects/*/EffectDuration", // EffectDuration of suspicious_stew is random
      "Offers/Recipes/*/sell/tag/map",
      "EatingHaystack",

      "Passengers/*/UUID",
      "Passengers/*/Pos",
      "Passengers/*/Attributes",
      "Passengers/*/Motion",
      "Passengers/*/LeftHanded",
      "Passengers/*/ForcedAge",
      "Passengers/*/HurtByTimestamp",
      "Passengers/*/NoAI",
      "Passengers/*/Fire",
  };
  if (id == "minecraft:armor_stand") {
    blacklist.insert("Pose");
    blacklist.insert("Health"); // Default health differs. B = 6, J = 20
  } else if (id == "minecraft:slime" || id == "minecraft:magma_cube") {
    blacklist.insert("wasOnGround"); // wasOnGround does not exist in BE
  } else if (id == "minecraft:bee") {
    blacklist.insert("TicksSincePollination"); // TicksSincePollination does not exist in BE
    blacklist.insert("FlowerPos");
  } else if (id == "minecraft:llama") {
    blacklist.insert("Temper"); // Temper does not exist in BE
  } else if (id == "minecraft:phantom") {
    blacklist.insert("AX");
    blacklist.insert("AY");
    blacklist.insert("AZ");
  } else if (id == "minecraft:turtle") {
    blacklist.insert("TravelPosX");
    blacklist.insert("TravelPosY");
    blacklist.insert("TravelPosZ");
  } else if (id == "minecraft:villager") {
    blacklist.insert("Brain/memories");
    blacklist.insert("Gossips");
    blacklist.insert("LastGossipDecay");
    blacklist.insert("LastRestock");
    blacklist.insert("RestocksToday");
  } else if (id == "minecraft:shulker") {
    blacklist.insert("AttachFace"); // not exists in BE
    blacklist.insert("Peek");       // not exists in BE
  } else if (id == "minecraft:iron_golem") {
    blacklist.insert("AngerTime");
    blacklist.insert("AngryAt");
  } else if (id == "minecraft:zombie_villager") {
    blacklist.insert("PersistenceRequired"); // BE requires "Persistent" = true to keep them alive, but JE doesn't
    blacklist.insert("InWaterTime");
  } else if (id == "minecraft:goat") {
    blacklist.insert("Brain/memories"); // long_jump_cooling_down, ram_cooldown_ticks memories
  } else if (id == "minecraft:piglin") {
    blacklist.insert("TimeInOverworld");

    /*
      "memories": {
        "minecraft:hunted_recently": {
          "ttl": 962, // long
          "value": 1 // byte
        }
      }
    */
    blacklist.insert("Brain/memories");

    blacklist.insert("IsBaby");
    CHECK(copyE->boolean("IsBaby", false) == copyA->boolean("IsBaby", false));
  } else if (id == "minecraft:piglin_brute") {
    blacklist.insert("TimeInOverworld");
  } else if (id == "minecraft:hoglin") {
    blacklist.insert("TimeInOverworld");

    blacklist.insert("IsBaby");
    CHECK(copyE->boolean("IsBaby", false) == copyA->boolean("IsBaby", false));
  } else if (id == "minecraft:chest_minecart") {
    blacklist.insert("LootTableSeed");
  } else if (id == "minecraft:zombie") {
    blacklist.insert("CanBreakDoors");
    blacklist.insert("InWaterTime");
  } else if (id == "minecraft:arrow") {
    blacklist.insert("HasBeenShot");
    blacklist.insert("LeftOwner");
    blacklist.insert("PierceLevel");
    blacklist.insert("ShotFromCrossbow");
    blacklist.insert("crit");
    blacklist.insert("damage");
    blacklist.insert("inGround");
    blacklist.insert("life");
    blacklist.insert("pickup");
    blacklist.insert("shake");
    blacklist.insert("inBlockState");
    blacklist.insert("SoundEvent");
  } else if (id == "minecraft:falling_block") {
    blacklist.insert("DropItem");
    blacklist.insert("FallDistance");
    blacklist.insert("FallHurtAmount");
    blacklist.insert("FallHurtMax");
    blacklist.insert("HurtEntities");
    blacklist.insert("Time"); // JE: int, BE: byte
  } else if (id == "minecraft:item") {
    blacklist.insert("PickupDelay");
  }
  auto itemId = entityE.query("Item/id");
  if (itemId && itemId->asString()) {
    auto name = itemId->asString()->fValue;
    if (name == "minecraft:petrified_oak_slab" || name == "minecraft:furnace_minecart") {
      blacklist.insert("Item/id");
    } else if (name == "minecraft:bundle") {
      // bundle does not exist in BE
      return;
    }
  }
  auto storedEnchantment = entityE.query("Item/tag/StoredEnchantments/0/id");
  if (storedEnchantment && storedEnchantment->asString()) {
    if (storedEnchantment->asString()->fValue == "minecraft:sweeping") { // sweeping does not exist in BE
      blacklist.insert("Item/tag/StoredEnchantments/*/id");
    }
  }
  auto tippedArrowPotion = entityE.query("Item/tag/Potion");
  if (tippedArrowPotion && tippedArrowPotion->asString()) {
    if (tippedArrowPotion->asString()->fValue == "minecraft:luck") { // luck does not exist in BE
      blacklist.insert("Item/tag/Potion");
    }
  }

  for (string const &it : blacklist) {
    Erase(copyE, it);
    Erase(copyA, it);
  }

  DiffCompoundTag(*copyE, *copyA);
}

static shared_ptr<CompoundTag> FindNearestEntity(Pos3d pos, Rotation rot, std::string const &id, vector<shared_ptr<CompoundTag>> const &entities) {
  shared_ptr<CompoundTag> ret = nullptr;
  double minDistance = numeric_limits<double>::max();
  double minRotDifference = numeric_limits<double>::max();
  for (auto const &entity : entities) {
    if (entity->string("id") != id) {
      continue;
    }
    auto p = props::GetPos3d(*entity, "Pos");
    if (!p) {
      continue;
    }
    auto r = props::GetRotation(*entity, "Rotation");
    if (!r) {
      continue;
    }
    double dx = p->fX - pos.fX;
    double dy = p->fY - pos.fY;
    double dz = p->fZ - pos.fZ;
    double distance = hypot(dx, dy, dz);
    double rotDifference = Rotation::DiffDegrees(r->fPitch, rot.fPitch) + Rotation::DiffDegrees(r->fYaw, rot.fYaw);
    if (distance == minDistance) {
      if (rotDifference < minRotDifference) {
        minRotDifference = rotDifference;
        ret = entity;
      }
    } else if (distance < minDistance) {
      minDistance = distance;
      minRotDifference = rotDifference;
      ret = entity;
    }
  }
  return ret;
}

static void CheckTickingBlock(mcfile::je::TickingBlock e, mcfile::je::TickingBlock a) {
  CHECK(e.fI == a.fI);
  CHECK(e.fP == a.fP);
  CHECK(e.fT == a.fT);
  CHECK(e.fX == a.fX);
  CHECK(e.fY == a.fY);
  CHECK(e.fZ == a.fZ);
}

static void CheckChunk(mcfile::je::Region regionE, mcfile::je::Region regionA, int cx, int cz, Dimension dim) {
  auto chunkA = regionA.chunkAt(cx, cz);
  if (!chunkA) {
    return;
  }
  CHECK(regionA.entitiesAt(chunkA->fChunkX, chunkA->fChunkZ, chunkA->fEntities));

  auto chunkE = regionE.chunkAt(cx, cz);
  CHECK(chunkE);
  CHECK(regionE.entitiesAt(chunkE->fChunkX, chunkE->fChunkZ, chunkE->fEntities));

  CHECK(chunkA->minBlockY() == chunkE->minBlockY());
  CHECK(chunkA->maxBlockY() == chunkE->maxBlockY());

  for (int y = chunkE->minBlockY(); y <= chunkE->maxBlockY(); y++) {
    for (int z = chunkE->minBlockZ() + 1; z < chunkE->maxBlockZ(); z++) {
      for (int x = chunkE->minBlockX() + 1; x < chunkE->maxBlockX(); x++) {
        auto blockA = chunkA->blockAt(x, y, z);
        auto blockE = chunkE->blockAt(x, y, z);
        CheckBlock(blockE, blockA, dim, x, y, z);
      }
    }
  }

  int minChunkY = chunkE->fChunkY;
  int maxChunkY = minChunkY;
  for (auto const &sectionE : chunkE->fSections) {
    if (sectionE) {
      maxChunkY = (std::max)(maxChunkY, sectionE->y());
    }
  }
  for (int y = minChunkY * 16; y <= maxChunkY * 16 + 15; y++) {
    for (int z = chunkE->minBlockZ(); z <= chunkE->maxBlockZ(); z++) {
      for (int x = chunkE->minBlockX(); x <= chunkE->maxBlockX(); x++) {
        auto a = chunkA->biomeAt(x, y, z);
        auto e = chunkE->biomeAt(x, y, z);
        CHECK(a == e);
      }
    }
  }

  for (auto const &it : chunkE->fTileEntities) {
    Pos3i pos = it.first;
    shared_ptr<CompoundTag> const &tileE = it.second;
    static unordered_set<string> blacklist({"minecraft:sculk_sensor"});
    if (blacklist.find(tileE->string("id", "")) != blacklist.end()) {
      continue;
    }
    auto found = chunkA->fTileEntities.find(pos);
    if (found == chunkA->fTileEntities.end()) {
      PrintAsJson(cerr, *tileE, {.fTypeHint = true});
      CHECK(false);
      break;
    }
    auto tileA = found->second;
    CheckTileEntity(*tileE, *tileA);
  }

  for (shared_ptr<CompoundTag> const &entityE : chunkE->fEntities) {
    Pos3d posE = *props::GetPos3d(*entityE, "Pos");
    Rotation rotE = *props::GetRotation(*entityE, "Rotation");
    auto id = entityE->string("id");
    shared_ptr<CompoundTag> entityA = FindNearestEntity(posE, rotE, *id, chunkA->fEntities);
    if (entityA) {
      CheckEntity(*id, *entityE, *entityA);
    } else {
      PrintAsJson(cerr, *entityE, {.fTypeHint = true});
      CHECK(false);
      break;
    }
  }

  CHECK(chunkE->fLiquidTicks.size() == chunkA->fLiquidTicks.size());
  for (size_t i = 0; i < chunkE->fLiquidTicks.size(); i++) {
    mcfile::je::TickingBlock tbE = chunkE->fLiquidTicks[i];
    mcfile::je::TickingBlock tbA = chunkA->fLiquidTicks[i];
    CheckTickingBlock(tbE, tbA);
  }

  CHECK(chunkE->fTileTicks.size() == chunkA->fTileTicks.size());
  for (size_t i = 0; i < chunkE->fTileTicks.size(); i++) {
    mcfile::je::TickingBlock tbE = chunkE->fTileTicks[i];
    mcfile::je::TickingBlock tbA = chunkA->fTileTicks[i];
    CheckTickingBlock(tbE, tbA);
  }
}

static std::shared_ptr<CompoundTag> ReadLevelDat(fs::path p) {
  vector<uint8_t> buffer;
  if (!file::GetGzContents(p, buffer)) {
    return nullptr;
  }
  return CompoundTag::Read(buffer, {.fLittleEndian = false});
}

static void CheckLevelDat(fs::path pathE, fs::path pathA) {
  auto e = ReadLevelDat(pathE);
  auto a = ReadLevelDat(pathA);
  CHECK(e);
  CHECK(a);

  auto dataE = e->compoundTag("Data");
  auto dataA = a->compoundTag("Data");
  CHECK(dataE);
  CHECK(dataA);

  unordered_set<string> blacklist = {
      "BorderCenterX",
      "BorderCenterY",
      "BorderCenterZ",
      "BorderSafeZone",
      "BorderSize",
      "BorderSizeLerpTarget",
      "BorderSizeLerpTime",
      "BorderWarningBlocks",
      "BorderWarningTime",
      "BorderDamagePerBlock",
      "CustomBossEvents",
      "DifficultyLocked",
      "DragonFight", //TODO: remove this
  };
  unordered_set<string> ignoredGameRules = {
      "announceAdvancements",
      "disableElytraMovementCheck",
      "disableRaids",
      "doLimitedCrafting",
      "doPatrolSpawning",
      "doTraderSpawning",
      "forgiveDeadPlayers",
      "logAdminCommands",
      "maxEntityCramming",
      "reducedDebugInfo",
      "spectatorsGenerateChunks",
      "universalAnger",
      "playersSleepingPercentage",
  };
  for (string rule : ignoredGameRules) {
    blacklist.insert("GameRules/" + rule);
  }
  for (string const &s : blacklist) {
    Erase(dataE, s);
    Erase(dataA, s);
  }
  DiffCompoundTag(*dataE, *dataA);
}

TEST_CASE("j2b2j") {
  fs::path thisFile(__FILE__);
  auto dataDir = thisFile.parent_path() / "data";
  auto in = dataDir / "je2be-test";
  auto tmp = mcfile::File::CreateTempDir(fs::temp_directory_path());
  CHECK(tmp);
  defer {
    fs::remove_all(*tmp);
  };

  // java -> bedrock
  auto outB = mcfile::File::CreateTempDir(*tmp);
  CHECK(outB);
  je2be::tobe::InputOption io;
  bool multithread = true;
#if 1
  int radius = 32;
  for (int cz = -radius; cz < radius; cz++) {
    for (int cx = -radius; cx < radius; cx++) {
      io.fChunkFilter.insert(Pos2i(cx, cz));
    }
  }
#else
  io.fDimensionFilter.insert(mcfile::Dimension::Overworld);
  io.fChunkFilter.insert(Pos2i(0, 0));
  multithread = false;
#endif
  je2be::tobe::OutputOption oo;
  je2be::tobe::Converter tobe(in, io, *outB, oo);
  CHECK(tobe.run(thread::hardware_concurrency()));

  // bedrock -> java
  auto outJ = mcfile::File::CreateTempDir(*tmp);
  CHECK(outJ);
  je2be::toje::Converter toje(*outB, *outJ);
  CHECK(toje.run(thread::hardware_concurrency()));

  // Compare initial Java input and final Java output.

  CheckLevelDat(in / "level.dat", *outJ / "level.dat");

  for (auto dim : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
    if (!io.fDimensionFilter.empty()) {
      if (io.fDimensionFilter.find(dim) == io.fDimensionFilter.end()) {
        continue;
      }
    }
    auto regionDirA = io.getWorldDirectory(*outJ, dim) / "region";
    auto regionDirE = io.getWorldDirectory(in, dim) / "region";

    for (auto it : fs::directory_iterator(regionDirA)) {
      if (!it.is_regular_file()) {
        continue;
      }
      auto fileA = it.path();
      if (fileA.extension() != ".mca") {
        continue;
      }

      deque<future<void>> futures;
      unique_ptr<hwm::task_queue> pool;
      if (multithread) {
        pool.reset(new hwm::task_queue(thread::hardware_concurrency()));
      }

      auto regionA = mcfile::je::Region::MakeRegion(fileA);
      CHECK(regionA);

      auto fileE = regionDirE / fileA.filename();
      auto regionE = mcfile::je::Region::MakeRegion(fileE);
      CHECK(regionE);

      for (int cz = regionA->minChunkZ(); cz <= regionA->maxChunkZ(); cz++) {
        for (int cx = regionA->minChunkX(); cx <= regionA->maxChunkX(); cx++) {
          if (!io.fChunkFilter.empty()) {
            if (io.fChunkFilter.find(Pos2i(cx, cz)) == io.fChunkFilter.end()) {
              continue;
            }
            if (multithread) {
              futures.push_back(move(pool->enqueue(CheckChunk, *regionE, *regionA, cx, cz, dim)));
            } else {
              CheckChunk(*regionE, *regionA, cx, cz, dim);
            }
          }
        }
      }

      for (auto &f : futures) {
        f.get();
      }
    }
  }
}
