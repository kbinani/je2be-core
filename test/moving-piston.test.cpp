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

static string ReIndent(string input) {
  int indent = 0;
  string s = input;
  while (strings::StartsWith(s, " ")) {
    indent++;
    s = s.substr(1);
  }

  vector<string> lines = mcfile::detail::String::Split(s, '\n');
  for (int i = 0; i < lines.size(); i++) {
    string l = lines[i];
    for (int j = 0; j < indent && strings::StartsWith(l, " "); j++) {
      l = l.substr(1);
    }
    lines[i] = l;
  }

  string out;
  for (int i = 0; i < lines.size(); i++) {
    out += lines[i];
    if (i + 1 < lines.size()) {
      out += "\n";
    }
  }
  return out;
}

static void CheckStringLineByLine(string actual, string expected) {
  CHECK(actual == expected);
  if (actual != expected) {
    vector<string> aLines = mcfile::detail::String::Split(actual, '\n');
    vector<string> eLines = mcfile::detail::String::Split(expected, '\n');
    for (size_t i = 0; i < aLines.size() && i < eLines.size(); i++) {
      CHECK(aLines[i] == eLines[i]);
    }
    CHECK(aLines.size() == eLines.size());
  }
}

static void CheckTileEntity(shared_ptr<CompoundTag const> const &tag, string expected) {
  CHECK(tag);
  if (!tag) {
    return;
  }
  ostringstream ss;
  JsonPrintOptions o;
  o.fTypeHint = true;
  PrintAsJson(ss, *tag, o);
  string actual = strings::RTrim(ss.str(), "\n");
  string e = ReIndent(strings::Trim("\n", expected, "\n"));
  CheckStringLineByLine(actual, e);
}

