#include <doctest/doctest.h>
#include <je2be.hpp>

using namespace std;
using namespace je2be;
using namespace mcfile::nbt;
namespace fs = std::filesystem;

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

static void CheckBlock(shared_ptr<mcfile::je::Block const> const &blockE, shared_ptr<mcfile::je::Block const> const &blockA) {
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
        } else {
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
    cerr << "actual:" << endl;
    for (int i = 0; i < linesA.size(); i++) {
      if (i < linesE.size() && linesA[i] != linesE[i]) {
        cerr << ">> ";
      } else {
        cerr << "   ";
      }
      cerr << i << ":\t" << linesA[i] << endl;
    }
    cerr << "expected:" << endl;
    for (int i = 0; i < linesE.size(); i++) {
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
      "Levels", // beacon. Sometimes reset to 0 in JE
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

  unordered_set<string> blacklist = {
      "UUID",
      "Attributes",
      "Motion",
      "LeftHanded", // left handed skeleton does not exist in BE
      "ForcedAge",
      "IsChickenJockey", //TODO: remove this
      "Item/tag/map",
      "Owner",
  };
  if (id == "minecraft:armor_stand") {
    blacklist.insert("Pose");
    blacklist.insert("Health"); // Default health differs. B = 6, J = 20
    blacklist.insert("Pos/1");  // y is aligned 0.5 block in BE
  } else if (id == "minecraft:slime") {
    blacklist.insert("wasOnGround"); // wasOnGround does not exist in BE
  } else if (id == "minecraft:salmon") {
    blacklist.insert("Health"); // Health differs. B = 6, J = 3
  }
  auto itemId = entityE.query("Item/id");
  if (itemId && itemId->asString()) {
    auto name = itemId->asString()->fValue;
    if (name == "minecraft:petrified_oak_slab" || name == "minecraft:furnace_minecart") {
      blacklist.insert("Item/id");
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

static shared_ptr<CompoundTag> FindNearestEntity(Pos3d pos, std::string const &id, vector<shared_ptr<CompoundTag>> const &entities) {
  shared_ptr<CompoundTag> ret = nullptr;
  double minDistance = numeric_limits<double>::max();
  for (auto const &entity : entities) {
    if (entity->string("id") != id) {
      continue;
    }
    auto p = props::GetPos3d(*entity, "Pos");
    if (!p) {
      continue;
    }
    double dx = p->fX - pos.fX;
    double dy = p->fY - pos.fY;
    double dz = p->fZ - pos.fZ;
    double distance = hypot(dx, dy, dz);
    if (distance < minDistance) {
      minDistance = distance;
      ret = entity;
    }
  }
  return ret;
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
  for (int cx = 0; cx <= 20; cx++) {
    io.fChunkFilter.insert(Pos2i(cx, 0));
  }
  io.fDimensionFilter.insert(mcfile::Dimension::Overworld);
  je2be::tobe::OutputOption oo;
  je2be::tobe::Converter tobe(in, io, *outB, oo);
  CHECK(tobe.run(thread::hardware_concurrency()));

  // bedrock -> java
  auto outJ = mcfile::File::CreateTempDir(*tmp);
  CHECK(outJ);
  je2be::toje::Converter toje(*outB, *outJ);
  CHECK(toje.run(thread::hardware_concurrency()));

  // Compare initial Java input and final Java output.

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
          }
          auto chunkA = regionA->chunkAt(cx, cz);
          if (!chunkA) {
            continue;
          }
          CHECK(regionA->entitiesAt(chunkA->fChunkX, chunkA->fChunkZ, chunkA->fEntities));

          auto chunkE = regionE->chunkAt(cx, cz);
          CHECK(chunkE);
          CHECK(regionE->entitiesAt(chunkE->fChunkX, chunkE->fChunkZ, chunkE->fEntities));

          CHECK(chunkA->minBlockY() == chunkE->minBlockY());
          CHECK(chunkA->maxBlockY() == chunkE->maxBlockY());

          for (int y = chunkE->minBlockY(); y <= chunkE->maxBlockY(); y++) {
            for (int z = chunkE->minBlockZ(); z <= chunkE->maxBlockZ(); z++) {
              for (int x = chunkE->minBlockX(); x <= chunkE->maxBlockX(); x++) {
                auto blockA = chunkA->blockAt(x, y, z);
                auto blockE = chunkE->blockAt(x, y, z);
                CheckBlock(blockE, blockA);
              }
            }
          }

          for (int y = chunkE->minBlockY(); y <= chunkE->maxBlockY(); y++) {
            for (int z = chunkE->minBlockZ(); z <= chunkE->maxBlockZ(); z++) {
              for (int x = chunkE->minBlockX(); x <= chunkE->maxBlockX(); x++) {
                CHECK(chunkA->biomeAt(x, y, z) == chunkE->biomeAt(x, y, z));
              }
            }
          }

          for (auto const &it : chunkE->fTileEntities) {
            Pos3i pos = it.first;
            shared_ptr<CompoundTag> const &tileE = it.second;
            static unordered_set<string> blacklist({"minecraft:sculk_sensor"});
            blacklist.insert("minecraft:beehive"); //TODO: remove this
            if (blacklist.find(tileE->string("id", "")) != blacklist.end()) {
              continue; //TODO: remove this
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
            auto id = entityE->string("id");
            shared_ptr<CompoundTag> entityA = FindNearestEntity(posE, *id, chunkA->fEntities);
            if (entityA) {
              CheckEntity(*id, *entityE, *entityA);
            } else {
              PrintAsJson(cerr, *entityE, {.fTypeHint = true});
              CHECK(false);
              break;
            }
          }
        }
      }
    }
  }
}
