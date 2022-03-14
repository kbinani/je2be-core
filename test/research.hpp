#pragma once

static optional<wstring> GetLocalApplicationDirectory() {
#if __has_include(<shlobj_core.h>)
  int csidType = CSIDL_LOCAL_APPDATA;
  wchar_t path[MAX_PATH + 256];

  if (SHGetSpecialFolderPathW(nullptr, path, csidType, FALSE)) {
    return wstring(path);
  }
#endif
  return nullopt;
}

static optional<fs::path> GetWorldDirectory(string const &name) {
  auto appDir = GetLocalApplicationDirectory(); // X:/Users/whoami/AppData/Local
  if (!appDir) {
    return nullopt;
  }
  return fs::path(*appDir) / L"Packages" / L"Microsoft.MinecraftUWP_8wekyb3d8bbwe" / L"LocalState" / L"games" / L"com.mojang" / L"minecraftWorlds" / name / L"db";
}

static leveldb::DB *OpenF(fs::path p) {
  using namespace leveldb;
  Options o;
  o.compression = kZlibRawCompression;
  DB *db;
  Status st = DB::Open(o, p, &db);
  if (!st.ok()) {
    return nullptr;
  }
  return db;
}

static leveldb::DB *Open(string const &name) {
  using namespace leveldb;
  auto dir = GetWorldDirectory(name);
  if (!dir) {
    return nullptr;
  }
  return OpenF(*dir);
}

static void VisitDbUntil(string const &name, function<bool(string const &key, string const &value, leveldb::DB *db)> callback) {
  using namespace leveldb;
  unique_ptr<DB> db(Open(name));
  if (!db) {
    return;
  }
  ReadOptions ro;
  unique_ptr<Iterator> itr(db->NewIterator(ro));
  for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
    string k = itr->key().ToString();
    string v = itr->value().ToString();
    if (!callback(k, v, db.get())) {
      break;
    }
  }
}

static void VisitDb(string const &name, function<void(string const &key, string const &value, leveldb::DB *db)> callback) {
  VisitDbUntil(name, [callback](string const &k, string const &v, leveldb::DB *db) {
    callback(k, v, db);
    return true;
  });
}

static void FenceGlassPaneIronBarsConnectable() {
  set<string> uniq;
  for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
    string name = mcfile::blocks::Name(id);
    if (name.ends_with("_stairs")) {
      continue;
    }
    if (name.ends_with("piston")) {
      continue;
    }
    if (name.ends_with("door")) {
      continue;
    }
    uniq.insert(name);
  }
  vector<string> names(uniq.begin(), uniq.end());

  int const x0 = -42;
  int const z0 = 165;
  int const y = 4;
  int x = x0;
  int x1 = x0;
  fs::path root("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/labo");
  {
    ofstream os((root / "datapacks" / "kbinani" / "data" / "je2be" / "functions" / "place_blocks.mcfunction").string());
    for (string const &name : names) {
      os << "setblock " << x << " " << y << " " << z0 << " " << name << endl;
      x++;
    }
    x1 = x;
    os << "fill " << x0 << " " << y << " " << (z0 + 1) << " " << x1 << " " << y << " " << (z0 + 1) << " glass_pane" << endl;
    os << "fill " << x0 << " " << y << " " << (z0 - 1) << " " << x1 << " " << y << " " << (z0 - 1) << " oak_fence" << endl;
  }

  // login the game, then execute /function je2be:place_blocks

  mcfile::je::World w(root);
  shared_ptr<mcfile::je::Chunk> chunk;
  int cz = mcfile::Coordinate::ChunkFromBlock(z0);
  set<string> glassPaneAttachable;
  set<string> fenceAttachable;
  for (int x = x0; x < x1; x++) {
    int cx = mcfile::Coordinate::ChunkFromBlock(x);
    if (!chunk || (chunk && chunk->fChunkX != cx)) {
      chunk = w.chunkAt(cx, cz);
    }
    auto center = chunk->blockAt(x, y, z0);
    auto expected = names[x - x0];
    if (expected != center->fName) {
      cerr << "block does not exist: expected=" << expected << "; actual=" << center->fName << endl;
    } else {
      auto fence = chunk->blockAt(x, y, z0 - 1);
      auto fenceAttached = fence->property("south") == "true";
      if (fenceAttached) {
        fenceAttachable.insert(expected);
      }

      auto glassPane = chunk->blockAt(x, y, z0 + 1);
      auto glassPaneAttached = glassPane->property("north") == "true";
      if (glassPaneAttached) {
        glassPaneAttachable.insert(expected);
      }
    }
  }

  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());
  code << "static bool IsFenceAlwaysAttachable(mcfile::blocks::BlockId id) {" << endl;
  code << "  switch (id) {" << endl;
  for (auto n : fenceAttachable) {
    code << "    case mcfile::blocks::minecraft::" << n.substr(10) << ":" << endl;
  }
  code << "      return true;" << endl;
  code << "    default:" << endl;
  code << "      break;" << endl;
  code << "  }" << endl;
  code << "  //TODO:" << endl;
  code << "  return false;" << endl;
  code << "}" << endl;
  code << endl;
  code << "static bool IsGlassPaneOrIronBarsAlwaysAttachable(mcfile::blocks::BlockId id) {" << endl;
  code << "  switch (id) {" << endl;
  for (auto n : glassPaneAttachable) {
    code << "    case mcfile::blocks::minecraft::" << n.substr(10) << ":" << endl;
  }
  code << "      return true;" << endl;
  code << "    default:" << endl;
  code << "      break;" << endl;
  code << "  }" << endl;
  code << "  //TODO:" << endl;
  code << "  return false;" << endl;
  code << "}" << endl;
}

