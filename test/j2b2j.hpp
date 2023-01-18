#pragma once

static std::mutex sMutCerr;

static void CheckTileEntity(CompoundTag const &expected, CompoundTag const &actual);

static void CheckBlockWithIgnore(mcfile::je::Block const &e, mcfile::je::Block const &a, std::initializer_list<std::string> ignore) {
  CHECK(e.fName == a.fName);
  map<string, optional<string>> op;
  for (string const &p : ignore) {
    op[p] = nullopt;
  }
  auto blockE = e.applying(op);
  auto blockA = a.applying(op);
  CHECK(blockA->toString() == blockE->toString());
}

static void CheckBlock(shared_ptr<mcfile::je::Block const> const &blockE, shared_ptr<mcfile::je::Block const> const &blockA, Dimension dim, int x, int y, int z) {
  unordered_map<string_view, string> fallbackJtoB;
  fallbackJtoB["minecraft:petrified_oak_slab"] = "minecraft:oak_slab"; // does not exist in BE. should be replaced to oak_slab when java -> bedrock.
  fallbackJtoB["minecraft:cave_air"] = "minecraft:air";

  unordered_map<string_view, string> fallbackBtoJ;
  fallbackBtoJ["minecraft:frame"] = "minecraft:air";      // frame should be converted as an entity.
  fallbackBtoJ["minecraft:glow_frame"] = "minecraft:air"; // frame should be converted as an entity.

  if (blockA && blockE) {
    auto foundJtoB = fallbackJtoB.find(blockE->fName);
    if (foundJtoB == fallbackJtoB.end()) {
      auto foundBtoJ = fallbackBtoJ.find(blockA->fName);
      if (foundBtoJ == fallbackBtoJ.end()) {
        if (blockE->fName == "minecraft:red_mushroom_block" || blockE->fName == "minecraft:brown_mushroom_block" || blockE->fName == "minecraft:sculk_sensor") {
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
        } else if (blockE->fName == "minecraft:mangrove_propagule") {
          CheckBlockWithIgnore(*blockE, *blockA, {"stage"});
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

static void Erase(shared_ptr<CompoundTag> t, string const &path) {
  auto keys = mcfile::String::Split(path, '/');
  if (keys.size() == 1) {
    t->erase(keys[0]);
    return;
  }
  auto index = strings::Toi(keys[1]);
  if (keys[1] == "*") {
    assert(keys.size() >= 3);
    string nextPath = keys[2];
    for (int i = 3; i < keys.size(); i++) {
      nextPath += "/" + keys[i];
    }
    if (auto list = t->listTag(keys[0]); list) {
      for (auto const &it : *list) {
        if (it->type() != Tag::Type::Compound) {
          continue;
        }
        shared_ptr<CompoundTag> c = dynamic_pointer_cast<CompoundTag>(it);
        if (!c) {
          continue;
        }
        Erase(c, nextPath);
      }
    } else if (auto c = t->compoundTag(keys[0]); c) {
      for (auto const &it : *c) {
        if (it.second->type() != Tag::Type::Compound) {
          continue;
        }
        shared_ptr<CompoundTag> c = dynamic_pointer_cast<CompoundTag>(it.second);
        if (!c) {
          continue;
        }
        Erase(c, nextPath);
      }
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
    ostringstream out;
    vector<string> linesE = mcfile::String::Split(jsonE, '\n');
    vector<string> linesA = mcfile::String::Split(jsonA, '\n');
    size_t lines = (std::min)(linesE.size(), linesA.size());
    out << "actual:" << endl;
    for (size_t i = 0; i < lines; i++) {
      if (i < linesE.size() && linesA[i] != linesE[i]) {
        out << ">> ";
      } else {
        out << "   ";
      }
      out << i << ":\t" << linesA[i] << endl;
    }
    out << "expected:" << endl;
    for (int i = 0; i < lines; i++) {
      if (i < linesA.size() && linesA[i] != linesE[i]) {
        out << ">> ";
      } else {
        out << "   ";
      }
      out << i << ":\t" << linesE[i] << endl;
    }
    lock_guard<mutex> lock(sMutCerr);
    cerr << out.str();
    CHECK(false);
  }
}

static void CheckItem(CompoundTag const &itemE, CompoundTag const &itemA) {
  unordered_set<string> blacklist = {
      "tag/map",
      "tag/BlockEntityTag",
  };

  CompoundTagPtr copyE = itemE.copy();
  CompoundTagPtr copyA = itemA.copy();

  auto itemId = itemE.string("id");
  if (itemId) {
    if (*itemId == "minecraft:petrified_oak_slab" || *itemId == "minecraft:furnace_minecart") {
      blacklist.insert("id");
    } else if (*itemId == "minecraft:bundle") {
      // bundle does not exist in BE
      return;
    }
  }
  auto storedEnchantment = itemE.query("tag/StoredEnchantments/0/id");
  if (storedEnchantment && storedEnchantment->asString()) {
    if (storedEnchantment->asString()->fValue == "minecraft:sweeping") { // sweeping does not exist in BE
      blacklist.insert("tag/StoredEnchantments/*/id");
    }
  }
  auto tippedArrowPotion = itemE.query("tag/Potion");
  if (tippedArrowPotion && tippedArrowPotion->asString()) {
    if (tippedArrowPotion->asString()->fValue == "minecraft:luck") { // luck does not exist in BE
      blacklist.insert("tag/Potion");
    }
  }

  auto tileE = itemE.query("tag/BlockEntityTag");
  auto tileA = itemA.query("tag/BlockEntityTag");
  if (tileE && tileE->type() != Tag::Type::End) {
    REQUIRE(tileA);
    REQUIRE(tileE->type() == Tag::Type::Compound);
    CHECK(tileE->type() == tileA->type());
    CheckTileEntity(*tileE->asCompound(), *tileA->asCompound());
  } else if (tileA && tileA->type() != Tag::Type::End) {
    CHECK(false);
  }

  for (string const &it : blacklist) {
    Erase(copyE, it);
    Erase(copyA, it);
  }

  DiffCompoundTag(*copyE, *copyA);
}

static void CheckTileEntity(CompoundTag const &expected, CompoundTag const &actual) {
  auto copyE = expected.copy();
  auto copyA = actual.copy();

  unordered_set<string> tagBlacklist = {
      "LootTableSeed",           // chest in dungeon etc.
      "RecipesUsed",             // furnace, blast_furnace, and smoker
      "LastOutput",              // command_block
      "author",                  // structure_block
      "Book/tag/resolved",       // written_book
      "Book/tag/filtered_title", // written_book
      "Items",
      "Levels",          // beacon. Sometimes reset to 0 in JE
      "SpawnPotentials", // mob_spawner, SpawnPotentials sometimes doesn't contained in JE
  };
  auto id = expected.string("id", "");
  if (id == "minecraft:sculk_shrieker") {
    tagBlacklist.insert("listener");
    tagBlacklist.insert("warning_level");
  } else if (id == "minecraft:sculk_catalyst") {
    tagBlacklist.insert("cursors");
  } else if (id == "minecraft:jukebox") {
    tagBlacklist.insert("IsPlaying");
    tagBlacklist.insert("TickCount");
    tagBlacklist.insert("RecordStartTick");
  }
  auto itemsE = expected.listTag("Items");
  auto itemsA = actual.listTag("Items");
  if (itemsE) {
    REQUIRE(itemsA);
    CHECK(itemsE->size() == itemsA->size());
    for (size_t i = 0; i < itemsE->size(); i++) {
      auto itemE = itemsE->at(i);
      auto itemA = itemsA->at(i);
      REQUIRE(itemE);
      REQUIRE(itemA);
      CHECK(itemE->type() == Tag::Type::Compound);
      CHECK(itemA->type() == Tag::Type::Compound);
      CheckItem(*itemE->asCompound(), *itemA->asCompound());
    }
  } else if (itemsA) {
    CHECK(false);
  }
  for (string const &b : tagBlacklist) {
    Erase(copyE, b);
    Erase(copyA, b);
  }
  DiffCompoundTag(*copyE, *copyA);
}

static void CheckBrain(CompoundTag const &brainE, CompoundTag const &brainA) {
  static unordered_set<string> const blacklist = {
      // allay, etc
      "memories/minecraft:gaze_cooldown_ticks",
      // goat, frog
      "memories/minecraft:long_jump_cooling_down",
      // goat
      "memories/minecraft:ram_cooldown_ticks memories",
      "memories/minecraft:ram_cooldown_ticks",
      // piglin
      /*
        "minecraft:hunted_recently": {
          "ttl": 962, // long
          "value": 1 // byte
        }
       */
      "memories/minecraft:hunted_recently",
      // allay
      "memories/minecraft:liked_player",
      // villager
      "memories/minecraft:job_site",
      "memories/minecraft:last_worked_at_poi",
      "memories/minecraft:meeting_point",
      "memories/minecraft:golem_detected_recently",
      "memories/minecraft:potential_job_site",
      "memories/minecraft:home",
      "memories/minecraft:last_slept",
      "memories/minecraft:last_woken",
      // piglin etc.
      "memories/minecraft:angry_at", //TODO:
  };
  auto copyE = brainE.copy();
  auto copyA = brainA.copy();

  auto likedPlayerA = copyA->query("memories/minecraft:liked_player");
  auto likedPlayerE = copyE->query("memories/minecraft:liked_player");
  CHECK((likedPlayerA == nullptr) == (likedPlayerE == nullptr));

  for (string const &b : blacklist) {
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
      "Owner",
      "HurtByTimestamp",
      "NoAI",
      "Fire",
      "Offers/Recipes/*/sell/tag/Effects/*/EffectDuration", // EffectDuration of suspicious_stew is random
      "Offers/Recipes/*/sell/tag/map",
      "EatingHaystack",
      "PersistenceRequired", // This is default false for animals in JE. BE reqires Persistent = true even for animals. So this property cannot completely recover in round-trip conversion.
      "Leash/UUID",
      "listener",
      "HandDropChances", // TODO: Is there any way identifying hand items was picked-up thing or not?
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
  } else if (id == "minecraft:piglin") {
    blacklist.insert("TimeInOverworld");
    blacklist.insert("IsBaby");
    CHECK(copyE->boolean("IsBaby", false) == copyA->boolean("IsBaby", false));
  } else if (id == "minecraft:piglin_brute") {
    blacklist.insert("TimeInOverworld");
  } else if (id == "minecraft:hoglin") {
    blacklist.insert("TimeInOverworld");
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
  } else if (id == "minecraft:trader_llama") {
    blacklist.insert("DespawnDelay"); // "TimeStamp" does not exist in BE.
  } else if (id == "minecraft:warden") {
    blacklist.insert("anger");
  } else if (id == "minecraft:zombified_piglin") {
    blacklist.insert("CanBreakDoors");
    blacklist.insert("AngerTime");
    blacklist.insert("AngryAt");
    auto angryAtA = copyA->intArrayTag("AngryAt");
    auto angryAtE = copyE->intArrayTag("AngryAt");
    CHECK((angryAtE == nullptr) == (angryAtA == nullptr));
  } else if (id == "minecraft:camel") {
    blacklist.insert("LastPoseTick");
  } else if (id == "minecraft:wandering_trader") {
    blacklist.insert("WanderTarget");
  }

  for (string const &it : blacklist) {
    Erase(copyE, it);
    Erase(copyA, it);
  }

  auto itemE = entityE.compoundTag("Item");
  auto itemA = entityA.compoundTag("Item");
  copyE->erase("Item");
  copyA->erase("Item");
  if (itemE) {
    REQUIRE(itemA);
    CheckItem(*itemE, *itemA);
  }

  auto passengersE = entityE.listTag("Passengers");
  auto passengersA = entityA.listTag("Passengers");
  copyE->erase("Passengers");
  copyA->erase("Passengers");
  if (passengersE) {
    REQUIRE(passengersA);
    CHECK(passengersE->size() == passengersA->size());
    for (int i = 0; i < passengersE->size(); i++) {
      auto passengerE = passengersE->at(i);
      auto passengerA = passengersA->at(i);
      REQUIRE(passengerE->type() == Tag::Type::Compound);
      CHECK(passengerE->type() == passengerA->type());
      auto id = passengerE->asCompound()->string("id");
      REQUIRE(id);
      CheckEntity(*id, *passengerE->asCompound(), *passengerA->asCompound());
    }
  } else if (passengersA) {
    CHECK(false);
  }

  auto brainE = entityE.compoundTag("Brain");
  auto brainA = entityA.compoundTag("Brain");
  copyE->erase("Brain");
  copyA->erase("Brain");
  if (brainE) {
    REQUIRE(brainA);
    CheckBrain(*brainE, *brainA);
  } else if (brainA) {
    CHECK(false);
  }

  auto inventoryE = entityE.listTag("Inventory");
  auto inventoryA = entityA.listTag("Inventory");
  copyE->erase("Inventory");
  copyA->erase("Inventory");
  if (inventoryE) {
    REQUIRE(inventoryA);
    CHECK(inventoryE->size() == inventoryA->size());
    for (int i = 0; i < inventoryE->size(); i++) {
      auto itemE = inventoryE->at(i);
      auto itemA = inventoryA->at(i);
      REQUIRE(itemE->type() == Tag::Type::Compound);
      CHECK(itemE->type() == itemA->type());
      CheckItem(*itemE->asCompound(), *itemA->asCompound());
    }
  } else if (inventoryA) {
    CHECK(false);
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

static std::optional<mcfile::je::TickingBlock> FindNearestTickingBlock(int x, int y, int z, std::vector<mcfile::je::TickingBlock> const &list) {
  double minDistance = std::numeric_limits<double>::max();
  std::optional<mcfile::je::TickingBlock> ret;
  for (mcfile::je::TickingBlock const &tb : list) {
    double distance = hypot(tb.fX - x, tb.fY - y, tb.fZ - z);
    if (distance < minDistance) {
      ret = tb;
      minDistance = distance;
    }
  }
  return ret;
}

static void CheckTickingBlock(mcfile::je::TickingBlock e, mcfile::je::TickingBlock a) {
  CHECK(e.fI == a.fI);
  //CHECK(e.fP == a.fP); "P" does not exist in BE
  CHECK(e.fT == a.fT);
  CHECK(e.fX == a.fX);
  CHECK(e.fY == a.fY);
  CHECK(e.fZ == a.fZ);
}

static void CheckHeightmaps(CompoundTag const &expected, CompoundTag const &actual) {
  static set<string> const sTypes = {"MOTION_BLOCKING",
                                     "MOTION_BLOCKING_NO_LEAVES",
                                     "OCEAN_FLOOR",
                                     "WORLD_SURFACE"};
  for (auto const &type : sTypes) {
    auto tagA = actual.longArrayTag(type);
    auto tagE = expected.longArrayTag(type);
    REQUIRE(tagA);
    REQUIRE(tagE);
    auto mapA = Heightmap::Load(*tagA);
    auto mapE = Heightmap::Load(*tagE);
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        auto e = mapE->getUnchecked(x, z);
        auto a = mapA->getUnchecked(x, z);
        CHECK(e == a);
      }
    }
  }
}

static void CheckSectionLight(Pos3i const &origin, std::vector<uint8_t> &e, std::vector<uint8_t> &a) {
  auto dataE = mcfile::Data4b3dView::Make(origin, 16, 16, 16, &e);
  auto dataA = mcfile::Data4b3dView::Make(origin, 16, 16, 16, &a);
  REQUIRE(dataE);
  REQUIRE(dataA);
#if 1
  for (int y = 0; y < 16; y++) {
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        auto vE = dataE->getUnchecked(origin + Pos3i{x, y, z});
        auto vA = dataA->getUnchecked(origin + Pos3i{x, y, z});
        CHECK(vE == vA);
      }
    }
  }
#else
  for (int y = 0; y < 16; y++) {
    bool ok = true;
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        auto vE = dataE->getUnchecked(origin + Pos3i{x, y, z});
        auto vA = dataA->getUnchecked(origin + Pos3i{x, y, z});
        if (vE != vA) {
          ok = false;
          break;
        }
      }
    }
    if (!ok) {
      printf("%3d", origin.fY + y);
      for (int x = 0; x < 16; x++) {
        printf("%6d ", x + origin.fX);
      }
      cout << endl;
      for (int z = 0; z < 16; z++) {
        printf("%2d:", z + origin.fZ);
        for (int x = 0; x < 16; x++) {
          int vE = dataE->getUnchecked(origin + Pos3i{x, y, z});
          int vA = dataA->getUnchecked(origin + Pos3i{x, y, z});
          string bra = "[";
          string ket = "]";
          if (vE == vA) {
            bra = " ";
            ket = " ";
          }
          printf("%s%2d,%2d%s", bra.c_str(), vE, vA, ket.c_str());
        }
        cout << endl;
      }
      cout << endl;
    }
  }
#endif
}

static void CheckChunkLight(mcfile::je::Chunk const &chunkE, mcfile::je::Chunk const &chunkA) {
  REQUIRE(chunkA.fSections.size() == chunkE.fSections.size());
  for (int i = 0; i < chunkE.fSections.size(); i++) {
    auto const &sectionE = chunkE.fSections[i];
    auto const &sectionA = chunkA.fSections[i];
    Pos3i origin{chunkE.fChunkX * 16, sectionE->y() * 16, chunkE.fChunkZ * 16};
    REQUIRE(sectionE);
    REQUIRE(sectionA);
    REQUIRE(sectionE->y() == sectionA->y());
    if (!sectionE->fSkyLight.empty()) {
      CHECK(sectionE->fSkyLight.size() == sectionA->fSkyLight.size());
      CheckSectionLight(origin, sectionE->fSkyLight, sectionA->fSkyLight);
    }
    if (!sectionE->fBlockLight.empty()) {
      CHECK(sectionE->fBlockLight.size() == sectionA->fBlockLight.size());
      CheckSectionLight(origin, sectionE->fBlockLight, sectionA->fBlockLight);
    }
  }
}

static void CheckChunk(mcfile::je::Region const &regionE, mcfile::je::Region const &regionA, int cx, int cz, Dimension dim) {
  auto chunkA = regionA.writableChunkAt(cx, cz);
  if (!chunkA) {
    return;
  }
  CHECK(regionA.entitiesAt(chunkA->fChunkX, chunkA->fChunkZ, chunkA->fEntities));

  auto chunkE = regionE.writableChunkAt(cx, cz);
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
      auto block = chunkE->blockAt(pos);
      ostringstream out;
      PrintAsJson(out, *tileE, {.fTypeHint = true});
      lock_guard<mutex> lock(sMutCerr);
      cerr << out.str();
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
      ostringstream out;
      PrintAsJson(out, *entityE, {.fTypeHint = true});
      lock_guard<mutex> lock(sMutCerr);
      cerr << out.str();
      CHECK(false);
      break;
    }
  }

  CHECK(chunkE->fLiquidTicks.size() == chunkA->fLiquidTicks.size());
  for (size_t i = 0; i < chunkE->fLiquidTicks.size(); i++) {
    mcfile::je::TickingBlock tbE = chunkE->fLiquidTicks[i];
    auto tbA = FindNearestTickingBlock(tbE.fX, tbE.fY, tbE.fZ, chunkA->fLiquidTicks);
    CHECK(tbA);
    CheckTickingBlock(tbE, *tbA);
  }

  CHECK(chunkE->fTileTicks.size() == chunkA->fTileTicks.size());
  for (size_t i = 0; i < chunkE->fTileTicks.size(); i++) {
    mcfile::je::TickingBlock tbE = chunkE->fTileTicks[i];
    auto tbA = FindNearestTickingBlock(tbE.fX, tbE.fY, tbE.fZ, chunkA->fTileTicks);
    CHECK(tbA);
    CheckTickingBlock(tbE, *tbA);
  }

  auto heightMapsE = chunkE->fRoot->compoundTag("Heightmaps");
  auto heightMapsA = chunkA->fRoot->compoundTag("Heightmaps");
  REQUIRE(heightMapsE);
  REQUIRE(heightMapsA);
  CheckHeightmaps(*heightMapsE, *heightMapsA);

  CheckChunkLight(*chunkE, *chunkA);
}

