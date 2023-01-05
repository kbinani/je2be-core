#pragma once

static optional<wstring> GetSpecialDirectory(int csidType) {
#if __has_include(<shlobj_core.h>)
  wchar_t path[MAX_PATH + 256];

  if (SHGetSpecialFolderPathW(nullptr, path, csidType, FALSE)) {
    return wstring(path);
  }
#endif
  return nullopt;
}

static optional<wstring> GetLocalApplicationDirectory() {
#if __has_include(<shlobj_core.h>)
  return GetSpecialDirectory(CSIDL_LOCAL_APPDATA);
#else
  return nullopt;
#endif
}

static optional<wstring> GetApplicationDirectory() {
#if __has_include(<shlobj_core.h>)
  return GetSpecialDirectory(CSIDL_APPDATA);
#else
  return nullopt;
#endif
}

static optional<fs::path> BedrockWorldDirectory(string const &name) {
  auto appDir = GetLocalApplicationDirectory(); // X:/Users/whoami/AppData/Local
  if (!appDir) {
    return nullopt;
  }
  return fs::path(*appDir) / L"Packages" / L"Microsoft.MinecraftUWP_8wekyb3d8bbwe" / L"LocalState" / L"games" / L"com.mojang" / L"minecraftWorlds" / name / L"db";
}

static optional<fs::path> BedrockPreviewWorldDirectory(string const &name) {
  auto appDir = GetLocalApplicationDirectory(); // X:/Users/whoami/AppData/Local
  if (!appDir) {
    return nullopt;
  }
  return fs::path(*appDir) / L"Packages" / L"Microsoft.MinecraftWindowsBeta_8wekyb3d8bbwe" / L"LocalState" / L"games" / L"com.mojang" / L"minecraftWorlds" / name / L"db";
}

static optional<fs::path> JavaSavesDirectory() {
  if (auto appData = GetApplicationDirectory(); appData) {
    return fs::path(*appData) / ".minecraft" / "saves";
  } else {
    return nullopt;
  }
}

static leveldb::DB *OpenF(fs::path p) {
  using namespace leveldb;
  Options o;
  o.compression = kZlibRawCompression;
  DB *db;
  leveldb::Status st = DB::Open(o, p, &db);
  if (!st.ok()) {
    return nullptr;
  }
  return db;
}

static leveldb::DB *Open(string const &name) {
  using namespace leveldb;
  auto dir = BedrockWorldDirectory(name);
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
  code << "static bool IsFenceAlwaysConnectable(mcfile::blocks::BlockId id) {" << endl;
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
  code << "static bool IsGlassPaneOrIronBarsAlwaysConnectable(mcfile::blocks::BlockId id) {" << endl;
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
  using namespace mcfile::blocks::minecraft;
  map<string, Pos3i> uniq;
  for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
    switch (id) {
    case cave_vines_plant:
    case chorus_flower:
    case chorus_plant:
    case soul_fire:
    case sugar_cane:
    case kelp_plant:
    case twisting_vines_plant:
    case weeping_vines_plant:
      continue;
    }
    string name = mcfile::blocks::Name(id);
    uniq[name] = {0, 0, 0};
  }

  int const x0 = -42;
  int const z0 = 165;
  int const y = 4;
  int x = x0;
  int x1 = x0;
  fs::path root("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/labo");
  {
    ofstream os((root / "datapacks" / "kbinani" / "data" / "je2be" / "functions" / "research_note_block.mcfunction").string());
    for (auto const &it : uniq) {
      string name = it.first;
      if (name.ends_with("_door")) {
        os << "setblock " << x << " " << (y - 1) << " " << z0 << " " << name << "[half=upper]" << endl;
        os << "setblock " << x << " " << (y - 2) << " " << z0 << " " << name << "[half=lower]" << endl;
      } else {
        os << "setblock " << x << " " << (y - 1) << " " << z0 << " " << name << endl;
      }
      os << "setblock " << x << " " << y << " " << z0 << " note_block" << endl;
      uniq[name] = {x, y - 1, z0};
      x += 2;
    }
    x1 = x;
  }
  {
    ofstream os((root / "datapacks" / "kbinani" / "data" / "je2be" / "functions" / "research_note_block_cleanup.mcfunction").string());
    os << "fill " << x0 << " " << y << " " << z0 << " " << x1 << " " << y << " " << z0 << " air " << endl;
    os << "fill " << x0 << " " << (y - 2) << " " << z0 << " " << x1 << " " << (y - 2) << " " << z0 << " dirt" << endl;
    os << "fill " << x0 << " " << (y - 1) << " " << z0 << " " << x1 << " " << (y - 1) << " " << z0 << " grass_block" << endl;
  }

  // login the game, then execute /function je2be:research_note_block

  mcfile::je::World w(root);
  shared_ptr<mcfile::je::Chunk> chunk;
  map<string, set<string>> instruments;
  set<string> error;
  for (auto const &it : uniq) {
    string expected = it.first;
    Pos3i pos = it.second;
    int cx = mcfile::Coordinate::ChunkFromBlock(pos.fX);
    int cz = mcfile::Coordinate::ChunkFromBlock(pos.fZ);
    if (!chunk || (chunk && chunk->fChunkX != cx)) {
      chunk = w.chunkAt(cx, cz);
    }
    auto center = chunk->blockAt(pos.fX, pos.fY, pos.fZ);
    if (expected != center->fName) {
      error.insert(expected);
    } else {
      auto noteBlock = chunk->blockAt(pos.fX, pos.fY + 1, pos.fZ);
      auto instrument = noteBlock->property("instrument", "");
      if (instrument.empty()) {
        cerr << "empty instrument: [" << x << ", " << z0 << "]" << endl;
      } else {
        instruments[string(instrument)].insert(string(center->fName));
      }
    }
  }

  instruments.erase("harp");

  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());
  code << "static std::string NoteBlockInstrumentAutogenCode(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  auto static const bedrock = mcfile::blocks::minecraft::bedrock;" << endl;
  code << "  switch (id) {" << endl;
  for (auto const &it : instruments) {
    string instrument = it.first;
    set<string> const &blocks = it.second;
    for (string const &block : blocks) {
      code << "  case " << block.substr(10) << ":" << endl;
    }
    code << "    return \"" + instrument << "\";" << endl;
  }
  code << "  default:" << endl;
  code << "    return \"harp\";" << endl;
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
  auto levelA = CompoundTag::Read(levelAStream, Endian::Little);

  auto levelBStream = make_shared<mcfile::stream::GzFileInputStream>(pathB / "level.dat");
  levelBStream->seek(8);
  auto levelB = CompoundTag::Read(levelBStream, Endian::Little);

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
  WrapDb wdb(db.get());
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
    terraform::bedrock::BlockAccessorBedrock<5, 5> cache(Dimension::Overworld, cx, cz, &wdb, Endian::Little);

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
  // string name = "2-Save20220306005722-077-gyazo-1fcbd51efa6d0795fe34e912619aa4f5.bin";
  Options options;
  CHECK(Converter::Run(dir / name, output, thread::hardware_concurrency(), options).ok());
}