static void NoteBlock() {
  set<string> uniq;
  for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
    string name = mcfile::blocks::Name(id);
    uniq.insert(name);
  }
  vector<string> names(uniq.begin(), uniq.end());

  int const x0 = -42;
  int const z0 = 165;
  int const y = 4;
  int x = x0;
  int x1 = x0;
  fs::path root("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/labo");
  {
    ofstream os((root / "datapacks" / "kbinani" / "data" / "je2be" / "functions" / "research_note_block.mcfunction").string());
    for (string const &name : names) {
      os << "setblock " << x << " " << (y - 1) << " " << z0 << " " << name << endl;
      os << "setblock " << x << " " << y << " " << z0 << " note_block" << endl;
      x += 2;
    }
    x1 = x;
  }

  // login the game, then execute /function je2be:research_note_block

  mcfile::je::World w(root);
  shared_ptr<mcfile::je::Chunk> chunk;
  int cz = mcfile::Coordinate::ChunkFromBlock(z0);
  map<string, set<string>> instruments;
  int i = 0;
  for (int x = x0; x < x1; x += 2, i++) {
    int cx = mcfile::Coordinate::ChunkFromBlock(x);
    if (!chunk || (chunk && chunk->fChunkX != cx)) {
      chunk = w.chunkAt(cx, cz);
    }
    auto center = chunk->blockAt(x, y - 1, z0);
    auto expected = names[i];
    if (expected != center->fName) {
      cerr << "block does not exist: expected=" << expected << "; actual=" << center->fName << endl;
    } else {
      auto noteBlock = chunk->blockAt(x, y, z0);
      auto instrument = noteBlock->property("instrument", "");
      if (instrument.empty()) {
        cerr << "empty instrument: [" << x << ", " << z0 << "]" << endl;
      } else {
        instruments[instrument].insert(center->fName);
      }
    }
  }

  instruments.erase("harp");

  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());
  code << "static std::string NoteBlockInstrument(mcfile::blocks::BlockId id) {" << endl;
  code << "  switch (id) {" << endl;
  for (auto const &it : instruments) {
    string instrument = it.first;
    set<string> const &blocks = it.second;
    for (string const &block : blocks) {
      code << "  case mcfile::blocks::minecraft::" << block.substr(10) << ":" << endl;
    }
    code << "    return \"" + instrument << "\";" << endl;
  }
  code << "  default:" << endl;
  code << "    return \"harp\"" << endl;
  code << "  }" << endl;
  code << "}" << endl;
  code << endl;
}