static std::shared_ptr<CompoundTag> ReadLevelDat(fs::path const &p) {
  auto s = make_shared<mcfile::stream::GzFileInputStream>(p);
  return CompoundTag::Read(s, Endian::Big);
}

static void CheckLevelDat(fs::path const &pathE, fs::path const &pathA) {
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
      "DragonFight/Dragon",
      "LastPlayed", // JE: milli-seconds, BE: seconds
      "ScheduledEvents",
      "SpawnAngle",
      "WanderingTraderId",
      "WanderingTraderSpawnChance",
      "WanderingTraderSpawnDelay",
      "WasModded",
      "clearWeatherTime",
      "hardcore",
      "initialized",
  };
  unordered_set<string> ignoredGameRules = {
      "announceAdvancements",
      "disableElytraMovementCheck",
      "disableRaids",
      "doLimitedCrafting",
      "forgiveDeadPlayers",
      "logAdminCommands",
      "maxEntityCramming",
      "reducedDebugInfo",
      "spectatorsGenerateChunks",
      "universalAnger",
      "playersSleepingPercentage",
      "doWardenSpawning",
      "blockExplosionDropDecay",
      "globalSoundEvents",
      "lavaSourceConversion",
      "tntExplosionDropDecay",
      "waterSourceConversion",
      "mobExplosionDropDecay",
      "snowAccumulationHeight",
  };
  for (string const &rule : ignoredGameRules) {
    blacklist.insert("GameRules/" + rule);
  }
  unordered_set<string> ignoredPlayerAttributes = {
      "Attributes",
      "EnderItems/*/tag/HideFlags", //?
      "EnderItems/*/tag/map",
      "Fire",
      "Motion",
      "Score",
      "foodTickTimer",
      "previousPlayerGameType",
      "recipeBook",
      "HurtByTimestamp",
      "SpawnAngle",
      "SpawnForced",
      "ActiveEffects", // TODO:
  };
  for (string const &rule : ignoredPlayerAttributes) {
    blacklist.insert("Player/" + rule);
  }
  for (string const &s : blacklist) {
    Erase(dataE, s);
    Erase(dataA, s);
  }
  auto playerE = dataE->compoundTag("Player");
  auto playerA = dataA->compoundTag("Player");
  CheckEntity("player", *playerE, *playerA);

  dataE->erase("Player");
  dataA->erase("Player");
  DiffCompoundTag(*dataE, *dataA);
}

