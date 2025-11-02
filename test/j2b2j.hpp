#pragma once

#include <minecraft-file.hpp>

using namespace mcfile::u8stream;

#define J2B2J_MAX_CHUNK_X (30)

static std::mutex sMutCerr;

static void CheckTileEntityJ(CompoundTag const &expected, CompoundTag const &actual);
struct CheckEntityJOptions {
  enum : uint32_t {
    ignoreUUID = 1 << 0,
    ignorePos = 1 << 1,
  };

  uint32_t const fRawValue = 0;
  explicit CheckEntityJOptions(uint32_t rawValue) : fRawValue(rawValue) {}
  CheckEntityJOptions() : fRawValue(0) {}
};

static void CheckEntityJ(std::u8string const &id, CompoundTag const &entityE, CompoundTag const &entityA, CheckEntityJOptions options = {});

static void CheckBlockJWithIgnore(mcfile::je::Block const &e, mcfile::je::Block const &a, std::initializer_list<std::u8string> ignore) {
  CHECK(e.fName == a.fName);
  map<u8string, optional<u8string>> op;
  for (u8string const &p : ignore) {
    op[p] = nullopt;
  }
  auto blockE = e.applying(op);
  auto blockA = a.applying(op);
  CHECK(blockA->fId == blockE->fId);
  CHECK(blockA->fData == blockE->fData);
}

static void CheckBlockJ(shared_ptr<mcfile::je::Block const> const &blockE, shared_ptr<mcfile::je::Block const> const &blockA, Dimension dim, int x, int y, int z) {
  unordered_map<u8string_view, u8string> fallbackJtoB;
  fallbackJtoB[u8"minecraft:petrified_oak_slab"] = u8"minecraft:oak_slab"; // does not exist in BE. should be replaced to oak_slab when java -> bedrock.
  fallbackJtoB[u8"minecraft:cave_air"] = u8"minecraft:air";
  fallbackJtoB[u8"minecraft:void_air"] = u8"minecraft:air";
  fallbackJtoB[u8"minecraft:test_block"] = u8"minecraft:air";
  fallbackJtoB[u8"minecraft:test_instance_block"] = u8"minecraft:air";

  unordered_map<u8string_view, u8string> fallbackBtoJ;
  fallbackBtoJ[u8"minecraft:frame"] = u8"minecraft:air";      // frame should be converted as an entity.
  fallbackBtoJ[u8"minecraft:glow_frame"] = u8"minecraft:air"; // frame should be converted as an entity.

  if (blockA && blockE) {
    auto foundJtoB = fallbackJtoB.find(blockE->fName);
    if (foundJtoB == fallbackJtoB.end()) {
      auto foundBtoJ = fallbackBtoJ.find(blockA->fName);
      if (foundBtoJ == fallbackBtoJ.end()) {
        if (blockE->fName == u8"minecraft:red_mushroom_block" || blockE->fName == u8"minecraft:brown_mushroom_block") {
          CHECK(blockE->fName == blockA->fName);
        } else if (blockE->fName == u8"minecraft:scaffolding") {
          CheckBlockJWithIgnore(*blockE, *blockA, {u8"distance"});
        } else if (blockE->fName == u8"minecraft:repeater") {
          CheckBlockJWithIgnore(*blockE, *blockA, {u8"locked"});
        } else if (blockE->fName == u8"minecraft:note_block" || blockE->fName.ends_with(u8"_trapdoor") || blockE->fName.ends_with(u8"_fence_gate") || blockE->fName == u8"minecraft:lectern" || blockE->fName.ends_with(u8"_door") || blockE->fName == u8"minecraft:lightning_rod") {
          CheckBlockJWithIgnore(*blockE, *blockA, {u8"powered"});
        } else if (blockE->fName.ends_with(u8"_button")) {
          CheckBlockJWithIgnore(*blockE, *blockA, {u8"facing", u8"powered"});
        } else if (blockE->fName == u8"minecraft:fire") {
          CheckBlockJWithIgnore(*blockE, *blockA, {u8"east", u8"north", u8"south", u8"west", u8"up"});
        } else if (blockE->fName == u8"minecraft:weeping_vines_plant") {
          if (blockA->toString() == u8"minecraft:weeping_vines[age=25]") {
            // JE: weeping_vines -> tip
            //     weeping_vines_plant -> body
            // BE: weeping_vines -> tip / body

            // "weeping_vines_plant without tip" sometimes seen in vanilla JE. tobe::Converter recognize this as a tip of vines
            CHECK(true);
          } else {
            CHECK(blockA->toString() == blockE->toString());
          }
        } else if (blockE->fName == u8"minecraft:twisting_vines_plant") {
          if (blockA->toString() == u8"minecraft:twisting_vines[age=25]") {
            CHECK(true);
          } else {
            CHECK(blockA->toString() == blockE->toString());
          }
        } else if (blockE->fName == u8"minecraft:vine") {
          CheckBlockJWithIgnore(*blockE, *blockA, {u8"up"});
        } else if (blockE->fName == u8"minecraft:mangrove_propagule") {
          CheckBlockJWithIgnore(*blockE, *blockA, {u8"stage"});
        } else if (blockE->fName == u8"minecraft:lever") {
          CheckBlockJWithIgnore(*blockE, *blockA, {u8"facing"});
        } else if (blockE->fName == u8"minecraft:sculk_sensor" || blockE->fName == u8"minecraft:calibrated_sculk_sensor") {
          CheckBlockJWithIgnore(*blockE, *blockA, {u8"power"});
        } else if (blockE->fName == u8"minecraft:piglin_wall_head" || blockE->fName == u8"minecraft:skeleton_skull" || blockE->fName == u8"minecraft:wither_skeleton_skull" || blockE->fName == u8"minecraft:zombie_head" || blockE->fName == u8"minecraft:creeper_head" || blockE->fName == u8"minecraft:dragon_head" || blockE->fName == u8"minecraft:player_head" || blockE->fName == u8"minecraft:skeleton_wall_skull" || blockE->fName == u8"minecraft:piglin_head" || blockE->fName == u8"minecraft:wither_skeleton_wall_skull" || blockE->fName == u8"minecraft:player_wall_head" || blockE->fName == u8"minecraft:zombie_wall_head" || blockE->fName == u8"minecraft:creeper_wall_head" || blockE->fName == u8"minecraft:dragon_wall_head") {
          CheckBlockJWithIgnore(*blockE, *blockA, {u8"powered"});
        } else {
          if (blockA->fId != blockE->fId || blockA->fData != blockE->fData) {
            lock_guard<mutex> lock(sMutCerr);
            cerr << u8"[" << x << u8", " << y << u8", " << z << "] " << JavaStringFromDimension(dim) << endl;
            cerr << "expected=" << blockE->toString() << endl;
            cerr << "actual  =" << blockA->toString() << endl;
          }
          CHECK(blockA->fId == blockE->fId);
          CHECK(blockA->fData == blockE->fData);
        }
      } else {
        CHECK(foundBtoJ->second == blockE->fName);
      }
    } else {
      CHECK(blockA->fName == foundJtoB->second);
    }
  } else if (blockA) {
    CHECK(blockA->fName == u8"minecraft:air");
  } else if (blockE) {
    CHECK(blockE->fName == u8"minecraft:air");
  }
}

