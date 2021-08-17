#include <doctest/doctest.h>
#include <iostream>
#include <je2be.hpp>
#include <sstream>

using namespace j2b;
using namespace mcfile;
using namespace mcfile::nbt;
using namespace std;
namespace fs = std::filesystem;

static tuple<shared_ptr<Chunk>, shared_ptr<Region>> Load(string type) {
  fs::path thisFile(__FILE__);
  World world((thisFile.parent_path() / "data" / "piston" / type).string());
  auto region = world.region(0, 0);
  auto chunk = region->chunkAt(0, 0);
  return make_pair(chunk, region);
}

struct ExpectedBlock {
  string name;
  map<string, string> states;
  int version;
};

struct ExpectedMovingBlock {
  string id;
  bool isMovable;
  ExpectedBlock movingBlock;
  ExpectedBlock movingBlockExtra;
  int pistonPosX;
  int pistonPosY;
  int pistonPosZ;
  int x;
  int y;
  int z;
};

static void CheckStringStringMap(shared_ptr<CompoundTag const> const &tag, map<string, string> expected) {
  CHECK(tag);
  if (!tag) {
    return;
  }
  CHECK(tag->fValue.size() == expected.size());
  for (auto it : expected) {
    auto found = tag->find(it.first);
    CHECK(found != tag->end());
    CHECK(it.second == found->second->asString()->fValue);
  }
}

static void CheckBlock(shared_ptr<CompoundTag const> const &tag, ExpectedBlock expected) {
  CHECK(tag->string("name") == expected.name);
  CheckStringStringMap(tag->compoundTag("states"), expected.states);
  CHECK(tag->int32("version") == expected.version);
}

static void CheckMovingBlock(shared_ptr<CompoundTag const> const &actual, ExpectedMovingBlock expected) {
  CHECK(actual->string("id") == expected.id);
  CHECK(actual->boolean("isMovable") == expected.isMovable);
  CheckBlock(actual->compoundTag("movingBlock"), expected.movingBlock);
  CheckBlock(actual->compoundTag("movingBlockExtra"), expected.movingBlockExtra);
  CHECK(actual->int32("pistonPosX") == expected.pistonPosX);
  CHECK(actual->int32("pistonPosY") == expected.pistonPosY);
  CHECK(actual->int32("pistonPosZ") == expected.pistonPosZ);
  CHECK(actual->int32("x") == expected.x);
  CHECK(actual->int32("y") == expected.y);
  CHECK(actual->int32("z") == expected.z);
}

struct ExpectedPistonArm {
  unordered_set<Pos3i, Pos3iHasher> AttachedBlocks;
  unordered_set<Pos3i, Pos3iHasher> BreakBlocks;
  float LastProgress;
  uint8_t NewState;
  float Progress;
  uint8_t State;
  bool Sticky;
  string id;
  bool isMovable;
  int x;
  int y;
  int z;
};

static void CheckPos3Set(shared_ptr<ListTag const> const &actual, unordered_set<Pos3i, Pos3iHasher> expected) {
  CHECK(actual->fValue.size() % 3 == 0);
  unordered_set<Pos3i, Pos3iHasher> actualBlocks;
  for (int i = 0; i < actual->fValue.size(); i += 3) {
    int x = actual->fValue[i]->asInt()->fValue;
    int y = actual->fValue[i + 1]->asInt()->fValue;
    int z = actual->fValue[i + 2]->asInt()->fValue;
    actualBlocks.insert(Pos3i(x, y, z));
  }
  CHECK(actualBlocks.size() == expected.size());
  for (Pos3i e : expected) {
    CHECK(actualBlocks.find(e) != actualBlocks.end());
  }
}

static void CheckPistonArm(shared_ptr<CompoundTag const> const &actual, ExpectedPistonArm expected) {
  CheckPos3Set(actual->listTag("AttachedBlocks"), expected.AttachedBlocks);
  CheckPos3Set(actual->listTag("BreakBlocks"), expected.BreakBlocks);
  CHECK(actual->float32("LastProgress") == expected.LastProgress);
  CHECK(actual->byte("NewState") == expected.NewState);
  CHECK(actual->float32("Progress") == expected.Progress);
  CHECK(actual->byte("State") == expected.State);
  CHECK(actual->boolean("Sticky") == expected.Sticky);
  CHECK(actual->string("id") == expected.id);
  CHECK(actual->boolean("isMovable") == expected.isMovable);
  CHECK(actual->int32("x") == expected.x);
  CHECK(actual->int32("y") == expected.y);
  CHECK(actual->int32("z") == expected.z);
}