static void RemoveEmpty(CompoundTag &t) {
  using namespace std;
  vector<string> remove;
  for (auto &it : t) {
    if (auto c = dynamic_pointer_cast<CompoundTag>(it.second); c) {
      RemoveEmpty(*c);
      if (c->empty()) {
        remove.push_back(it.first);
      }
    } else if (auto l = dynamic_pointer_cast<ListTag>(it.second); l) {
      if (l->empty()) {
        remove.push_back(it.first);
      }
    }
  }
  for (auto const &r : remove) {
    t.erase(r);
  }
}

static void CheckPoi(mcfile::je::Region const &regionE, mcfile::je::Region const &regionA, int cx, int cz, Dimension dim) {
  auto poiE = regionE.exportToNbt(cx, cz);
  if (poiE) {
    static std::unordered_set<std::string> const blacklist = {
        "Sections/*/Valid",
        "DataVersion"};
    for (string const &s : blacklist) {
      Erase(poiE, s);
    }
    RemoveEmpty(*poiE);
    if (poiE->empty()) {
      poiE.reset();
    }
  }
  auto poiA = regionA.exportToNbt(cx, cz);
  CHECK((bool)poiE == (bool)poiA);
  if (!poiE || !poiA) {
    return;
  }
  auto sectionsE = poiE->compoundTag("Sections");
  auto sectionsA = poiA->compoundTag("Sections");
  CHECK(sectionsE);
  CHECK(sectionsA);
  CHECK(sectionsE->fValue.size() == sectionsA->fValue.size());
  for (auto const &itE : *sectionsE) {
    auto sectionE = itE.second->asCompound();
    CHECK(sectionE);

    auto sectionATag = (*sectionsA)[itE.first];
    CHECK(sectionATag);
    auto sectionA = sectionATag->asCompound();
    CHECK(sectionA);

    auto recordsE = sectionE->listTag("Records");
    auto recordsA = sectionA->listTag("Records");
    if (recordsE) {
      if (recordsE->empty()) {
        if (recordsA) {
          CHECK(recordsA->empty());
        } else {
          CHECK(true);
        }
      } else {
        for (auto const &rE : *recordsE) {
          auto recordE = rE->asCompound();
          CHECK(recordE);
          auto posE = recordE->intArrayTag("pos");
          CHECK(posE);
          CHECK(posE->fValue.size() == 3);
          int x = posE->fValue[0];
          int y = posE->fValue[1];
          int z = posE->fValue[2];
          bool found = false;
          for (auto const &rA : *recordsA) {
            auto recordA = rA->asCompound();
            if (!recordA) {
              continue;
            }
            auto posA = recordA->intArrayTag("pos");
            if (!posA) {
              continue;
            }
            if (posA->fValue.size() != 3) {
              continue;
            }
            if (posA->fValue[0] == x && posA->fValue[1] == y && posA->fValue[2] == z) {
              CHECK(recordE->string("type") == recordA->string("type"));
              auto freeTicketsE = recordE->int32("free_tickets");
              auto freeTicketsA = recordA->int32("free_tickets");
              CHECK(freeTicketsE);
              CHECK(freeTicketsA);
              CHECK(*freeTicketsE <= *freeTicketsA);
              found = true;
              break;
            }
          }
          CHECK(found);
        }
        CHECK(recordsE->fValue.size() == recordsA->fValue.size());
      }
    } else {
      if (recordsA) {
        CHECK(recordsA->empty());
      } else {
        CHECK(true);
      }
    }
  }
}