static void Erase(shared_ptr<CompoundTag> t, u8string const &path) {
  auto keys = mcfile::String::Split(path, u8'/');
  if (keys.size() == 1) {
    t->erase(keys[0]);
    return;
  }
  auto index = strings::ToI32(keys[1]);
  if (keys[1] == u8"*") {
    assert(keys.size() >= 3);
    u8string nextPath = keys[2];
    for (int i = 3; i < keys.size(); i++) {
      nextPath += u8"/" + keys[i];
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
    u8string nextPath = keys[1];
    for (int i = 2; i < keys.size(); i++) {
      nextPath += u8"/" + keys[i];
    }
    Erase(t, nextPath);
  }
}

static void DiffCompoundTag(CompoundTag const &e, CompoundTag const &a, std::string hint = "") {
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
    if (!hint.empty()) {
      out << hint << endl;
    }
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

static void CheckJson(props::Json const &e, props::Json const &a) {
  CHECK(e.size() == a.size());
  for (auto const &it : e.items()) {
    auto key = it.key();
    REQUIRE(a.contains(key));
    auto actual = a.at(key);
    auto expected = it.value();
    CHECK(expected == actual);
  }
}

static void CheckText(std::u8string const &e, std::u8string const &a, bool unquote) {
  auto jsonE = props::ParseAsJson(e);
  auto jsonA = props::ParseAsJson(a);
  if (jsonE && jsonA) {
    CheckJson(*jsonE, *jsonA);
  } else if (jsonE) {
    props::Json jA;
    auto v = a;
    if (unquote && v.starts_with(u8'"') && v.ends_with(u8'"')) {
      v = v.substr(1, v.size() - 2);
    }
    props::SetJsonString(jA, u8"text", v);
    auto jAs = props::StringFromJson(jA);
    CHECK(e == jAs);
  } else if (jsonA) {
    props::Json jE;
    auto v = e;
    if (unquote && v.starts_with(u8'"') && v.ends_with(u8'"')) {
      v = v.substr(1, v.size() - 2);
    }
    props::SetJsonString(jE, u8"text", v);
    auto jEs = props::StringFromJson(jE);
    CHECK(jEs == a);
  } else {
    CHECK(e == a);
  }
}

static void CheckTextComponent(shared_ptr<Tag> const &e, shared_ptr<Tag> const &a) {
  static auto getText = [](shared_ptr<Tag> const &tag) -> optional<u8string> {
    if (auto text = tag->asString(); text) {
      auto json = props::ParseAsJson(text->fValue);
      if (json) {
        if (auto found = json->find("text"); found != json->end() && found->is_string()) {
          auto str = found->get<string>();
          return std::u8string((char8_t const *)str.c_str(), str.size());
        } else {
          return std::nullopt;
        }
      } else {
        return text->fValue;
      }
    } else if (auto compound = tag->asCompound(); compound) {
      return compound->string(u8"text");
    } else {
      return nullopt;
    }
  };
  auto textE = getText(e);
  auto textA = getText(a);
  CHECK(textE == textA);
}

static void CheckItemName(CompoundTag const &expected, CompoundTag const &actual, u8string const &key) {
  if (auto e = expected.query(key)->asString(); e) {
    auto a = actual.query(key)->asString();
    CHECK((bool)a);
    if (!a) {
      return;
    }
    bool unquote = key != u8"CustomName";
    CheckText(e->fValue, e->fValue, unquote);
  } else if (auto e = expected.query(key)->asCompound(); e) {
    auto a = actual.query(key)->asCompound();
    CHECK((bool)a);
    if (!a) {
      return;
    }
    DiffCompoundTag(*e, *a);
  }
}

static void RemoveEmpty(CompoundTag &t) {
  using namespace std;
  vector<u8string> remove;
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

static void CheckItemJ(CompoundTag const &itemE, CompoundTag const &itemA) {
  unordered_set<u8string> blacklist = {
      u8"components/minecraft:map_id",
      u8"components/minecraft:block_entity_data",
      u8"components/minecraft:hide_additional_tooltip",           // ominous banner
      u8"components/minecraft:tooltip_display/hidden_components", // ominous banner
  };

  CompoundTagPtr copyE = itemE.copy();
  CompoundTagPtr copyA = itemA.copy();

  auto itemId = itemE.string(u8"id");
  if (itemId) {
    if (*itemId == u8"minecraft:petrified_oak_slab" || *itemId == u8"minecraft:furnace_minecart") {
      blacklist.insert(u8"id");
    } else if (*itemId == u8"minecraft:debug_stick") {
      return;
    } else if (*itemId == u8"minecraft:painting") {
      // tag: {EntityTag: {variant: "minecraft:kebab"}} for java prior to 1.20.5
      // components: {minecraft:entity_data: {id: "minecraft:painting", variant: "minecraft:kebab"}} for java 1.20.5 or later
      blacklist.insert(u8"tag");
      blacklist.insert(u8"components");
    } else if (*itemId == u8"minecraft:suspicious_stew") {
      // Duration may vary between the source of stews in JE such as craft table, trade, chest loot. Duration doesn't recorded in item tag in BE.
      blacklist.insert(u8"tag/effects/*/duration");
      blacklist.insert(u8"components/minecraft:suspicious_stew_effects/*/duration");
    } else if (*itemId == u8"minecraft:written_book") {
      blacklist.insert(u8"components/minecraft:written_book_content/resolved");
    }
  }
  CHECK(itemA.compoundTag(u8"tag") == nullptr);
  if (auto sweepingEdge = itemE.query(u8"components/minecraft:stored_enchantments/levels/minecraft:sweeping_edge"); sweepingEdge && sweepingEdge->asInt()) {
    // sweeping_edge does not exist in BE
    blacklist.insert(u8"components/minecraft:stored_enchantments/levels");
  } else if (auto sweepingEdge = itemE.query(u8"components/minecraft:stored_enchantments/minecraft:sweeping_edge"); sweepingEdge && sweepingEdge->asInt()) {
    // sweeping_edge does not exist in BE
    blacklist.insert(u8"components/minecraft:stored_enchantments");
  }
  auto tippedArrowPotion = itemE.query(u8"components/minecraft:potion_contents/potion");
  if (tippedArrowPotion && tippedArrowPotion->asString() && tippedArrowPotion->asString()->fValue == u8"minecraft:luck") {
    // luck does not exist in BE
    blacklist.insert(u8"components/minecraft:potion_contents/potion");
  }

  auto tileE = itemE.query(u8"components/minecraft:block_entity_data")->asCompound();
  auto tileA = itemA.query(u8"components/minecraft:block_entity_data")->asCompound();
  if (tileE) {
    REQUIRE(tileA);
    CHECK(tileE->type() == tileA->type());
    CheckTileEntityJ(*tileE, *tileA);
  } else if (tileA) {
    CHECK(false);
  }

  static set<u8string> const sJsonKeys = {u8"components/minecraft:custom_name", u8"components/minecraft:item_name"};
  for (u8string const &key : sJsonKeys) {
    blacklist.insert(key);
    CheckItemName(itemE, itemA, key);
  }

  auto itemsE = itemE.query(u8"components/minecraft:container")->asList();
  auto itemsA = itemA.query(u8"components/minecraft:container")->asList();
  if (itemsE) {
    CHECK(itemsA);
    if (itemsA) {
      CHECK(itemsE->size() == itemsA->size());
      for (size_t i = 0; i < itemsE->size(); i++) {
        auto elementE = itemsE->at(i)->asCompound();
        auto elementA = itemsA->at(i)->asCompound();
        auto slotE = elementE->int32(u8"slot");
        auto slotA = elementA->int32(u8"slot");
        REQUIRE(slotE);
        CHECK(*slotE == *slotA);
        REQUIRE(elementE);
        REQUIRE(elementA);
        auto itemE = elementE->compoundTag(u8"item");
        auto itemA = elementA->compoundTag(u8"item");
        CheckItemJ(*itemE, *itemA);
      }
    }
  }
  blacklist.insert(u8"components/minecraft:container");

  for (u8string const &it : blacklist) {
    Erase(copyE, it);
    Erase(copyA, it);
  }

  auto componentsE = copyE->compoundTag(u8"components");
  if (componentsE) {
    RemoveEmpty(*componentsE);
  }
  if (componentsE && componentsE->empty()) {
    copyE->erase(u8"components");
  }
  auto componentsA = copyA->compoundTag(u8"components");
  if (componentsA) {
    RemoveEmpty(*componentsA);
  }
  if (componentsA && componentsA->empty()) {
    copyA->erase(u8"components");
  }

  DiffCompoundTag(*copyE, *copyA);
}

static void CheckSignTextLinesJ(CompoundTag const &e, CompoundTag const &a) {
  auto messagesE = e.listTag(u8"messages");
  if (!messagesE) {
    messagesE = e.listTag(u8"filtered_messages");
  }
  auto messagesA = a.listTag(u8"messages");
  if (!messagesA) {
    messagesA = a.listTag(u8"filtered_messages");
  }
  if (messagesE) {
    CHECK(messagesA);
    CHECK(messagesE->size() == messagesA->size());
    for (size_t i = 0; i < messagesE->size(); i++) {
      auto lineE = messagesE->at(i);
      auto lineA = messagesA->at(i);
      CheckTextComponent(lineE, lineA);
    }
  }
  auto copyE = e.copy();
  auto copyA = a.copy();
  for (auto const &key : {u8"messages", u8"filtered_messages"}) {
    copyE->erase(key);
    copyA->erase(key);
  }
  DiffCompoundTag(*copyE, *copyA);
}

static void CheckTileEntityJ(CompoundTag const &expected, CompoundTag const &actual) {
  auto copyE = expected.copy();
  auto copyA = actual.copy();

  auto x = expected.int32(u8"x");
  auto y = expected.int32(u8"y");
  auto z = expected.int32(u8"z");

  unordered_set<u8string> tagBlacklist = {
      u8"LootTableSeed",           // chest in dungeon etc.
      u8"RecipesUsed",             // furnace, blast_furnace, and smoker
      u8"LastOutput",              // command_block
      u8"author",                  // structure_block
      u8"Book/tag/resolved",       // written_book
      u8"Book/tag/filtered_title", // written_book
      u8"Items",
      u8"Levels",                                                 // beacon. Sometimes reset to 0 in JE
      u8"SpawnPotentials",                                        // mob_spawner, SpawnPotentials sometimes doesn't contained in JE
      u8"placement_priority",                                     // jigsaw
      u8"selection_priority",                                     // jigsaw
      u8"components/minecraft:hide_additional_tooltip",           // banner
      u8"components/minecraft:tooltip_display/hidden_components", // banner
  };
  auto id = expected.string(u8"id", u8"");
  if (id == u8"minecraft:sculk_shrieker") {
    tagBlacklist.insert(u8"listener");
    tagBlacklist.insert(u8"warning_level");
  } else if (id == u8"minecraft:sculk_catalyst") {
    tagBlacklist.insert(u8"cursors");
  } else if (id == u8"minecraft:jukebox") {
    tagBlacklist.insert(u8"IsPlaying");
    tagBlacklist.insert(u8"TickCount");
    tagBlacklist.insert(u8"RecordStartTick");
    tagBlacklist.insert(u8"ticks_since_song_started");
  } else if (id == u8"minecraft:sign") {
    tagBlacklist.insert(u8"Color");
    tagBlacklist.insert(u8"GlowingText");
    tagBlacklist.insert(u8"Text1");
    tagBlacklist.insert(u8"Text2");
    tagBlacklist.insert(u8"Text3");
    tagBlacklist.insert(u8"Text4");
  } else if (id == u8"minecraft:beehive" || id == u8"minecraft:bee_nest") {
    tagBlacklist.insert(u8"FlowerPos");
    tagBlacklist.insert(u8"flower_pos");
  } else if (id == u8"minecraft:banner") {
    // banners do not have CustomName in BE
    tagBlacklist.insert(u8"CustomName");
  } else if (id == u8"minecraft:vault") {
    tagBlacklist.insert(u8"shared_data/connected_players");
  } else if (id == u8"minecraft:smoker" || id == u8"minecraft:blast_furnace" || id == u8"minecraft:furnace") {
    tagBlacklist.insert(u8"cooking_total_time");
  }
  CHECK(actual.compoundTag(u8"tag") == nullptr);
  auto itemsE = expected.listTag(u8"Items");
  auto itemsA = actual.listTag(u8"Items");
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
      CheckItemJ(*itemE->asCompound(), *itemA->asCompound());
    }
  } else if (itemsA) {
    CHECK(false);
  }
  static set<u8string> const sJsonKeys = {u8"components/minecraft:custom_name", u8"components/minecraft:item_name", u8"CustomName"};
  for (u8string const &key : sJsonKeys) {
    tagBlacklist.insert(key);
    CheckItemName(expected, actual, key);
  }
  for (auto const &key : {u8"back_text", u8"front_text"}) {
    auto backTextE = expected.compoundTag(key);
    auto backTextA = actual.compoundTag(key);
    if (backTextE) {
      CHECK(backTextA);
      CheckSignTextLinesJ(*backTextE, *backTextA);
    }
    tagBlacklist.insert(key);
  }
  auto beesE = FallbackPtr<ListTag>(expected, {u8"bees", u8"Bees"});
  auto beesA = FallbackPtr<ListTag>(actual, {u8"bees", u8"Bees"});
  if (beesE) {
    CHECK(beesA);
    CHECK(beesE->size() == beesA->size());
    for (size_t i = 0; i < beesE->size(); i++) {
      auto placeholderE = beesE->at(i)->asCompound();
      auto placeholderA = beesA->at(i)->asCompound();
      REQUIRE(placeholderE);
      CHECK(placeholderA);
      auto beeE = FallbackPtr<CompoundTag>(*placeholderE, {u8"entity_data", u8"EntityData"});
      auto beeA = FallbackPtr<CompoundTag>(*placeholderA, {u8"entity_data", u8"EntityData"});
      REQUIRE(beeE);
      CHECK(beeA);
      auto beeCopyE = beeE->copy();
      auto beeCopyA = beeA->copy();
      for (auto const &ignore : {u8"attributes"}) {
        beeCopyE->erase(ignore);
        beeCopyA->erase(ignore);
      }
      CheckEntityJ(u8"minecraft:bee", *beeCopyE, *beeCopyA, CheckEntityJOptions(CheckEntityJOptions::ignorePos | CheckEntityJOptions::ignoreUUID));
    }
    tagBlacklist.insert(u8"Bees");
    tagBlacklist.insert(u8"bees");
  }
  for (auto const &itemKey : {u8"Book"}) {
    auto itemE = expected.compoundTag(itemKey);
    auto itemA = actual.compoundTag(itemKey);
    if (itemE) {
      CHECK(itemA);
      if (itemA) {
        CheckItemJ(*itemE, *itemA);
      }
    } else {
      CHECK(!itemA);
    }
    tagBlacklist.insert(itemKey);
  }
  for (u8string const &b : tagBlacklist) {
    Erase(copyE, b);
    Erase(copyA, b);
  }
  auto componentsE = copyE->compoundTag(u8"components");
  if (componentsE) {
    RemoveEmpty(*componentsE);
  }
  if (componentsE && componentsE->empty()) {
    copyE->erase(u8"components");
  }
  auto componentsA = copyA->compoundTag(u8"components");
  if (componentsA) {
    RemoveEmpty(*componentsA);
  }
  if (componentsA && componentsA->empty()) {
    copyA->erase(u8"components");
  }
  DiffCompoundTag(*copyE, *copyA);
}

static void CheckBrainJ(CompoundTag const &brainE, CompoundTag const &brainA) {
  static unordered_set<u8string> const blacklist = {
      // allay, etc
      u8"memories/minecraft:gaze_cooldown_ticks",
      // goat, frog
      u8"memories/minecraft:long_jump_cooling_down",
      // goat
      u8"memories/minecraft:ram_cooldown_ticks memories",
      u8"memories/minecraft:ram_cooldown_ticks",
      // piglin
      /*
        "minecraft:hunted_recently": {
          "ttl": 962, // long
          "value": 1 // byte
        }
       */
      u8"memories/minecraft:hunted_recently",
      // allay
      u8"memories/minecraft:liked_player",
      // villager
      u8"memories/minecraft:job_site",
      u8"memories/minecraft:last_worked_at_poi",
      u8"memories/minecraft:meeting_point",
      u8"memories/minecraft:golem_detected_recently",
      u8"memories/minecraft:potential_job_site",
      u8"memories/minecraft:home",
      u8"memories/minecraft:last_slept",
      u8"memories/minecraft:last_woken",
      // piglin etc.
      u8"memories/minecraft:angry_at", // TODO:
      // sniffer
      u8"memories/minecraft:sniff_cooldown",
      u8"memories/minecraft:sniffer_explored_positions",
      // armadillo
      u8"memories/minecraft:danger_detected_recently",
      // happy_ghast
      u8"memories/minecraft:is_tempted",
      // copper_golem
      u8"memories/minecraft:visited_block_positions",
      u8"memories/minecraft:unreachable_transport_block_positions"};
  auto copyE = brainE.copy();
  auto copyA = brainA.copy();

  auto likedPlayerA = copyA->query(u8"memories/minecraft:liked_player");
  auto likedPlayerE = copyE->query(u8"memories/minecraft:liked_player");
  CHECK((likedPlayerA->type() == Tag::Type::End) == (likedPlayerE->type() == Tag::Type::End));

  for (u8string const &b : blacklist) {
    Erase(copyE, b);
    Erase(copyA, b);
  }

  DiffCompoundTag(*copyE, *copyA);
}

static void CheckRecipeJ(CompoundTag const &e, CompoundTag const &a) {
  auto copyE = e.copy();
  auto copyA = a.copy();
  for (u8string key : {u8"buy", u8"buyB", u8"sell"}) {
    auto itemE = e.compoundTag(key);
    auto itemA = a.compoundTag(key);
    copyE->erase(key);
    copyA->erase(key);
    if (itemE) {
      CHECK(itemA);
      if (itemA) {
        CheckItemJ(*itemE, *itemA);
      }
    } else {
      CHECK(!itemA);
    }
  }
  DiffCompoundTag(*copyE, *copyA);
}

static void CheckOffersJ(CompoundTag const &e, CompoundTag const &a) {
  auto copyE = e.copy();
  auto copyA = a.copy();
  auto recipesE = e.listTag(u8"Recipes");
  auto recipesA = a.listTag(u8"Recipes");
  CHECK(recipesE->size() == recipesA->size());
  for (size_t i = 0; i < recipesE->size(); i++) {
    auto recipeE = recipesE->at(i);
    auto recipeA = recipesA->at(i);
    CHECK(recipeE);
    CHECK(recipeA);
    if (recipeE->asCompound() && recipeA->asCompound()) {
      CheckRecipeJ(*recipeE->asCompound(), *recipeA->asCompound());
    }
  }
  copyE->erase(u8"Recipes");
  copyA->erase(u8"Recipes");
  DiffCompoundTag(*copyE, *copyA);
}

static void CheckContainerItemsJ(ListTagPtr const &expected, ListTagPtr const &actual) {
  if (expected) {
    REQUIRE(actual);
    REQUIRE(expected->size() == actual->size());
    static std::unordered_set<std::u8string> ignore = {
        u8"minecraft:test_block",
        u8"minecraft:test_instance_block",
    };
    for (int i = 0; i < expected->size(); i++) {
      auto e = expected->at(i);
      auto a = actual->at(i);
      auto compoundE = e->asCompound();
      auto compoundA = a->asCompound();
      REQUIRE(compoundE);
      CHECK(compoundA);
      if (!compoundA) {
        continue;
      }
      if (auto idE = compoundE->string(u8"id"); ignore.count(*idE) > 0) {
        CHECK(compoundA->string(u8"id") == u8"minecraft:air");
        continue;
      }
      CheckItemJ(*compoundE, *compoundA);
    }
  } else if (actual) {
    CHECK(false);
  }
}

static void CheckRotationJ(ListTag const &e, ListTag const &a) {
  REQUIRE(e.size() == 2);
  REQUIRE(a.size() == 2);
  for (size_t i = 0; i < 2; i++) {
    auto ev = e.at(i)->asFloat();
    auto av = a.at(i)->asFloat();
    REQUIRE(ev);
    REQUIRE(av);
    float diff = fabsf(ev->fValue - av->fValue);
    CHECK(diff < 0.01f);
  }
}

static void CheckUuidJ(CompoundTag const &e, CompoundTag const &a, std::u8string const &key) {
  auto idE = e.intArrayTag(key);
  auto idA = a.intArrayTag(key);
  if (idE) {
    CHECK(idA);
  } else {
    CHECK(!idA);
  }
}

static void CheckEntityJ(std::u8string const &id, CompoundTag const &entityE, CompoundTag const &entityA, CheckEntityJOptions options) {
  auto copyE = entityE.copy();
  auto copyA = entityA.copy();

  auto posE = props::GetPos3d(*copyE, u8"Pos");
  auto posA = props::GetPos3d(*copyA, u8"Pos");

  if ((options.fRawValue & CheckEntityJOptions::ignoreUUID) == 0) {
    CheckUuidJ(*copyE, *copyA, u8"UUID");
  }

  if ((options.fRawValue & CheckEntityJOptions::ignorePos) == 0) {
    CHECK(posE);
    CHECK(posA);
    CHECK(fabs(posE->fX - posA->fX) <= 0.001);
    if (id == u8"minecraft:armor_stand") {
      // y is aligned 0.5 block in BE
    } else if (id == u8"minecraft:chest_minecart") {
      // y of chest_minecart in abandoned_mineshaft has usually conversion failure because OnGround=false but not on rail
    } else if (id == u8"minecraft:boat" || id.ends_with(u8"_boat") || id.ends_with(u8"_raft") || id.ends_with(u8"_chest_boat") || id.ends_with(u8"_chest_raft")) {
      // y of boat usually different between JE and BE
    } else {
      CHECK(fabs(posE->fY - posA->fY) <= 0.001);
    }
    CHECK(fabs(posE->fZ - posA->fZ) <= 0.001);
  }

  unordered_set<u8string> blacklist = {
      u8"UUID",
      u8"Pos",
      u8"attributes",
      u8"Attributes",
      u8"Motion",
      u8"LeftHanded", // left handed skeleton does not exist in BE
      u8"ForcedAge",
      u8"Owner",
      u8"HurtByTimestamp",
      u8"NoAI",
      u8"Fire",
      u8"Offers/Recipes/*/sell/tag/Effects/*/EffectDuration", // EffectDuration of suspicious_stew is random
      u8"Offers/Recipes/*/sell/tag/map",
      u8"EatingHaystack",
      u8"PersistenceRequired", // This is default false for animals in JE. BE reqires Persistent = true even for animals. So this property cannot completely recover in round-trip conversion.
      u8"listener",
      u8"HandDropChances", // TODO: Is there any way identifying hand items was picked-up thing or not?
      u8"home_pos",
      u8"home_radius",
  };
  if (id == u8"minecraft:armor_stand") {
    blacklist.insert(u8"Pose");
    blacklist.insert(u8"Health"); // Default health differs. B = 6, J = 20
  } else if (id == u8"minecraft:slime" || id == u8"minecraft:magma_cube") {
    blacklist.insert(u8"wasOnGround"); // wasOnGround does not exist in BE
  } else if (id == u8"minecraft:bee") {
    blacklist.insert(u8"TicksSincePollination"); // TicksSincePollination does not exist in BE
    blacklist.insert(u8"FlowerPos");
    blacklist.insert(u8"flower_pos");
  } else if (id == u8"minecraft:llama") {
    blacklist.insert(u8"Temper"); // Temper does not exist in BE
  } else if (id == u8"minecraft:phantom") {
    blacklist.insert(u8"AX");
    blacklist.insert(u8"AY");
    blacklist.insert(u8"AZ");
    blacklist.insert(u8"anchor_pos");
  } else if (id == u8"minecraft:turtle") {
    blacklist.insert(u8"TravelPosX");
    blacklist.insert(u8"TravelPosY");
    blacklist.insert(u8"TravelPosZ");
  } else if (id == u8"minecraft:villager") {
    blacklist.insert(u8"Gossips");
    blacklist.insert(u8"LastGossipDecay");
    blacklist.insert(u8"LastRestock");
    blacklist.insert(u8"RestocksToday");
  } else if (id == u8"minecraft:shulker") {
    blacklist.insert(u8"AttachFace"); // not exists in BE
    blacklist.insert(u8"Peek");       // not exists in BE
  } else if (id == u8"minecraft:iron_golem") {
    blacklist.insert(u8"AngerTime");
    blacklist.insert(u8"AngryAt");
  } else if (id == u8"minecraft:zombie_villager") {
    blacklist.insert(u8"PersistenceRequired"); // BE requires "Persistent" = true to keep them alive, but JE doesn't
    blacklist.insert(u8"InWaterTime");
    blacklist.insert(u8"IsBaby"); // TODO:
  } else if (id == u8"minecraft:piglin") {
    blacklist.insert(u8"TimeInOverworld");
    blacklist.insert(u8"IsBaby");
    CHECK(copyE->boolean(u8"IsBaby", false) == copyA->boolean(u8"IsBaby", false));
  } else if (id == u8"minecraft:piglin_brute") {
    blacklist.insert(u8"TimeInOverworld");
  } else if (id == u8"minecraft:hoglin") {
    blacklist.insert(u8"TimeInOverworld");
    blacklist.insert(u8"ticks_since_last_hurt_by_mob");
  } else if (id == u8"minecraft:minecart" || id == u8"minecraft:tnt_minecart" || id == u8"minecraft:hopper_minecart") {
    blacklist.insert(u8"FlippedRotation");
    blacklist.insert(u8"HasTicked");
  } else if (id == u8"minecraft:chest_minecart") {
    blacklist.insert(u8"LootTableSeed");
    blacklist.insert(u8"FlippedRotation");
    blacklist.insert(u8"HasTicked");
  } else if (id == u8"minecraft:zombie") {
    blacklist.insert(u8"InWaterTime");
  } else if (id == u8"minecraft:arrow") {
    blacklist.insert(u8"HasBeenShot");
    blacklist.insert(u8"LeftOwner");
    blacklist.insert(u8"PierceLevel");
    blacklist.insert(u8"ShotFromCrossbow");
    blacklist.insert(u8"crit");
    blacklist.insert(u8"damage");
    blacklist.insert(u8"inGround");
    blacklist.insert(u8"life");
    blacklist.insert(u8"pickup");
    blacklist.insert(u8"shake");
    blacklist.insert(u8"inBlockState");
    blacklist.insert(u8"SoundEvent");
    blacklist.insert(u8"weapon");
  } else if (id == u8"minecraft:falling_block") {
    blacklist.insert(u8"DropItem");
    blacklist.insert(u8"FallDistance");
    blacklist.insert(u8"FallHurtAmount");
    blacklist.insert(u8"FallHurtMax");
    blacklist.insert(u8"HurtEntities");
    blacklist.insert(u8"Time"); // JE: int, BE: byte
  } else if (id == u8"minecraft:item") {
    blacklist.insert(u8"PickupDelay");
  } else if (id == u8"minecraft:trader_llama") {
    blacklist.insert(u8"DespawnDelay"); // "TimeStamp" does not exist in BE.
  } else if (id == u8"minecraft:warden") {
    blacklist.insert(u8"anger");
  } else if (id == u8"minecraft:zombified_piglin") {
    blacklist.insert(u8"AngerTime");
    blacklist.insert(u8"AngryAt");
    auto angryAtA = copyA->intArrayTag(u8"AngryAt");
    auto angryAtE = copyE->intArrayTag(u8"AngryAt");
    CHECK((angryAtE == nullptr) == (angryAtA == nullptr));
  } else if (id == u8"minecraft:camel") {
    blacklist.insert(u8"LastPoseTick");
  } else if (id == u8"minecraft:wandering_trader") {
    blacklist.insert(u8"WanderTarget");
  } else if (id == u8"minecraft:experience_orb") {
    // NOTE: "Count" does not exist in BE.
    blacklist.insert(u8"Count");
    blacklist.insert(u8"Value");
    auto totalE = copyE->int32(u8"Count", 0) * (int)copyE->int16(u8"Value", 0);
    auto totalA = copyA->int32(u8"Count", 0) * (int)copyA->int16(u8"Value", 0);
    CHECK(totalE == totalA);
  } else if (id == u8"minecraft:parrot") {
    blacklist.insert(u8"NoGravity");
  } else if (id == u8"minecraft:ender_dragon") {
    blacklist.insert(u8"DragonPhase");
  } else if (id == u8"player") {
    blacklist.insert(u8"current_impulse_context_reset_grace_time");
  } else if (id == u8"minecraft:squid" || id == u8"minecraft:glow_squid") {
    // BE doesn't have Age tag
    blacklist.insert(u8"Age");
  } else if (id == u8"minecraft:dolphin") {
    blacklist.insert(u8"GotFish");
    blacklist.insert(u8"Moistness");
    blacklist.insert(u8"TreasurePosX");
    blacklist.insert(u8"TreasurePosY");
    blacklist.insert(u8"TreasurePosZ");
  }

  CheckItemName(entityE, entityA, u8"CustomName");
  blacklist.insert(u8"CustomName");

  CheckUuidJ(entityE, entityA, u8"last_hurt_by_mob");
  blacklist.insert(u8"last_hurt_by_mob");

  for (u8string const &it : blacklist) {
    Erase(copyE, it);
    Erase(copyA, it);
  }

  auto itemE = entityE.compoundTag(u8"Item");
  auto itemA = entityA.compoundTag(u8"Item");
  copyE->erase(u8"Item");
  copyA->erase(u8"Item");
  if (itemE) {
    REQUIRE(itemA);
    CheckItemJ(*itemE, *itemA);
  }

  auto passengersE = entityE.listTag(u8"Passengers");
  auto passengersA = entityA.listTag(u8"Passengers");
  copyE->erase(u8"Passengers");
  copyA->erase(u8"Passengers");
  if (passengersE) {
    REQUIRE(passengersA);
    CHECK(passengersE->size() == passengersA->size());
    for (int i = 0; i < passengersE->size(); i++) {
      auto passengerE = passengersE->at(i);
      auto passengerA = passengersA->at(i);
      REQUIRE(passengerE->type() == Tag::Type::Compound);
      CHECK(passengerE->type() == passengerA->type());
      auto id = passengerE->asCompound()->string(u8"id");
      REQUIRE(id);
      CheckEntityJ(*id, *passengerE->asCompound(), *passengerA->asCompound());
    }
  } else if (passengersA) {
    CHECK(false);
  }

  auto brainE = entityE.compoundTag(u8"Brain");
  auto brainA = entityA.compoundTag(u8"Brain");
  copyE->erase(u8"Brain");
  copyA->erase(u8"Brain");
  if (brainE) {
    REQUIRE(brainA);
    CheckBrainJ(*brainE, *brainA);
  } else if (brainA) {
    CHECK(false);
  }

  for (auto const &containerKey : {u8"Inventory", u8"EnderItems", u8"Items"}) {
    auto contentsE = entityE.listTag(containerKey);
    auto contentsA = entityA.listTag(containerKey);
    copyE->erase(containerKey);
    copyA->erase(containerKey);
    CheckContainerItemsJ(contentsE, contentsA);
  }
  REQUIRE(!entityE.listTag(u8"ArmorItems"));
  CHECK(!entityA.listTag(u8"ArmorItems"));

  auto equipmentE = entityE.compoundTag(u8"equipment");
  auto equipmentA = entityA.compoundTag(u8"equipment");
  copyE->erase(u8"equipment");
  copyA->erase(u8"equipment");
  if (equipmentE) {
    if (id == u8"minecraft:horse") {
      equipmentE->erase(u8"chest");
    }
    if (equipmentE->empty()) {
      equipmentE = nullptr;
    }
  }
  if (equipmentE) {
    CHECK(equipmentA);
    if (equipmentA) {
      CHECK(equipmentA->size() == equipmentE->size());
      for (auto it : *equipmentE) {
        auto key = it.first;
        auto valueE = it.second->asCompound();
        REQUIRE(valueE);
        auto foundA = equipmentA->find(key);
        CHECK(foundA != equipmentA->end());
        if (foundA != equipmentA->end()) {
          auto valueA = foundA->second->asCompound();
          CHECK(valueA);
          if (valueA) {
            CheckItemJ(*valueE, *valueA);
          }
        }
      }
    }
  } else {
    CHECK(!equipmentA);
  }

  auto dropChancesE = entityE.compoundTag(u8"drop_chances");
  auto dropChancesA = entityA.compoundTag(u8"drop_chances");
  copyE->erase(u8"drop_chances");
  copyA->erase(u8"drop_chances");
  if (dropChancesE && id == u8"minecraft:horse") {
    dropChancesE->erase(u8"chest");
    if (dropChancesE->empty()) {
      dropChancesE = nullptr;
    }
  }
  if (dropChancesE) {
    CHECK(dropChancesA);
    if (dropChancesA) {
      DiffCompoundTag(*dropChancesE, *dropChancesA);
    }
  } else {
    CHECK(!dropChancesA);
  }

  auto offersE = entityE.compoundTag(u8"Offers");
  auto offersA = entityA.compoundTag(u8"Offers");
  copyE->erase(u8"Offers");
  copyA->erase(u8"Offers");
  if (offersE) {
    CHECK(offersA);
    if (offersA) {
      CheckOffersJ(*offersE, *offersA);
    }
  }

  auto throwerE = entityE.intArrayTag(u8"Thrower");
  auto throwerA = entityA.intArrayTag(u8"Thrower");
  copyE->erase(u8"Thrower");
  copyA->erase(u8"Thrower");
  CHECK((bool)throwerE == (bool)throwerA);

  auto rotationE = entityE.listTag(u8"Rotation");
  auto rotationA = entityA.listTag(u8"Rotation");
  copyE->erase(u8"Rotation");
  copyA->erase(u8"Rotation");
  if (rotationE) {
    CHECK(rotationA);
    CheckRotationJ(*rotationE, *rotationA);
  } else {
    CHECK(!rotationA);
  }

  auto leashE = entityE.query(u8"leash");
  auto leashA = entityA.query(u8"leash");
  if (leashE->type() == Tag::Type::Compound) {
    auto le = leashE->asCompound();
    REQUIRE(le);
    REQUIRE(le->intArrayTag(u8"UUID"));
    CHECK(leashA->type() == Tag::Type::Compound);
    if (auto la = leashA->asCompound(); la) {
      CHECK(la->intArrayTag(u8"UUID"));
    }
  } else if (leashE->type() == Tag::Type::IntArray) {
    auto le = leashE->asIntArray();
    REQUIRE(le);
    REQUIRE(le->fValue.size() == 3);
    CHECK(leashA->type() == Tag::Type::IntArray);
    if (auto la = leashA->asIntArray(); la) {
      REQUIRE(la->fValue.size() == 3);
      for (size_t i = 0; i < 3; i++) {
        CHECK(le->fValue[i] == la->fValue[i]);
      }
    }
  } else {
    REQUIRE(leashE->type() == Tag::Type::End);
    CHECK(leashA->type() == Tag::Type::End);
  }
  copyE->erase(u8"leash");
  copyA->erase(u8"leash");

  DiffCompoundTag(*copyE, *copyA);
}

static shared_ptr<CompoundTag> FindNearestEntity(Pos3d pos, Rotation rot, std::u8string const &id, vector<shared_ptr<CompoundTag>> const &entities) {
  shared_ptr<CompoundTag> ret = nullptr;
  double minDistance = numeric_limits<double>::max();
  double minRotDifference = numeric_limits<double>::max();
  for (auto const &entity : entities) {
    if (entity->string(u8"id") != id) {
      continue;
    }
    auto p = props::GetPos3d(*entity, u8"Pos");
    if (!p) {
      continue;
    }
    auto r = props::GetRotation(*entity, u8"Rotation");
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

static void CheckTickingBlockJ(mcfile::je::TickingBlock e, mcfile::je::TickingBlock a) {
  CHECK(e.fI == a.fI);
  // CHECK(e.fP == a.fP); "P" does not exist in BE
  CHECK(e.fT == a.fT);
  CHECK(e.fX == a.fX);
  CHECK(e.fY == a.fY);
  CHECK(e.fZ == a.fZ);
}

static void CheckHeightmapsJ(CompoundTag const &expected, CompoundTag const &actual) {
  static set<u8string> const sTypes = {u8"MOTION_BLOCKING",
                                       u8"MOTION_BLOCKING_NO_LEAVES",
                                       u8"OCEAN_FLOOR",
                                       u8"WORLD_SURFACE"};
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

static void CheckSectionLightJ(Pos3i const &origin, std::vector<uint8_t> &e, std::vector<uint8_t> &a, Dimension dim, u8string const &kind) {
  auto dataE = mcfile::Data4b3dView::Make(origin, 16, 16, 16, &e);
  auto dataA = mcfile::Data4b3dView::Make(origin, 16, 16, 16, &a);
  REQUIRE(dataE);
  REQUIRE(dataA);
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
      lock_guard<mutex> lock(sMutCerr);
      cerr << "-------------------------------------------------------------------------------------------------------------------" << endl;
      cerr << kind << " " << JavaStringFromDimension(dim) << endl;
      fprintf(stderr, "%4d", origin.fY + y);
      for (int x = 0; x < 16; x++) {
        fprintf(stderr, "%6d ", x + origin.fX);
      }
      cerr << endl;
      for (int z = 0; z < 16; z++) {
        fprintf(stderr, "%3d:", z + origin.fZ);
        for (int x = 0; x < 16; x++) {
          int vE = dataE->getUnchecked(origin + Pos3i{x, y, z});
          int vA = dataA->getUnchecked(origin + Pos3i{x, y, z});
          string bra = "[";
          string ket = "]";
          if (vE == vA) {
            bra = " ";
            ket = " ";
          }
          fprintf(stderr, "%s%2d,%2d%s", bra.c_str(), vE, vA, ket.c_str());
        }
        cerr << endl;
      }
      cerr << endl;
    }
    CHECK(ok);
  }
}

static void CheckChunkLightJ(mcfile::je::Chunk const &chunkE, mcfile::je::Chunk const &chunkA, Dimension dim) {
  REQUIRE(chunkA.fSections.size() == chunkE.fSections.size());
  for (int i = 0; i < chunkE.fSections.size(); i++) {
    auto const &sectionE = chunkE.fSections[i];
    auto const &sectionA = chunkA.fSections[i];
    Pos3i origin{chunkE.fChunkX * 16, sectionE->y() * 16, chunkE.fChunkZ * 16};
    REQUIRE(sectionE);
    REQUIRE(sectionA);
    REQUIRE(sectionE->y() == sectionA->y());
    if (!sectionE->fSkyLight.empty() && std::any_of(sectionE->fSkyLight.begin(), sectionE->fSkyLight.end(), [](uint8_t v) { return v != 0; })) {
      CHECK(sectionE->fSkyLight.size() == sectionA->fSkyLight.size());
      CheckSectionLightJ(origin, sectionE->fSkyLight, sectionA->fSkyLight, dim, u8"sky");
    }
    if (!sectionE->fBlockLight.empty() && std::any_of(sectionE->fBlockLight.begin(), sectionE->fBlockLight.end(), [](uint8_t v) { return v != 0; })) {
      CHECK(sectionE->fBlockLight.size() == sectionA->fBlockLight.size());
      CheckSectionLightJ(origin, sectionE->fBlockLight, sectionA->fBlockLight, dim, u8"block");
    }
  }
}

static void CheckChunkJ(mcfile::je::Region const &regionE, mcfile::je::Region const &regionA, int cx, int cz, Dimension dim) {
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

  CHECK(chunkE->getDataVersion() == chunkA->getDataVersion());

  for (int y = chunkE->minBlockY(); y <= chunkE->maxBlockY(); y++) {
    for (int z = chunkE->minBlockZ() + 1; z < chunkE->maxBlockZ(); z++) {
      for (int x = chunkE->minBlockX() + 1; x < chunkE->maxBlockX(); x++) {
        auto blockA = chunkA->blockAt(x, y, z);
        auto blockE = chunkE->blockAt(x, y, z);
        CheckBlockJ(blockE, blockA, dim, x, y, z);
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
        if (e == mcfile::biomes::unknown) {
          continue;
        }
        CHECK(a == e);
      }
    }
  }

  for (auto const &it : chunkE->fTileEntities) {
    Pos3i pos = it.first;
    shared_ptr<CompoundTag> const &tileE = it.second;
    static unordered_set<u8string> blacklist({
        u8"minecraft:sculk_sensor",
        u8"minecraft:calibrated_sculk_sensor",
        u8"minecraft:test_block",
        u8"minecraft:test_instance_block",
    });
    if (blacklist.find(tileE->string(u8"id", u8"")) != blacklist.end()) {
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
    CheckTileEntityJ(*tileE, *tileA);
  }

  static set<u8string> const sEntityBlacklist = {
      u8"minecraft:text_display",
      u8"minecraft:item_display",
      u8"minecraft:block_display",
  };
  for (shared_ptr<CompoundTag> const &entityE : chunkE->fEntities) {
    Pos3d posE = *props::GetPos3d(*entityE, u8"Pos");
    Rotation rotE = *props::GetRotation(*entityE, u8"Rotation");
    auto id = entityE->string(u8"id");
    REQUIRE(id);
    if (sEntityBlacklist.find(*id) != sEntityBlacklist.end()) {
      continue;
    }
    shared_ptr<CompoundTag> entityA = FindNearestEntity(posE, rotE, *id, chunkA->fEntities);
    if (entityA) {
      CheckEntityJ(*id, *entityE, *entityA);
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
    CheckTickingBlockJ(tbE, *tbA);
  }

  CHECK(chunkE->fTileTicks.size() == chunkA->fTileTicks.size());
  for (size_t i = 0; i < chunkE->fTileTicks.size(); i++) {
    mcfile::je::TickingBlock tbE = chunkE->fTileTicks[i];
    auto tbA = FindNearestTickingBlock(tbE.fX, tbE.fY, tbE.fZ, chunkA->fTileTicks);
    CHECK(tbA);
    CheckTickingBlockJ(tbE, *tbA);
  }

  auto heightMapsE = chunkE->fRoot->compoundTag(u8"Heightmaps");
  auto heightMapsA = chunkA->fRoot->compoundTag(u8"Heightmaps");
  REQUIRE(heightMapsE);
  REQUIRE(heightMapsA);
  CheckHeightmapsJ(*heightMapsE, *heightMapsA);

  if (0 <= chunkE->fChunkX && chunkE->fChunkX <= J2B2J_MAX_CHUNK_X && 0 <= chunkE->fChunkZ && chunkE->fChunkZ <= 1) {
    // SkyLight and BlockLight sometimes have wrong value, so check light only for limited chunks
    CheckChunkLightJ(*chunkE, *chunkA, dim);
  }
}

static std::shared_ptr<CompoundTag> ReadLevelDatJ(fs::path const &p) {
  auto s = make_shared<mcfile::stream::GzFileInputStream>(p);
  return CompoundTag::Read(s, Encoding::Java);
}

static void CheckLevelDatJ(fs::path const &pathE, fs::path const &pathA) {
  auto e = ReadLevelDatJ(pathE);
  auto a = ReadLevelDatJ(pathA);
  CHECK(e);
  CHECK(a);

  auto dataE = e->compoundTag(u8"Data");
  auto dataA = a->compoundTag(u8"Data");
  CHECK(dataE);
  CHECK(dataA);

  unordered_set<u8string> blacklist = {
      u8"BorderCenterX",
      u8"BorderCenterY",
      u8"BorderCenterZ",
      u8"BorderSafeZone",
      u8"BorderSize",
      u8"BorderSizeLerpTarget",
      u8"BorderSizeLerpTime",
      u8"BorderWarningBlocks",
      u8"BorderWarningTime",
      u8"BorderDamagePerBlock",
      u8"CustomBossEvents",
      u8"DifficultyLocked",
      u8"DragonFight/Dragon",
      u8"LastPlayed", // JE: milli-seconds, BE: seconds
      u8"ScheduledEvents",
      u8"SpawnAngle",
      u8"WanderingTraderId",
      u8"WanderingTraderSpawnChance",
      u8"WanderingTraderSpawnDelay",
      u8"WasModded",
      u8"clearWeatherTime",
      u8"hardcore",
      u8"initialized",
      u8"removed_features",
  };
  unordered_set<u8string> ignoredGameRules = {
      u8"announceAdvancements",
      u8"disableElytraMovementCheck",
      u8"disableRaids",
      u8"forgiveDeadPlayers",
      u8"logAdminCommands",
      u8"maxEntityCramming",
      u8"spectatorsGenerateChunks",
      u8"universalAnger",
      u8"doWardenSpawning",
      u8"blockExplosionDropDecay",
      u8"globalSoundEvents",
      u8"lavaSourceConversion",
      u8"tntExplosionDropDecay",
      u8"waterSourceConversion",
      u8"mobExplosionDropDecay",
      u8"snowAccumulationHeight",
      u8"commandModificationBlockLimit",
      u8"doVinesSpread",
      u8"maxCommandForkCount",
      u8"playersNetherPortalCreativeDelay",
      u8"playersNetherPortalDefaultDelay",
      u8"spawnChunkRadius",
  };
  for (u8string const &rule : ignoredGameRules) {
    blacklist.insert(u8"GameRules/" + rule);
  }
  unordered_set<u8string> ignoredPlayerAttributes = {
      u8"attributes",
      u8"EnderItems/*/tag/HideFlags", //?
      u8"EnderItems/*/tag/map",
      u8"Fire",
      u8"Motion",
      u8"Score",
      u8"foodTickTimer",
      u8"previousPlayerGameType",
      u8"recipeBook",
      u8"HurtByTimestamp",
      u8"SpawnAngle",
      u8"SpawnForced",
      u8"active_effects",        // TODO:
      u8"enteredNetherPosition", // TODO: exists when logged out from the nether
      u8"ignore_fall_damage_from_current_explosion",
      u8"spawn_extra_particles_on_fall",
  };
  for (u8string const &rule : ignoredPlayerAttributes) {
    blacklist.insert(u8"Player/" + rule);
  }
  for (u8string const &s : blacklist) {
    Erase(dataE, s);
    Erase(dataA, s);
  }
  auto playerE = dataE->compoundTag(u8"Player");
  auto playerA = dataA->compoundTag(u8"Player");
  CheckEntityJ(u8"player", *playerE, *playerA);

  dataE->erase(u8"Player");
  dataA->erase(u8"Player");
  DiffCompoundTag(*dataE, *dataA);
}

static void CheckPoiJ(mcfile::je::Region const &regionE, mcfile::je::Region const &regionA, int cx, int cz, Dimension dim) {
  auto poiE = regionE.exportToNbt(cx, cz);
  if (poiE) {
    static std::unordered_set<std::u8string> const blacklist = {
        u8"Sections/*/Valid",
        u8"DataVersion"};
    for (u8string const &s : blacklist) {
      Erase(poiE, s);
    }

    static std::unordered_set<std::u8string> const typeBlacklist = {
        u8"minecraft:test_instance",
    };
    if (auto sectionsE = poiE->compoundTag(u8"Sections"); sectionsE) {
      for (auto &[chunk, section] : sectionsE->fValue) {
        auto sectionCompound = section->asCompound();
        REQUIRE(sectionCompound);
        auto records = sectionCompound->listTag(u8"Records");
        if (!records) {
          continue;
        }
        for (int i = (int)records->fValue.size() - 1; i >= 0; i--) {
          auto record = records->fValue[i];
          auto recordCompound = record->asCompound();
          REQUIRE(recordCompound);
          if (typeBlacklist.count(recordCompound->string(u8"type", u8"")) > 0) {
            records->fValue.erase(records->fValue.begin() + i);
          }
        }
      }
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
  auto sectionsE = poiE->compoundTag(u8"Sections");
  auto sectionsA = poiA->compoundTag(u8"Sections");
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

    auto recordsE = sectionE->listTag(u8"Records");
    auto recordsA = sectionA->listTag(u8"Records");
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
          auto posE = recordE->intArrayTag(u8"pos");
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
            auto posA = recordA->intArrayTag(u8"pos");
            if (!posA) {
              continue;
            }
            if (posA->fValue.size() != 3) {
              continue;
            }
            if (posA->fValue[0] == x && posA->fValue[1] == y && posA->fValue[2] == z) {
              CHECK(recordE->string(u8"type") == recordA->string(u8"type"));
              auto freeTicketsE = recordE->int32(u8"free_tickets");
              auto freeTicketsA = recordA->int32(u8"free_tickets");
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
  je2be::java::Options optB;
  optB.fTempDirectory = mcfile::File::CreateTempDir(fs::temp_directory_path());
  defer {
    if (optB.fTempDirectory) {
      Fs::DeleteAll(*optB.fTempDirectory);
    }
  };
  bool multithread = true;
  bool checkLevelDat = true;
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
#if 1
  for (int cx = 0; cx <= J2B2J_MAX_CHUNK_X; cx++) {
    chunks.insert({cx, 0});
  }
#else
  Pos2i center(29, 0);
  int radius = 0;
  for (int cz = center.fZ - radius; cz <= center.fZ + radius; cz++) {
    for (int cx = center.fX - radius; cx <= center.fX + radius; cx++) {
      chunks.insert({cx, cz});
    }
  }
#endif
  for (Pos2i const &p : chunks) {
    for (int dx = -1; dx <= 1; dx++) {
      for (int dz = -1; dz <= 1; dz++) {
        optB.fChunkFilter.insert({p.fX + dx, p.fZ + dz});
      }
    }
  }
  multithread = false;
  checkLevelDat = false;
#endif
  je2be::bedrock::Options optJ;
  optJ.fTempDirectory = mcfile::File::CreateTempDir(fs::temp_directory_path());
  optJ.fDimensionFilter = optB.fDimensionFilter;
  optJ.fChunkFilter = optB.fChunkFilter;
  defer {
    if (optJ.fTempDirectory) {
      Fs::DeleteAll(*optJ.fTempDirectory);
    }
  };

  Status st = je2be::java::Converter::Run(in, *outB, optB, multithread ? thread::hardware_concurrency() : 0);
  CHECK(st.ok());

  // bedrock -> java
  auto outJ = mcfile::File::CreateTempDir(*tmp);
  CHECK(outJ);
  st = je2be::bedrock::Converter::Run(*outB, *outJ, optJ, multithread ? thread::hardware_concurrency() : 0);
  CHECK(st.ok());

  // Compare initial Java input and final Java output.

  if (checkLevelDat) {
    CheckLevelDatJ(in / "level.dat", *outJ / "level.dat");
  }

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
      if (fileA.extension() != u8".mca") {
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
        Parallel::Process<Pos2i>(
            regionChunks,
            thread::hardware_concurrency(),
            [regionE, regionA, dim](Pos2i const &pos) -> Status {
              CheckChunkJ(*regionE, *regionA, pos.fX, pos.fZ, dim);
              return Status::Ok();
            });
      } else {
        for (auto const &pos : regionChunks) {
          CheckChunkJ(*regionE, *regionA, pos.fX, pos.fZ, dim);
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
      if (fileA.extension() != u8".mca") {
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
        Parallel::Process<Pos2i>(
            regionChunks,
            thread::hardware_concurrency(),
            [regionE, regionA, dim](Pos2i const &pos) -> Status {
              CheckPoiJ(*regionE, *regionA, pos.fX, pos.fZ, dim);
              return Status::Ok();
            });
      } else {
        for (auto const &pos : regionChunks) {
          CheckPoiJ(*regionE, *regionA, pos.fX, pos.fZ, dim);
        }
      }
    }
  }
}