static void RedstoneWire() {
  set<string> uniq;
  for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
    string name = mcfile::blocks::Name(id);
    if (name.ends_with("slab")) {
      continue;
    }
    if (name.ends_with("rail")) {
      continue;
    }
    if (!mcfile::blocks::IsTransparent(id)) {
      continue;
    }
    uniq.insert(name);
  }
  vector<string> names(uniq.begin(), uniq.end());

  int const x0 = -42;
  int const z0 = 165;
  int const y = 4;
  int x = x0;
  int x1 = x0;
  fs::path root("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/labo");
  {
    ofstream os((root / "datapacks" / "kbinani" / "data" / "je2be" / "functions" / "research_redstone_wire.mcfunction").string());
    os << "fill " << x0 << " " << y << " " << (z0 - 1) << " " << (x0 + 2 * names.size()) << " " << (y + 2) << " " << (z0 + 1) << " air" << endl;
    for (string const &name : names) {
      os << "setblock " << x << " " << (y - 1) << " " << z0 << " air" << endl;
      os << "setblock " << x << " " << (y - 2) << " " << z0 << " redstone_wire" << endl;
      os << "setblock " << x << " " << (y - 1) << " " << (z0 - 1) << " redstone_wire" << endl;
      os << "setblock " << x << " " << (y - 1) << " " << (z0 + 1) << " redstone_wire" << endl;
      os << "setblock " << x << " " << (y - 1) << " " << z0 << " " << name << endl;
      x += 2;
    }
    x1 = x0 + 2 * names.size();
  }

  // login the game, then execute /function je2be:research_redstone_wire

  mcfile::je::World w(root);
  shared_ptr<mcfile::je::Chunk> chunk;
  int cz = mcfile::Coordinate::ChunkFromBlock(z0);
  set<string> transparent;
  int i = 0;
  for (int x = x0; x < x1; x += 2, i++) {
    int cx = mcfile::Coordinate::ChunkFromBlock(x);
    if (!chunk || (chunk && chunk->fChunkX != cx)) {
      chunk = w.chunkAt(cx, cz);
    }
    auto center = chunk->blockAt(x, y - 1, z0);
    auto expected = names[i];
    if (expected != center->fName) {
      cerr << "block does not exist: expected=" << expected << "; actual=" << center->fName << endl;
    } else {
      auto wire = chunk->blockAt(x, y - 2, z0);
      auto north = wire->property("north", "");
      auto south = wire->property("south", "");
      if (north != "up" && south != "up") {
        transparent.insert(expected);
      }
    }
  }

  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());
  code << "static bool IsAlwaysTransparentAgainstRedstoneWire(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  switch (id) {" << endl;
  for (auto const &it : transparent) {
    code << "  case " << it.substr(10) << ":" << endl;
  }
  code << "    true false;" << endl;
  code << "  }" << endl;
  code << "}" << endl;
  code << endl;
}

static void Data3D() {
  unique_ptr<leveldb::DB> db(Open("1.18be"));
  auto key = mcfile::be::DbKey::Data3D(0, 0, mcfile::Dimension::End);
  leveldb::ReadOptions ro;
  std::string value;
  if (auto st = db->Get(ro, key, &value); !st.ok()) {
    return;
  }
  auto bmap = mcfile::be::BiomeMap::Decode(0, value, 512);
  std::cout << bmap->numSections() << std::endl; // 11(or more) for overworld, 8 for nether, 4 for end
}

static void Structures() {
  using namespace leveldb;
  using namespace mcfile;
  using namespace mcfile::be;
  unique_ptr<DB> db(Open("1.18stronghold"));
  unique_ptr<Iterator> itr(db->NewIterator({}));
  for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
    auto key = itr->key().ToString();
    auto parsed = DbKey::Parse(key);
    if (!parsed) {
      continue;
    }
    if (!parsed->fIsTagged) {
      continue;
    }
    uint8_t tag = parsed->fTagged.fTag;
    if (tag != static_cast<uint8_t>(DbKey::Tag::StructureBounds)) {
      continue;
    }
    vector<StructurePiece> buffer;
    StructurePiece::Parse(itr->value().ToString(), buffer);
    for (StructurePiece const &p : buffer) {
      switch (p.fType) {
      case StructureType::Fortress:
      case StructureType::Monument:
      case StructureType::Outpost:
        break;
      default:
        auto chunk = parsed->fTagged.fChunk;
        cout << "unknown structure type: " << (int)p.fType << " at chunk (" << chunk.fX << ", " << chunk.fZ << "), block (" << (chunk.fX * 16) << ", " << (chunk.fZ * 16) << ")" << endl;
        break;
      }
    }
  }
}

static void Keys(leveldb::DB &db, unordered_set<string> &buffer) {
  unique_ptr<leveldb::Iterator> itr(db.NewIterator({}));
  for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
    buffer.insert(itr->key().ToString());
  }
}