static void WallConnectable() {
  set<string> uniq;
  for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
    switch (id) {
    case mcfile::blocks::minecraft::reinforced_deepslate:
      continue;
    }
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
    if (name.find("sculk") != string::npos && id != mcfile::blocks::minecraft::sculk_sensor) {
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
      os << "setblock " << x << " " << (y + 1) << " " << (z0 + 1) << " " << name << endl;
      x++;
    }
    x1 = x;
    os << "fill " << x0 << " " << y << " " << (z0 + 1) << " " << x1 << " " << y << " " << (z0 + 1) << " cobblestone_wall" << endl;
    os << "fill " << x0 << " " << y << " " << (z0 + 2) << " " << x1 << " " << y << " " << (z0 + 2) << " cobblestone_wall" << endl;
  }

  // login the game, then execute /function je2be:place_blocks

  mcfile::je::World w(root);
  shared_ptr<mcfile::je::Chunk> chunk;
  int cz = mcfile::Coordinate::ChunkFromBlock(z0);
  set<string> wallAttachable;
  set<string> tall;
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
      auto wall = chunk->blockAt(x, y, z0 + 1);
      auto wallAttached = wall->property("north") != "none";
      if (wallAttached) {
        wallAttachable.insert(expected);
      }
      if (wall->property("south") == "tall") {
        if (!expected.ends_with("slab") && !expected.ends_with("stairs") && !expected.ends_with("trapdoor")) {
          tall.insert(expected);
        }
      }
    }
  }

  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());

  code << "static bool IsWallAlwaysConnectable(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks;" << endl;
  code << "  switch (id) {" << endl;
  for (auto n : wallAttachable) {
    code << "    case minecraft::" << n.substr(10) << ":" << endl;
  }
  code << "      return true;" << endl;
  code << "    default:" << endl;
  code << "      return false;" << endl;
  code << "  }" << endl;
  code << "}" << endl;
  code << endl;

  code << "static bool IsBlockAlwaysMakeWallTallShape(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks;" << endl;
  code << "  switch (id) {" << endl;
  for (auto n : tall) {
    code << "    case minecraft::" << n.substr(10) << ":" << endl;
  }
  code << "      return true;" << endl;
  code << "    default:" << endl;
  code << "      return false;" << endl;
  code << "  }" << endl;
  code << "}" << endl;
  code << endl;
}