TEST_CASE("moving-piston") {
  SUBCASE("extending=0") {
    auto [chunk, region] = Load("extending=0");
    MovingPiston::PreprocessChunk(chunk, *region);
    CheckMovingBlock(chunk->fTileEntities[Pos3i(14, 5, 8)], {.id = "j2b:MovingBlock",
                                                             .isMovable = true,
                                                             .movingBlock = {
                                                                 .name = "minecraft:stone",
                                                                 .states = {
                                                                     {"stone_type", "stone"},
                                                                 },
                                                                 .version = BlockData::kBlockDataVersion},
                                                             .movingBlockExtra = {.name = "minecraft:air", .states = {}, .version = BlockData::kBlockDataVersion},
                                                             .pistonPosX = 14,
                                                             .pistonPosY = 4,
                                                             .pistonPosZ = 9,
                                                             .x = 14,
                                                             .y = 5,
                                                             .z = 8});
    CheckMovingBlock(chunk->fTileEntities[Pos3i(15, 6, 9)], {.id = "j2b:MovingBlock",
                                                             .isMovable = 1,
                                                             .movingBlock = {
                                                                 .name = "minecraft:wool",
                                                                 .states = {
                                                                     {"color", "blue"}},
                                                                 .version = BlockData::kBlockDataVersion},
                                                             .movingBlockExtra = {.name = "minecraft:air", .states = {}, .version = BlockData::kBlockDataVersion},
                                                             .pistonPosX = 14,
                                                             .pistonPosY = 4,
                                                             .pistonPosZ = 9,
                                                             .x = 15,
                                                             .y = 6,
                                                             .z = 9});
    CheckMovingBlock(chunk->fTileEntities[Pos3i(14, 5, 9)], {.id = "j2b:MovingBlock",
                                                             .isMovable = 1,
                                                             .movingBlock = {
                                                                 .name = "minecraft:slime",
                                                                 .states = {},
                                                                 .version = BlockData::kBlockDataVersion},
                                                             .movingBlockExtra = {.name = "minecraft:air", .states = {}, .version = BlockData::kBlockDataVersion},
                                                             .pistonPosX = 14,
                                                             .pistonPosY = 4,
                                                             .pistonPosZ = 9,
                                                             .x = 14,
                                                             .y = 5,
                                                             .z = 9});
    CheckMovingBlock(chunk->fTileEntities[Pos3i(15, 5, 9)], {.id = "j2b:MovingBlock",
                                                             .isMovable = 1,
                                                             .movingBlock = {
                                                                 .name = "minecraft:slime",
                                                                 .states = {},
                                                                 .version = BlockData::kBlockDataVersion},
                                                             .movingBlockExtra = {.name = "minecraft:air", .states = {}, .version = BlockData::kBlockDataVersion},
                                                             .pistonPosX = 14,
                                                             .pistonPosY = 4,
                                                             .pistonPosZ = 9,
                                                             .x = 15,
                                                             .y = 5,
                                                             .z = 9});
    CheckPistonArm(chunk->fTileEntities[Pos3i(14, 4, 9)], {.AttachedBlocks = {
                                                               {
                                                                   14,
                                                                   5,
                                                                   9,
                                                               },
                                                               {14, 5, 8},
                                                               {16, 5, 8},
                                                               {14, 5, 10},
                                                               {15, 5, 9},
                                                               {15, 6, 9},
                                                               {16, 5, 9},
                                                               {16, 5, 10},
                                                               {17, 5, 9}},
                                                           .BreakBlocks = {},
                                                           .LastProgress = 0.5,
                                                           .NewState = 3,
                                                           .Progress = 0,
                                                           .State = 3,
                                                           .Sticky = true,
                                                           .id = "j2b:PistonArm",
                                                           .isMovable = 0,
                                                           .x = 14,
                                                           .y = 4,
                                                           .z = 9});
    CheckMovingBlock(chunk->fTileEntities[Pos3i(14, 5, 10)], {.id = "j2b:MovingBlock",
                                                              .isMovable = 1,
                                                              .movingBlock = {
                                                                  .name = "minecraft:cobblestone",
                                                                  .states = {},
                                                                  .version = BlockData::kBlockDataVersion},
                                                              .movingBlockExtra = {.name = "minecraft:air", .states = {}, .version = BlockData::kBlockDataVersion},
                                                              .pistonPosX = 14,
                                                              .pistonPosY = 4,
                                                              .pistonPosZ = 9,
                                                              .x = 14,
                                                              .y = 5,
                                                              .z = 10});
  }

  SUBCASE("extending=1") {
    auto [chunk, region] = Load("extending=1");
    MovingPiston::PreprocessChunk(chunk, *region);
    CheckMovingBlock(chunk->fTileEntities[Pos3i(14, 6, 8)], {.id = "j2b:MovingBlock",
                                                             .isMovable = 1,
                                                             .movingBlock = {
                                                                 .name = "minecraft:stone",
                                                                 .states = {
                                                                     {"stone_type", "stone"}},
                                                                 .version = BlockData::kBlockDataVersion},
                                                             .movingBlockExtra = {.name = "minecraft:air", .states = {}, .version = BlockData::kBlockDataVersion},
                                                             .pistonPosX = 14,
                                                             .pistonPosY = 4,
                                                             .pistonPosZ = 9,
                                                             .x = 14,
                                                             .y = 6,
                                                             .z = 8});
    CheckMovingBlock(chunk->fTileEntities[Pos3i(14, 6, 9)], {.id = "j2b:MovingBlock",
                                                             .isMovable = 1,
                                                             .movingBlock = {
                                                                 .name = "minecraft:slime",
                                                                 .states = {},
                                                                 .version = BlockData::kBlockDataVersion},
                                                             .movingBlockExtra = {.name = "minecraft:air", .states = {}, .version = BlockData::kBlockDataVersion},
                                                             .pistonPosX = 14,
                                                             .pistonPosY = 4,
                                                             .pistonPosZ = 9,
                                                             .x = 14,
                                                             .y = 6,
                                                             .z = 9});
    CheckMovingBlock(chunk->fTileEntities[Pos3i(15, 7, 9)], {.id = "j2b:MovingBlock",
                                                             .isMovable = 1,
                                                             .movingBlock = {
                                                                 .name = "minecraft:wool",
                                                                 .states = {
                                                                     {"color", "blue"}},
                                                                 .version = BlockData::kBlockDataVersion},
                                                             .movingBlockExtra = {.name = "minecraft:air", .states = {}, .version = BlockData::kBlockDataVersion},
                                                             .pistonPosX = 14,
                                                             .pistonPosY = 4,
                                                             .pistonPosZ = 9,
                                                             .x = 15,
                                                             .y = 7,
                                                             .z = 9});
    CheckMovingBlock(chunk->fTileEntities[Pos3i(15, 6, 9)], {.id = "j2b:MovingBlock",
                                                             .isMovable = 1,
                                                             .movingBlock = {
                                                                 .name = "minecraft:slime",
                                                                 .states = {},
                                                                 .version = BlockData::kBlockDataVersion},
                                                             .movingBlockExtra = {.name = "minecraft:air", .states = {}, .version = BlockData::kBlockDataVersion},
                                                             .pistonPosX = 14,
                                                             .pistonPosY = 4,
                                                             .pistonPosZ = 9,
                                                             .x = 15,
                                                             .y = 6,
                                                             .z = 9});
    CheckMovingBlock(chunk->fTileEntities[Pos3i(14, 6, 10)], {.id = "j2b:MovingBlock",
                                                              .isMovable = 1,
                                                              .movingBlock = {
                                                                  .name = "minecraft:cobblestone",
                                                                  .states = {},
                                                                  .version = BlockData::kBlockDataVersion},
                                                              .movingBlockExtra = {.name = "minecraft:air", .states = {}, .version = BlockData::kBlockDataVersion},
                                                              .pistonPosX = 14,
                                                              .pistonPosY = 4,
                                                              .pistonPosZ = 9,
                                                              .x = 14,
                                                              .y = 6,
                                                              .z = 10});
    CheckPistonArm(chunk->fTileEntities[Pos3i(14, 4, 9)], {.AttachedBlocks = {
                                                               {15, 6, 9},
                                                               {14, 5, 9},
                                                               {14, 5, 8},
                                                               {16, 5, 8},
                                                               {14, 5, 10},
                                                               {15, 5, 9},
                                                               {16, 5, 9},
                                                               {16, 5, 10},
                                                               {17, 5, 9}},
                                                           .BreakBlocks = {},
                                                           .LastProgress = 0,
                                                           .NewState = 1,
                                                           .Progress = 0.5,
                                                           .State = 1,
                                                           .Sticky = true,
                                                           .id = "j2b:PistonArm",
                                                           .isMovable = 0,
                                                           .x = 14,
                                                           .y = 4,
                                                           .z = 9});
  }

  SUBCASE("normal_extending=1") {
    auto [chunk, region] = Load("normal_extending=1");
    MovingPiston::PreprocessChunk(chunk, *region);
    CheckMovingBlock(chunk->fTileEntities[Pos3i(14, 6, 9)], {.id = "j2b:MovingBlock",
                                                             .isMovable = 1,
                                                             .movingBlock = {
                                                                 .name = "minecraft:sand",
                                                                 .states = {
                                                                     {"sand_type", "normal"}},
                                                                 .version = BlockData::kBlockDataVersion},
                                                             .movingBlockExtra = {.name = "minecraft:air", .states = {}, .version = BlockData::kBlockDataVersion},
                                                             .pistonPosX = 14,
                                                             .pistonPosY = 4,
                                                             .pistonPosZ = 9,
                                                             .x = 14,
                                                             .y = 6,
                                                             .z = 9});
    CheckPistonArm(chunk->fTileEntities[Pos3i(14, 4, 9)], {.AttachedBlocks = {
                                                               {14, 5, 9}},
                                                           .BreakBlocks = {},
                                                           .LastProgress = 0,
                                                           .NewState = 1,
                                                           .Progress = 0.5,
                                                           .State = 1,
                                                           .Sticky = false,
                                                           .id = "j2b:PistonArm",
                                                           .isMovable = 0,
                                                           .x = 14,
                                                           .y = 4,
                                                           .z = 9});
  }
}