static void Stronghold() {
  using namespace leveldb;
  using namespace mcfile;
  using namespace mcfile::be;

  fs::path pathB("C:/Users/kbinani/Documents/Projects/je2be-gui/1.18stronghold.after");
  fs::path pathA("C:/Users/kbinani/Documents/Projects/je2be-gui/1.18stronghold.before");
  unique_ptr<DB> b(OpenF(pathB / "db"));
  unique_ptr<DB> a(OpenF(pathA / "db"));

  unordered_set<string> keysA;
  unordered_set<string> keysB;
  Keys(*a, keysA);
  Keys(*b, keysB);

  unordered_set<string> common;
  for (string s : keysA) {
    if (keysB.find(s) == keysB.end()) {
      auto p = DbKey::Parse(s);
      cout << "new: " << p->toString() << endl;
    } else {
      common.insert(s);
    }
  }

  auto levelAStream = make_shared<mcfile::stream::GzFileInputStream>(pathA / "level.dat");
  levelAStream->seek(8);
  auto levelA = CompoundTag::Read(levelAStream, std::endian::little);

  auto levelBStream = make_shared<mcfile::stream::GzFileInputStream>(pathB / "level.dat");
  levelBStream->seek(8);
  auto levelB = CompoundTag::Read(levelBStream, std::endian::little);

  DiffCompoundTag(*levelB, *levelA);
}

static void MonumentBedrock() {
  using namespace std;
  using namespace leveldb;
  using namespace mcfile;
  using namespace mcfile::be;
  using namespace je2be;
  using namespace je2be::tobe;

  unique_ptr<DB> db(Open("1.18be"));
  unique_ptr<Iterator> itr(db->NewIterator({}));
  vector<Volume> volumes;
  for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
    auto key = itr->key();
    auto parsed = DbKey::Parse(key.ToString());
    if (!parsed->fIsTagged) {
      continue;
    }
    if (parsed->fTagged.fTag != static_cast<uint8_t>(DbKey::Tag::StructureBounds)) {
      continue;
    }
    string value;
    if (auto st = db->Get({}, key, &value); !st.ok()) {
      continue;
    }
    vector<StructurePiece> buffer;
    StructurePiece::Parse(itr->value().ToString(), buffer);
    for (auto const &it : buffer) {
      if (it.fType != StructureType::Monument) {
        continue;
      }
      Volume v = it.fVolume;
      volumes.push_back(v);
    }
  }
  Volume::Connect(volumes);
  unordered_map<Pos3i, string, Pos3iHasher> blocks;
  unordered_set<string> ignore = {
      "water",
      "kelp",
      "seagrass",
      "dirt",
      "flowing_water",
      "sand",
      "gravel",
      "stone",
      "copper_ore",
      "coal_ore",
      "iron_ore",
      "lapis_ore",
      "sandstone",
      "packed_ice",
      "sponge",
      "ice",
      "blue_ice",
      "gold_block",
  };
  unordered_set<string> target = {
      "prismarine",
      "seaLantern",
  };
  for (Volume const &v : volumes) {
    cout << "=====" << endl;
    Pos2i start(v.fStart.fX, v.fStart.fZ);
    Pos2i end(v.fEnd.fX, v.fEnd.fZ);
    cout << "[" << start.fX << ", " << start.fZ << "] - [" << end.fX << ", " << end.fZ << "]" << endl;

    int x0 = start.fX;
    int z0 = start.fZ;
    int y0 = v.fStart.fY;
    int cx = Coordinate::ChunkFromBlock(x0);
    int cz = Coordinate::ChunkFromBlock(z0);
    toje::ChunkCache<5, 5> cache(Dimension::Overworld, cx, cz, db.get(), std::endian::little);

    string facing;
    if (start.fX == 8891 && start.fZ == 13979) {
      facing = "north"; // OK
    } else if (start.fX == 9435 && start.fZ == 8315) {
      facing = "east"; // OK
    } else if (start.fX == 9275 && start.fZ == 7259) {
      facing = "south"; // OK
    } else if (start.fX == 9195 && start.fZ == 6827) {
      facing = "west"; // OK
    } else if (start == Pos2i(2715, -3349)) {
      facing = "south"; // OK
    } else if (start == Pos2i(763, -629)) {
      facing = "west"; // OK
    } else if (start == Pos2i(747, -341)) {
      // broken
      continue;
    } else if (start == Pos2i(3227, 3563)) {
      facing = "south"; // OK
    } else if (start == Pos2i(9339, 5419)) {
      facing = "east"; // OK
    } else if (start == Pos2i(9771, 5819)) {
      facing = "south"; // OK
    } else if (start == Pos2i(10491, 6651)) {
      facing = "east"; // OK
    } else if (start == Pos2i(8891, 13979)) {
      facing = "north"; // OK
    } else if (start == Pos2i(9275, 7259)) {
      facing = "south"; // OK
    } else if (start == Pos2i(9387, 7819)) {
      facing = "north"; // OK
    } else if (start == Pos2i(9435, 8315)) {
      facing = "east"; // OK
    } else if (start == Pos2i(8747, 9003)) {
      facing = "north"; // OK
    } else if (start == Pos2i(8939, 9307)) {
      facing = "north"; // OK
    } else if (start == Pos2i(9195, 6827)) {
      facing = "west"; // OK
    } else if (start == Pos2i(8491, 8411)) {
      facing = "north"; // OK
    } else if (start == Pos2i(8363, 8859)) {
      facing = "east"; //
    } else {
      cout << "unknown facing monument at (" << start.fX << ", " << start.fZ << ")" << endl;
    }
    if (facing.empty()) {
      continue;
    }
    auto &b = blocks;
    for (int y = v.fStart.fY; y <= v.fEnd.fY; y++) {
      for (int z = z0; z <= end.fZ; z++) {
        for (int x = x0; x <= end.fX; x++) {
          auto bl = cache.blockAt(x, y, z);
          if (!bl) {
            cerr << "cannot get block at [" << x << ", " << y << ", " << z << "]" << endl;
            return;
          }
          string name = bl->fName.substr(10);
          Pos3i local(x - x0, y - y0, z - z0);
          Pos2i radius(29, 29);
          Pos2i xz(local.fX, local.fZ);
          xz = xz - radius;
          int l90 = 0;
          int dx = 0;
          int dz = 0;
          if (facing == "east") {
            l90 = 1;
            dz = -1;
          } else if (facing == "south") {
            l90 = 2;
            dx = -1;
            dz = -1;
          } else if (facing == "west") {
            l90 = 3;
            dx = -1;
          }
          for (int k = 0; k < l90; k++) {
            xz = Left90(xz);
          }
          xz = xz + radius;
          local = Pos3i(xz.fX + dx, local.fY, xz.fZ + dz);
          CHECK(0 <= local.fX);
          CHECK(local.fX < 58);
          CHECK(0 <= local.fY);
          CHECK(local.fY < 23);
          CHECK(0 <= local.fZ);
          CHECK(local.fZ < 58);

          auto found = b.find(local);
          if (ignore.find(name) != ignore.end()) {
            if (found == b.end()) {
              b[local] = "";
            } else if (found->second != "") {
              b[local] = "";
            }
            continue;
          }
          if (target.find(name) != target.end()) {
            if (found == b.end()) {
              b[local] = name;
            } else if (found->second != name) {
              b[local] = "";
            }
            continue;
          }
          cout << "unknown: " << name << " at (" << x << ", " << y << ", " << z << ")" << endl;
          return;
        }
      }
    }
  }