static void PistonArm() {
  {
    auto db = OpenF(*BedrockPreviewWorldDirectory("1.19piston_arm"));
    auto chunk = mcfile::be::Chunk::Load(0, 0, mcfile::Dimension::Overworld, db, mcfile::Endian::Little);
    auto root = Compound();
    for (int y = 0; y < 4; y++) {
      auto t = Compound();

      auto block = chunk->blockAt(0, y, 0);
      t->set("block", block->toCompoundTag());

      auto tile = chunk->blockEntityAt(0, y, 0);
      if (tile) {
        t->set("tile", tile->clone());
      }

      root->set(to_string(y), t);
    }
    ostringstream ss;
    mcfile::nbt::PrintAsJson(ss, *root, {.fTypeHint = true});
    auto s = ss.str();
    int64_t digest = XXHash::Digest(s.c_str(), s.size());
    fs::path out = fs::path("1.19piston_arm_bedrock_" + to_string(digest) + ".json");
    if (fs::exists(out)) {
      cout << out << " already exists" << endl;
    } else {
      ofstream fout(out.string().c_str());
      fout << s;
      cout << out << " written" << endl;
    }
  }
  {
    auto world = mcfile::je::World(*JavaSavesDirectory() / "1.19piston_arm");
    auto chunk = world.chunkAt(0, 0);
    auto root = Compound();
    for (int y = 0; y < 4; y++) {
      auto t = Compound();

      auto block = chunk->blockAt(0, y, 0);
      t->set("block", block->toCompoundTag());

      auto tile = chunk->tileEntityAt(0, y, 0);
      if (tile) {
        t->set("tile", tile->clone());
      }

      root->set(to_string(y), t);
    }
    ostringstream ss;
    mcfile::nbt::PrintAsJson(ss, *root, {.fTypeHint = true});
    auto s = ss.str();
    int64_t digest = XXHash::Digest(s.c_str(), s.size());
    fs::path out = fs::path("1.19piston_arm_java_" + to_string(digest) + ".json");
    if (fs::exists(out)) {
      cout << out << " already exists" << endl;
    } else {
      ofstream fout(out.string().c_str());
      fout << s;
      cout << out << " written" << endl;
    }
  }
}

struct LevelDbConcurrentIterationResult {
  char fPrefix;
  std::shared_ptr<leveldb::Iterator> fItr;
  bool fContinue;
  bool fValid;
  std::string fKey;
  std::string fValue;
};

static LevelDbConcurrentIterationResult LevelDbConcurrentIterationQueue(char prefix, std::shared_ptr<leveldb::Iterator> itr) {
  LevelDbConcurrentIterationResult ret;
  ret.fPrefix = prefix;
  ret.fItr = itr;

  if (itr->Valid()) {
    std::string key = itr->key().ToString();
    ret.fValid = prefix == key[0];
    if (prefix == key[0]) {
      ret.fContinue = true;
      ret.fKey = key;
      ret.fValue = itr->value().ToString();
      itr->Next();
    } else {
      ret.fContinue = false;
    }
  } else {
    ret.fContinue = false;
    ret.fValid = false;
  }
  return ret;
}

static void LevelDbConcurrentIteration() {
  using namespace leveldb;

  unique_ptr<DB> db(Open("1.19"));
  REQUIRE(db);
  set<string> keysExpected;
  unique_ptr<Iterator> itr(db->NewIterator({}));
  for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
    keysExpected.insert(itr->key().ToString());
  }
  unique_ptr<BS::thread_pool> queue(new BS::thread_pool(thread::hardware_concurrency()));
  deque<future<LevelDbConcurrentIterationResult>> futures;
  for (int i = 0; i < 256; i++) {
    shared_ptr<Iterator> itr(db->NewIterator({}));
    string prefix;
    prefix.append(1, (char)i);
    itr->Seek(Slice(prefix));
    futures.push_back(queue->submit(LevelDbConcurrentIterationQueue, (char)i, itr));
  }
  set<string> keysActual;
  while (!futures.empty()) {
    vector<future<LevelDbConcurrentIterationResult>> drain;
    FutureSupport::Drain(thread::hardware_concurrency() + 1, futures, drain);
    for (auto &f : drain) {
      LevelDbConcurrentIterationResult ret = f.get();
      if (ret.fValid) {
        keysActual.insert(ret.fKey);
      }
      if (ret.fContinue) {
        futures.push_back(queue->submit(LevelDbConcurrentIterationQueue, ret.fPrefix, ret.fItr));
      }
    }
  }
  CHECK(keysExpected.size() == keysActual.size());
  for (auto const &e : keysExpected) {
    auto found = keysActual.find(e);
    if (found == keysActual.end()) {
      cerr << "\"" << e << "\" not found; e[0]=" << (int)(0xff & e[0]) << endl;
    }
    CHECK(found != keysActual.end());
  }
}

#if 0
TEST_CASE("research") {
  LevelDbConcurrentIteration();
}
#endif