TEST_CASE("moving-piston") {
  SUBCASE("extending=0") {
    auto [chunk, region] = Load("extending=0");
    MovingPiston::PreprocessChunk(chunk, *region);
    CheckTileEntity(chunk->fTileEntities[Pos3i(14, 5, 8)], R"(
      {
        "id": "j2b:MovingBlock",
        "isMovable": 1, // byte
        "movingBlock": {
          "name": "minecraft:stone",
          "states": {
            "stone_type": "stone"
          },
          "version": 17879555 // int
        },
        "movingBlockExtra": {
          "name": "minecraft:air",
          "states": {
          },
          "version": 17879555 // int
        },
        "pistonPosX": 14, // int
        "pistonPosY": 4, // int
        "pistonPosZ": 9, // int
        "x": 14, // int
        "y": 5, // int
        "z": 8 // int
      })");
    CheckTileEntity(chunk->fTileEntities[Pos3i(15, 6, 9)], R"(
      {
        "id": "j2b:MovingBlock",
        "isMovable": 1, // byte
        "movingBlock": {
          "name": "minecraft:wool",
          "states": {
            "color": "blue"
          },
          "version": 17879555 // int
        },
        "movingBlockExtra": {
          "name": "minecraft:air",
          "states": {
          },
          "version": 17879555 // int
        },
        "x": 15, // int
        "y": 6, // int
        "z": 9 // int
      })");
    CheckTileEntity(chunk->fTileEntities[Pos3i(14, 5, 9)], R"(
      {
        "id": "j2b:MovingBlock",
        "isMovable": 1, // byte
        "movingBlock": {
          "name": "minecraft:slime",
          "states": {
          },
          "version": 17879555 // int
        },
        "movingBlockExtra": {
          "name": "minecraft:air",
          "states": {
          },
          "version": 17879555 // int
        },
        "pistonPosX": 14, // int
        "pistonPosY": 4, // int
        "pistonPosZ": 9, // int
        "x": 14, // int
        "y": 5, // int
        "z": 9 // int
      })");
    CheckTileEntity(chunk->fTileEntities[Pos3i(15, 5, 9)], R"(
      {
        "id": "j2b:MovingBlock",
        "isMovable": 1, // byte
        "movingBlock": {
          "name": "minecraft:slime",
          "states": {
          },
          "version": 17879555 // int
        },
        "movingBlockExtra": {
          "name": "minecraft:air",
          "states": {
          },
          "version": 17879555 // int
        },
        "pistonPosX": 14, // int
        "pistonPosY": 4, // int
        "pistonPosZ": 9, // int
        "x": 15, // int
        "y": 5, // int
        "z": 9 // int
      })");
    CheckTileEntity(chunk->fTileEntities[Pos3i(14, 4, 9)], R"(
      {
        "AttachedBlocks": [
          14, // int
          5, // int
          9, // int
          14, // int
          5, // int
          8, // int
          16, // int
          5, // int
          8, // int
          14, // int
          5, // int
          10, // int
          15, // int
          5, // int
          9, // int
          15, // int
          6, // int
          9, // int
          16, // int
          5, // int
          9, // int
          16, // int
          5, // int
          10, // int
          17, // int
          5, // int
          9 // int
        ],
        "BreakBlocks": [
        ],
        "LastProgress": 0.5, // float
        "NewState": 3, // byte
        "Progress": 0, // float
        "State": 3, // byte
        "Sticky": 1, // byte
        "id": "j2b:PistonArm",
        "isMovable": 0, // byte
        "x": 14, // int
        "y": 4, // int
        "z": 9 // int
      })");
    CheckTileEntity(chunk->fTileEntities[Pos3i(14, 5, 10)], R"(
      {
        "id": "j2b:MovingBlock",
        "isMovable": 1, // byte
        "movingBlock": {
          "name": "minecraft:cobblestone",
          "states": {
          },
          "version": 17879555 // int
        },
        "movingBlockExtra": {
          "name": "minecraft:air",
          "states": {
          },
          "version": 17879555 // int
        },
        "pistonPosX": 14, // int
        "pistonPosY": 4, // int
        "pistonPosZ": 9, // int
        "x": 14, // int
        "y": 5, // int
        "z": 10 // int
      })");
  }

  SUBCASE("extending=1") {
    auto [chunk, region] = Load("extending=1");
    MovingPiston::PreprocessChunk(chunk, *region);
    CheckTileEntity(chunk->fTileEntities[Pos3i(14, 6, 8)], R"(
      {
        "id": "j2b:MovingBlock",
        "isMovable": 1, // byte
        "movingBlock": {
          "name": "minecraft:stone",
          "states": {
            "stone_type": "stone"
          },
          "version": 17879555 // int
        },
        "movingBlockExtra": {
          "name": "minecraft:air",
          "states": {
          },
          "version": 17879555 // int
        },
        "pistonPosX": 14, // int
        "pistonPosY": 4, // int
        "pistonPosZ": 9, // int
        "x": 14, // int
        "y": 6, // int
        "z": 8 // int
      })");
    CheckTileEntity(chunk->fTileEntities[Pos3i(14, 6, 9)], R"(
      {
        "id": "j2b:MovingBlock",
        "isMovable": 1, // byte
        "movingBlock": {
          "name": "minecraft:slime",
          "states": {
          },
          "version": 17879555 // int
        },
        "movingBlockExtra": {
          "name": "minecraft:air",
          "states": {
          },
          "version": 17879555 // int
        },
        "pistonPosX": 14, // int
        "pistonPosY": 4, // int
        "pistonPosZ": 9, // int
        "x": 14, // int
        "y": 6, // int
        "z": 9 // int
      })");
    CheckTileEntity(chunk->fTileEntities[Pos3i(15, 7, 9)], R"(
      {
        "id": "j2b:MovingBlock",
        "isMovable": 1, // byte
        "movingBlock": {
          "name": "minecraft:wool",
          "states": {
            "color": "blue"
          },
          "version": 17879555 // int
        },
        "movingBlockExtra": {
          "name": "minecraft:air",
          "states": {
          },
          "version": 17879555 // int
        },
        "x": 15, // int
        "y": 7, // int
        "z": 9 // int
      })");
    CheckTileEntity(chunk->fTileEntities[Pos3i(15, 6, 9)], R"(
      {
        "id": "j2b:MovingBlock",
        "isMovable": 1, // byte
        "movingBlock": {
          "name": "minecraft:slime",
          "states": {
          },
          "version": 17879555 // int
        },
        "movingBlockExtra": {
          "name": "minecraft:air",
          "states": {
          },
          "version": 17879555 // int
        },
        "pistonPosX": 14, // int
        "pistonPosY": 4, // int
        "pistonPosZ": 9, // int
        "x": 15, // int
        "y": 6, // int
        "z": 9 // int
      })");
    CheckTileEntity(chunk->fTileEntities[Pos3i(14, 6, 10)], R"(
      {
        "id": "j2b:MovingBlock",
        "isMovable": 1, // byte
        "movingBlock": {
          "name": "minecraft:cobblestone",
          "states": {
          },
          "version": 17879555 // int
        },
        "movingBlockExtra": {
          "name": "minecraft:air",
          "states": {
          },
          "version": 17879555 // int
        },
        "pistonPosX": 14, // int
        "pistonPosY": 4, // int
        "pistonPosZ": 9, // int
        "x": 14, // int
        "y": 6, // int
        "z": 10 // int
      })");
    CheckTileEntity(chunk->fTileEntities[Pos3i(14, 4, 9)], R"(
      {
        "AttachedBlocks": [
          15, // int
          6, // int
          9, // int
          14, // int
          5, // int
          9, // int
          14, // int
          5, // int
          8, // int
          16, // int
          5, // int
          8, // int
          14, // int
          5, // int
          10, // int
          15, // int
          5, // int
          9, // int
          16, // int
          5, // int
          9, // int
          16, // int
          5, // int
          10, // int
          17, // int
          5, // int
          9 // int
        ],
        "BreakBlocks": [
        ],
        "LastProgress": 0, // float
        "NewState": 1, // byte
        "Progress": 0.5, // float
        "State": 1, // byte
        "Sticky": 1, // byte
        "id": "j2b:PistonArm",
        "isMovable": 0, // byte
        "x": 14, // int
        "y": 4, // int
        "z": 9 // int
      })");
  }

  SUBCASE("normal_extending=1") {
    auto [chunk, region] = Load("normal_extending=1");
    MovingPiston::PreprocessChunk(chunk, *region);
    CheckTileEntity(chunk->fTileEntities[Pos3i(14, 6, 9)], R"(
      {
        "id": "j2b:MovingBlock",
        "isMovable": 1, // byte
        "movingBlock": {
          "name": "minecraft:sand",
          "states": {
            "sand_type": "normal"
          },
          "version": 17879555 // int
        },
        "movingBlockExtra": {
          "name": "minecraft:air",
          "states": {
          },
          "version": 17879555 // int
        },
        "pistonPosX": 14, // int
        "pistonPosY": 4, // int
        "pistonPosZ": 9, // int
        "x": 14, // int
        "y": 6, // int
        "z": 9 // int
      })");
    CheckTileEntity(chunk->fTileEntities[Pos3i(14, 4, 9)], R"(
      {
        "AttachedBlocks": [
          14, // int
          5, // int
          9 // int
        ],
        "BreakBlocks": [
        ],
        "LastProgress": 0, // float
        "NewState": 1, // byte
        "Progress": 0.5, // float
        "State": 1, // byte
        "Sticky": 0, // byte
        "id": "j2b:PistonArm",
        "isMovable": 0, // byte
        "x": 14, // int
        "y": 4, // int
        "z": 9 // int
      })");
  }
}