static void TestJavaToBedrockToJava(fs::path in) {
  auto tmp = mcfile::File::CreateTempDir(fs::temp_directory_path());
  CHECK(tmp);
  defer {
    fs::remove_all(*tmp);
  };

  // java -> bedrock
  auto outB = mcfile::File::CreateTempDir(*tmp);
  CHECK(outB);
  je2be::tobe::Options optB;
  optB.fTempDirectory = mcfile::File::CreateTempDir(fs::temp_directory_path());
  defer {
    if (optB.fTempDirectory) {
      Fs::DeleteAll(*optB.fTempDirectory);
    }
  };
  bool multithread = true;
  unordered_set<Pos2i, Pos2iHasher> chunks;
#if 1
  Pos2i center(0, 0);
  int radius = 31;
  for (int cz = center.fZ - radius - 1; cz < center.fZ + radius + 1; cz++) {
    for (int cx = center.fX - radius - 1; cx < center.fX + radius + 1; cx++) {
      optB.fChunkFilter.insert({cx, cz});
    }
  }
  for (int cz = center.fZ - radius; cz < center.fZ + radius; cz++) {
    for (int cx = center.fX - radius; cx < center.fX + radius; cx++) {
      chunks.insert({cx, cz});
    }
  }
#else
  optB.fDimensionFilter.insert(mcfile::Dimension::Overworld);
  Pos2i center(0, 0);
  int radius = 0;
  for (int cz = center.fZ - radius - 1; cz <= center.fZ + radius + 1; cz++) {
    for (int cx = center.fX - radius - 1; cx <= center.fX + radius + 1; cx++) {
      optB.fChunkFilter.insert({cx, cz});
    }
  }
  for (int cz = center.fZ - radius; cz <= center.fZ + radius; cz++) {
    for (int cx = center.fX - radius; cx <= center.fX + radius; cx++) {
      chunks.insert({cx, cz});
    }
  }
  multithread = false;
#endif
  je2be::toje::Options optJ;
  optJ.fTempDirectory = mcfile::File::CreateTempDir(fs::temp_directory_path());
  optJ.fDimensionFilter = optB.fDimensionFilter;
  optJ.fChunkFilter = optB.fChunkFilter;
  defer {
    if (optJ.fTempDirectory) {
      Fs::DeleteAll(*optJ.fTempDirectory);
    }
  };

  Status st = je2be::tobe::Converter::Run(in, *outB, optB, thread::hardware_concurrency());
  CHECK(st.ok());

  // bedrock -> java
  auto outJ = mcfile::File::CreateTempDir(*tmp);
  CHECK(outJ);
  st = je2be::toje::Converter::Run(*outB, *outJ, optJ, thread::hardware_concurrency());
  CHECK(st.ok());

  // Compare initial Java input and final Java output.

  CheckLevelDat(in / "level.dat", *outJ / "level.dat");

  for (auto dim : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
    if (!optB.fDimensionFilter.empty()) {
      if (optB.fDimensionFilter.find(dim) == optB.fDimensionFilter.end()) {
        continue;
      }
    }
    auto regionDirA = optB.getWorldDirectory(*outJ, dim) / "region";
    auto regionDirE = optB.getWorldDirectory(in, dim) / "region";

    error_code ec;
    for (auto const &it : fs::directory_iterator(regionDirA, ec)) {
      if (!it.is_regular_file()) {
        continue;
      }
      auto const &fileA = it.path();
      if (fileA.extension() != ".mca") {
        continue;
      }

      auto regionA = mcfile::je::Region::MakeRegion(fileA);
      CHECK(regionA);

      auto fileE = regionDirE / fileA.filename();
      auto regionE = mcfile::je::Region::MakeRegion(fileE);
      CHECK(regionE);

      vector<Pos2i> regionChunks;
      for (int cz = regionA->minChunkZ(); cz <= regionA->maxChunkZ(); cz++) {
        for (int cx = regionA->minChunkX(); cx <= regionA->maxChunkX(); cx++) {
          if (chunks.find({cx, cz}) == chunks.end()) {
            continue;
          }
          regionChunks.push_back({cx, cz});
        }
      }
      if (multithread) {
        Parallel::Reduce<Pos2i, bool>(
            regionChunks,
            thread::hardware_concurrency(),
            true,
            [regionE, regionA, dim](Pos2i const &pos) -> bool {
              CheckChunk(*regionE, *regionA, pos.fX, pos.fZ, dim);
              return true;
            },
            Parallel::MergeBool);
      } else {
        for (auto const &pos : regionChunks) {
          CheckChunk(*regionE, *regionA, pos.fX, pos.fZ, dim);
        }
      }
    }

    auto poiDirA = optB.getWorldDirectory(*outJ, dim) / "poi";
    auto poiDirE = optB.getWorldDirectory(in, dim) / "poi";
    ec.clear();
    for (auto const &it : fs::directory_iterator(poiDirA, ec)) {
      if (!it.is_regular_file()) {
        continue;
      }
      auto const &fileA = it.path();
      if (fileA.extension() != ".mca") {
        continue;
      }

      auto regionA = mcfile::je::Region::MakeRegion(fileA);
      CHECK(regionA);

      auto fileE = poiDirE / fileA.filename();
      auto regionE = mcfile::je::Region::MakeRegion(fileE);
      CHECK(regionE);

      vector<Pos2i> regionChunks;
      for (int cz = regionA->minChunkZ(); cz <= regionA->maxChunkZ(); cz++) {
        for (int cx = regionA->minChunkX(); cx <= regionA->maxChunkX(); cx++) {
          if (chunks.find({cx, cz}) == chunks.end()) {
            continue;
          }
          regionChunks.push_back({cx, cz});
        }
      }
      if (multithread) {
        Parallel::Reduce<Pos2i, bool>(
            regionChunks,
            thread::hardware_concurrency(),
            true,
            [regionE, regionA, dim](Pos2i const &pos) -> bool {
              CheckPoi(*regionE, *regionA, pos.fX, pos.fZ, dim);
              return true;
            },
            Parallel::MergeBool);
      } else {
        for (auto const &pos : regionChunks) {
          CheckPoi(*regionE, *regionA, pos.fX, pos.fZ, dim);
        }
      }
    }
  }
}