#if 1
  for (auto const &it : blocks) {
    Pos3i p = it.first;
    string block = it.second;
    if (block.empty()) {
      continue;
    }
    cout << "blocks[{" << p.fX << ", " << p.fY << ", " << p.fZ << "}] = \"" << block << "\";" << endl;
  }
#endif
}

static void Box360Chunk() {
  using namespace je2be::box360;
  fs::path dir("C:/Users/kbinani/Documents/Projects/je2be-gui/00000001");
  fs::path output("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/_b2j-out");

  string name = "abc-Save20220303092528.bin";
  // string name = "2-Save20220306005722-062-gyazo-84792baa4e5d4ea30ada9746c10840be.bin";
  auto temp = File::CreateTempDir(fs::temp_directory_path());
  CHECK(temp);
  defer {
    Fs::Delete(*temp);
  };
  auto savegame = *temp / "savegame.dat";
  CHECK(Savegame::ExtractSavagameFromSaveBin(dir / name, savegame));
  vector<uint8_t> buffer;
  CHECK(Savegame::DecompressSavegame(savegame, buffer));
  CHECK(Savegame::ExtractFilesFromDecompressedSavegame(buffer, *temp));
  static std::unordered_map<mcfile::Dimension, fs::path> const sPathToRegion = {
      {mcfile::Dimension::Overworld, fs::path("region")},
      {mcfile::Dimension::Nether, fs::path("DIM-1") / "region"},
      {mcfile::Dimension::End, fs::path("DIM1") / "region"},
  };
  for (auto dimension : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
    auto pathToRegion = sPathToRegion.at(dimension);
    for (int rz = -1; rz <= 0; rz++) {
      for (int rx = -1; rx <= 0; rx++) {
        auto mcr = *temp / pathToRegion / ("r." + to_string(rx) + "." + to_string(rz) + ".mcr");
        if (!Fs::Exists(mcr)) {
          continue;
        }

        CHECK(Fs::CreateDirectories(output / pathToRegion));
        auto mca = output / pathToRegion / mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
        CHECK(Region::Convert(dimension, mcr, rx, rz, mca, *temp));
      }
    }
  }
}

#if 1
TEST_CASE("research") {
  Box360Chunk();
}
#endif
