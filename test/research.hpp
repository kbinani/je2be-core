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
  set<u8string> uniq;
  for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
    u8string name = mcfile::blocks::Name(id, kJavaDataVersion);
    if (name.ends_with(u8"_stairs")) {
      continue;
    }
    if (name.ends_with(u8"piston")) {
      continue;
    }
    if (name.ends_with(u8"door")) {
      continue;
    }
    uniq.insert(name);
  }
  vector<u8string> names(uniq.begin(), uniq.end());

  int const x0 = -42;
  int const z0 = 165;
  int const y = -60;
  int x = x0;
  int x1 = x0;
  fs::path root("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/FenceGlassPaneIronBarsConnectabl");
  Fs::CreateDirectories(root / "datapacks" / "kbinani" / "data" / "je2be" / "function");
  {
    ofstream os((root / "datapacks" / "kbinani" / "pack.mcmeta").string());
    nlohmann::json mcmeta;
    nlohmann::json pack;
    pack["pack_format"] = 57;
    pack["description"] = "datapack";
    mcmeta["pack"] = pack;
    os << nlohmann::to_string(mcmeta);
  }
  {
    ofstream os((root / "datapacks" / "kbinani" / "data" / "je2be" / "function" / "place_blocks.mcfunction").string());
    for (u8string const &name : names) {
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
  set<u8string> glassPaneAttachable;
  set<u8string> fenceAttachable;
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
      auto fenceAttached = fence->property(u8"south") == u8"true";
      if (fenceAttached) {
        fenceAttachable.insert(expected);
      }

      auto glassPane = chunk->blockAt(x, y, z0 + 1);
      auto glassPaneAttached = glassPane->property(u8"north") == u8"true";
      if (glassPaneAttached) {
        glassPaneAttachable.insert(expected);
      }
    }
  }

  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());
  code << "static bool IsFenceAlwaysConnectable(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  switch (id) {" << endl;
  for (auto n : fenceAttachable) {
    code << "    case " << n.substr(10) << ":" << endl;
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
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  switch (id) {" << endl;
  for (auto n : glassPaneAttachable) {
    code << "    case " << n.substr(10) << ":" << endl;
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
  map<u8string, Pos3i> uniq;
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
    u8string name = mcfile::blocks::Name(id, kJavaDataVersion);
    uniq[name] = {0, 0, 0};
  }

  int const x0 = 0;
  int const z0 = 0;
  int const y = -60;
  fs::path root("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/NoteBlock");
  Fs::CreateDirectories(root / "datapacks" / "kbinani" / "data" / "je2be" / "function");
  {
    ofstream os((root / "datapacks" / "kbinani" / "pack.mcmeta").string());
    nlohmann::json mcmeta;
    nlohmann::json pack;
    pack["pack_format"] = 57;
    pack["description"] = "datapack";
    mcmeta["pack"] = pack;
    os << nlohmann::to_string(mcmeta);
  }
  {
    ofstream os((root / "datapacks" / "kbinani" / "data" / "je2be" / "function" / "research_note_block.mcfunction").string());
    int width = (int)ceil(sqrt(uniq.size()));
    int i = 0;
    int j = 0;
    for (auto const &it : uniq) {
      int x = x0 + 2 * i;
      int z = z0 + 2 * j;
      u8string name = it.first;
      if (name.ends_with(u8"_door")) {
        os << "setblock " << x << " " << (y - 1) << " " << z << " " << name << "[half=upper]" << endl;
        os << "setblock " << x << " " << (y - 2) << " " << z << " " << name << "[half=lower]" << endl;
      } else {
        os << "setblock " << x << " " << (y - 1) << " " << z << " " << name << endl;
      }
      os << "setblock " << x << " " << y << " " << z << " note_block" << endl;
      uniq[name] = {x, y - 1, z};
      i++;
      if (i > width) {
        i = 0;
        j++;
      }
    }
  }

  // login the game, then execute /function je2be:research_note_block

  mcfile::je::World w(root);
  shared_ptr<mcfile::je::Chunk> chunk;
  map<u8string, set<u8string>> instruments;
  set<u8string> error;
  for (auto const &it : uniq) {
    u8string expected = it.first;
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
      auto instrument = noteBlock->property(u8"instrument", u8"");
      if (instrument.empty()) {
        cerr << "empty instrument: [" << pos.fX << ", " << pos.fZ << "]" << endl;
      } else {
        instruments[u8string(instrument)].insert(u8string(center->fName));
      }
    }
  }

  instruments.erase(u8"harp");

  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());
  code << "static std::u8string NoteBlockInstrumentAutogenCode(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  auto static const bedrock = mcfile::blocks::minecraft::bedrock;" << endl;
  code << "  switch (id) {" << endl;
  for (auto const &it : instruments) {
    u8string instrument = it.first;
    set<u8string> const &blocks = it.second;
    for (u8string const &block : blocks) {
      code << "  case " << block.substr(10) << ":" << endl;
    }
    code << u8"    return u8\"" + instrument << u8"\";" << endl;
  }
  code << "  default:" << endl;
  code << "    return u8\"harp\";" << endl;
  code << "  }" << endl;
  code << "}" << endl;
  code << endl;
}

static void RedstoneWire() {
  set<u8string> uniq;
  for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
    u8string name = mcfile::blocks::Name(id, kJavaDataVersion);
    if (name.ends_with(u8"slab")) {
      continue;
    }
    if (name.ends_with(u8"rail")) {
      continue;
    }
    uniq.insert(name);
  }
  vector<u8string> names(uniq.begin(), uniq.end());

  int const x0 = -42;
  int const z0 = 165;
  int const y = -60;
  int x = x0;
  int x1 = x0;
  fs::path root("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/RedstoneWire");
  Fs::CreateDirectories(root / "datapacks" / "kbinani" / "data" / "je2be" / "function");
  {
    ofstream os((root / "datapacks" / "kbinani" / "pack.mcmeta").string());
    nlohmann::json mcmeta;
    nlohmann::json pack;
    pack["pack_format"] = 57;
    pack["description"] = "datapack";
    mcmeta["pack"] = pack;
    os << nlohmann::to_string(mcmeta);
  }
  {
    ofstream os((root / "datapacks" / "kbinani" / "data" / "je2be" / "function" / "research_redstone_wire.mcfunction").string());
    os << "fill " << x0 << " " << y << " " << (z0 - 1) << " " << (x0 + 2 * names.size()) << " " << (y + 2) << " " << (z0 + 1) << " air" << endl;
    for (u8string const &name : names) {
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
  set<u8string> negative;
  set<u8string> positive;
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
      if (mcfile::blocks::IsTransparent(center->fId)) {
        auto wire = chunk->blockAt(x, y - 2, z0);
        auto north = wire->property(u8"north", u8"");
        auto south = wire->property(u8"south", u8"");
        if (north != u8"up" || south != u8"up") {
          negative.insert(expected);
        }
      } else {
        auto wire = chunk->blockAt(x, y - 2, z0);
        auto north = wire->property(u8"north", u8"");
        auto south = wire->property(u8"south", u8"");
        if (north == u8"up" && south == u8"up") {
          positive.insert(expected);
        }
      }
    }
  }

  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());
  code << "static bool IsAlwaysTransparentAgainstRedstoneWire(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  switch (id) {" << endl;
  for (auto const &it : positive) {
    code << "  case " << it.substr(10) << ":" << endl;
  }
  code << "      // mcfile::blocks::IsTransparent(id) == false, but transparent against redstone wire" << endl;
  code << "      return true;" << endl;
  for (auto const &it : negative) {
    code << "  case " << it.substr(10) << ":" << endl;
  }
  code << "      // mcfile::blocks::IsTransparent(id) == true, but not transparent against redstone wire" << endl;
  code << "      return false;" << endl;
  code << "  }" << endl;
  code << "  return mcfile::blocks::IsTransparent(id);" << endl;
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
    if (!parsed.fIsTagged) {
      continue;
    }
    uint8_t tag = parsed.fTagged.fTag;
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
        auto chunk = parsed.fTagged.fChunk;
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
      cout << "new: " << p.toString() << endl;
    } else {
      common.insert(s);
    }
  }

  auto levelAStream = make_shared<mcfile::stream::GzFileInputStream>(pathA / "level.dat");
  levelAStream->seek(8);
  auto levelA = CompoundTag::Read(levelAStream, Encoding::LittleEndian);

  auto levelBStream = make_shared<mcfile::stream::GzFileInputStream>(pathB / "level.dat");
  levelBStream->seek(8);
  auto levelB = CompoundTag::Read(levelBStream, Encoding::LittleEndian);

  DiffCompoundTag(*levelB, *levelA);
}

static void MonumentBedrock() {
  using namespace std;
  using namespace leveldb;
  using namespace mcfile;
  using namespace mcfile::be;
  using namespace je2be;
  using namespace je2be::java;

  unique_ptr<DB> db(Open("1.18be"));
  WrapDb wdb(db.get());
  unique_ptr<Iterator> itr(db->NewIterator({}));
  vector<Volume> volumes;
  for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
    auto key = itr->key();
    auto parsed = DbKey::Parse(key.ToString());
    if (!parsed.fIsTagged) {
      continue;
    }
    if (parsed.fTagged.fTag != static_cast<uint8_t>(DbKey::Tag::StructureBounds)) {
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
  unordered_map<Pos3i, u8string, Pos3iHasher> blocks;
  unordered_set<u8string> ignore = {
      u8"water",
      u8"kelp",
      u8"seagrass",
      u8"dirt",
      u8"flowing_water",
      u8"sand",
      u8"gravel",
      u8"stone",
      u8"copper_ore",
      u8"coal_ore",
      u8"iron_ore",
      u8"lapis_ore",
      u8"sandstone",
      u8"packed_ice",
      u8"sponge",
      u8"ice",
      u8"blue_ice",
      u8"gold_block",
  };
  unordered_set<u8string> target = {
      u8"prismarine",
      u8"seaLantern",
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
    terraform::bedrock::BlockAccessorBedrock<5, 5> cache(Dimension::Overworld, cx, cz, &wdb, Encoding::LittleEndian);

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
          u8string name = bl->fName.substr(10);
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
              b[local] = u8"";
            } else if (found->second != u8"") {
              b[local] = u8"";
            }
            continue;
          }
          if (target.find(name) != target.end()) {
            if (found == b.end()) {
              b[local] = name;
            } else if (found->second != name) {
              b[local] = u8"";
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
    u8string block = it.second;
    if (block.empty()) {
      continue;
    }
    cout << "blocks[{" << p.fX << ", " << p.fY << ", " << p.fZ << "}] = \"" << block << "\";" << endl;
  }
#endif
}

static void Box360Chunk() {
  fs::path dir("C:/Users/kbinani/Documents/Projects/je2be-gui/00000001");
  fs::path output("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/_b2j-out");

  string name = "abc-Save20220303092528.bin";
  // string name = "2-Save20220306005722-077-gyazo-1fcbd51efa6d0795fe34e912619aa4f5.bin";
  je2be::lce::Options options;
  CHECK(je2be::xbox360::Converter::Run(dir / name, output, thread::hardware_concurrency(), options).ok());
}

static void WallConnectable() {
  set<u8string> uniq;
  for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
    switch (id) {
    case mcfile::blocks::minecraft::reinforced_deepslate:
      continue;
    }
    u8string name = mcfile::blocks::Name(id, kJavaDataVersion);
    if (name.ends_with(u8"_stairs")) {
      continue;
    }
    if (name.ends_with(u8"piston")) {
      continue;
    }
    if (name.ends_with(u8"door")) {
      continue;
    }
    if (name.find(u8"sculk") != string::npos && id != mcfile::blocks::minecraft::sculk_sensor) {
      continue;
    }
    uniq.insert(name);
  }
  vector<u8string> names(uniq.begin(), uniq.end());

  int const x0 = -42;
  int const z0 = 165;
  int const y = -60;
  int x = x0;
  int x1 = x0;
  fs::path root("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/WallConnectable");
  Fs::CreateDirectories(root / "datapacks" / "kbinani" / "data" / "je2be" / "function");
  {
    ofstream os((root / "datapacks" / "kbinani" / "pack.mcmeta").string());
    nlohmann::json mcmeta;
    nlohmann::json pack;
    pack["pack_format"] = 57;
    pack["description"] = "datapack";
    mcmeta["pack"] = pack;
    os << nlohmann::to_string(mcmeta);
  }
  {
    ofstream os((root / "datapacks" / "kbinani" / "data" / "je2be" / "function" / "place_blocks.mcfunction").string());
    for (u8string const &name : names) {
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
  set<u8string> wallAttachable;
  set<u8string> tall;
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
      auto wallAttached = wall->property(u8"north") != u8"none";
      if (wallAttached) {
        wallAttachable.insert(expected);
      }
      if (wall->property(u8"south") == u8"tall") {
        if (!expected.ends_with(u8"slab") && !expected.ends_with(u8"stairs") && !expected.ends_with(u8"trapdoor")) {
          tall.insert(expected);
        }
      }
    }
  }

  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());

  code << "static bool IsWallAlwaysConnectable(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  switch (id) {" << endl;
  for (auto n : wallAttachable) {
    code << "    case " << n.substr(10) << ":" << endl;
  }
  code << "      return true;" << endl;
  code << "    default:" << endl;
  code << "      return false;" << endl;
  code << "  }" << endl;
  code << "}" << endl;
  code << endl;

  code << "static bool IsBlockAlwaysMakeWallTallShape(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  switch (id) {" << endl;
  for (auto n : tall) {
    code << "    case " << n.substr(10) << ":" << endl;
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
    auto chunk = mcfile::be::Chunk::Load(0, 0, mcfile::Dimension::Overworld, db, mcfile::Encoding::LittleEndian);
    auto root = Compound();
    for (int y = 0; y < 4; y++) {
      auto t = Compound();

      auto block = chunk->blockAt(0, y, 0);
      t->set(u8"block", block->toCompoundTag());

      auto tile = chunk->blockEntityAt(0, y, 0);
      if (tile) {
        t->set(u8"tile", tile->clone());
      }

      root->set(mcfile::String::ToString(y), t);
    }
    ostringstream ss;
    mcfile::nbt::PrintAsJson(ss, *root, {.fTypeHint = true});
    auto s = ss.str();
    int64_t digest = XXHash<int64_t>::Digest(s.c_str(), s.size());
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
      t->set(u8"block", block->toCompoundTag());

      auto tile = chunk->tileEntityAt(0, y, 0);
      if (tile) {
        t->set(u8"tile", tile->clone());
      }

      root->set(mcfile::String::ToString(y), t);
    }
    ostringstream ss;
    mcfile::nbt::PrintAsJson(ss, *root, {.fTypeHint = true});
    auto s = ss.str();
    int64_t digest = XXHash<int64_t>::Digest(s.c_str(), s.size());
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

static void LightTransmission1() {
  int const x0 = 1;
  int const y0 = -60;
  int const z0 = -36;
  fs::path root("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/lighting");
  {
    fs::path name = root / "datapacks" / "kbinani" / "pack.mcmeta";
    Fs::CreateDirectories(name.parent_path());
    ofstream os(name.string());
    os << R"({
  "pack": {
    "pack_format": 57,
    "description": "datapack"
  }
})" << endl;
  }
  vector<mcfile::blocks::BlockId> blocks;
  {
    fs::path name = root / "datapacks" / "kbinani" / "data" / "je2be" / "function" / "place_blocks.mcfunction";
    Fs::CreateDirectories(name.parent_path());
    ofstream os(name.string());
    int x = x0;
    for (mcfile::blocks::BlockId id = 1; id <= mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
      auto name = mcfile::blocks::Name(id, kJavaDataVersion);
      if (name.ends_with(u8"_stairs") || name.ends_with(u8"_slab")) {
        continue;
      }
      int y = y0;
      int z = z0;
      os << "fill " << (x - 2) << " " << (y - 1) << " " << (z - 2) << " " << (x + 1) << " " << (y + 1) << " " << (z + 2) << " tinted_glass" << endl;
      if (id == mcfile::blocks::minecraft::minecraft_max_block_id) {
        break;
      }
      os << "setblock " << (x - 1) << " " << y << " " << z << " barrier" << endl;
      os << "setblock " << x << " " << y << " " << (z - 1) << " barrier" << endl;
      os << "setblock " << (x + 1) << " " << y << " " << z << " barrier" << endl;
      os << "setblock " << x << " " << y << " " << (z + 1) << " barrier" << endl;
      os << "setblock " << x << " " << (y - 1) << " " << z << " sponge" << endl;
      os << "setblock " << x << " " << (y + 1) << " " << z << " barrier" << endl;
      os << "setblock " << x << " " << y << " " << z << " " << mcfile::blocks::Name(id, kJavaDataVersion) << endl;
      x += 4;
      blocks.push_back(id);
    }

    x = x0;
    int z = z0 - 5;
    int y = y0;
    for (auto id : blocks) {
      os << "fill " << (x - 1) << " " << (y - 1) << " " << (z - 2) << " " << x << " " << (y + 1) << " " << (z + 1) << " tinted_glass " << endl;
      os << "setblock " << x << " " << y << " " << (z - 1) << " torch" << endl;
      os << "setblock " << x << " " << y << " " << z << " " << mcfile::blocks::Name(id, kJavaDataVersion) << endl;
      os << "setblock " << x << " " << y << " " << (z - 1) << " glass" << endl;

      x += 2;
    }
    os << "fill " << (x - 1) << " " << (y0 - 1) << " " << (z - 2) << " " << x << " " << (y + 1) << " " << (z + 1) << " tinted_glass " << endl;
  }

  map<int, vector<mcfile::blocks::BlockId>> lightAttenuation;

  mcfile::je::World world(root);
  shared_ptr<mcfile::je::Chunk> chunk;
  Data4b3d skyLight({0, 0, 0}, 16, 16, 16);
  Data4b3d blockLight({0, 0, 0}, 16, 16, 16);
  for (int i = 0; i < blocks.size(); i++) {
    mcfile::blocks::BlockId id = blocks[i];
    int x = x0 + i * 4;
    int y = y0;
    int z = z0;
    int cx = mcfile::Coordinate::ChunkFromBlock(x);
    int cz = mcfile::Coordinate::ChunkFromBlock(z);
    if (!(chunk && chunk->fChunkX == cx && chunk->fChunkZ == cz)) {
      chunk = world.chunkAt(cx, cz);
    }
    assert(chunk);
    auto block = chunk->blockAt(x, y, z);
    if (!block) {
      cerr << mcfile::blocks::Name(id, kJavaDataVersion) << " not set (1)" << endl;
      continue;
    }
    if (block->fId != id) {
      cerr << mcfile::blocks::Name(id, kJavaDataVersion) << " not set (2)" << endl;
      continue;
    }
    if (block->property(u8"waterlogged") == u8"true") {
      cerr << mcfile::blocks::Name(id, kJavaDataVersion) << " waterlogged is set to true" << endl;
      continue;
    }
    int cy = mcfile::Coordinate::ChunkFromBlock(y);
    mcfile::je::ChunkSection *section = nullptr;
    for (auto const &s : chunk->fSections) {
      if (s && s->y() == cy) {
        section = s.get();
        break;
      }
    }
    assert(section);
    skyLight.copyFrom(section->fSkyLight);
    uint8_t centerUp = skyLight.getUnchecked({x - chunk->minBlockX(), y + 1 - section->y() * 16, z - chunk->minBlockZ()});
    uint8_t centerLight = skyLight.getUnchecked({x - chunk->minBlockX(), y - section->y() * 16, z - chunk->minBlockZ()});
    assert(centerUp == 15);
    int diff = (int)centerUp - (int)centerLight;
    if (diff != 15) {
      lightAttenuation[diff].push_back(id);
    }
  }

  map<int, vector<mcfile::blocks::BlockId>> lightEmission;
  for (int i = 0; i < blocks.size(); i++) {
    mcfile::blocks::BlockId id = blocks[i];
    int x = x0 + i * 2;
    int y = y0;
    int z = z0 - 5;
    int cx = mcfile::Coordinate::ChunkFromBlock(x);
    int cz = mcfile::Coordinate::ChunkFromBlock(z);
    if (!(chunk && chunk->fChunkX == cx && chunk->fChunkZ == cz)) {
      chunk = world.chunkAt(cx, cz);
    }
    assert(chunk);
    auto block = chunk->blockAt(x, y, z);
    if (!block) {
      cerr << mcfile::blocks::Name(id, kJavaDataVersion) << " not set (3)" << endl;
      continue;
    }
    if (block->fId != id) {
      cerr << mcfile::blocks::Name(id, kJavaDataVersion) << " not set (4)" << endl;
      continue;
    }
    int cy = mcfile::Coordinate::ChunkFromBlock(y);
    mcfile::je::ChunkSection *section = nullptr;
    for (auto const &s : chunk->fSections) {
      if (s && s->y() == cy) {
        section = s.get();
        break;
      }
    }
    assert(section);
    blockLight.copyFrom(section->fBlockLight);
    uint8_t centerBlockLight = blockLight.getUnchecked({x - chunk->minBlockX(), y - section->y() * 16, z - chunk->minBlockZ()});
    uint8_t northBlockLight = blockLight.getUnchecked({x - chunk->minBlockX(), y - section->y() * 16, z - 1 - chunk->minBlockZ()});
    int diff = (int)centerBlockLight - (int)northBlockLight;
    if (centerBlockLight == 0) {
      assert(northBlockLight == 0);
    } else {
      assert(northBlockLight + 1 == centerBlockLight);
    }
    if (centerBlockLight > 0) {
      lightEmission[(int)centerBlockLight].push_back(id);
    }
  }

  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());
  code << "static u8 LightAttenuationAmountById(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  switch (id) {" << endl;
  for (auto const &i : lightAttenuation) {
    int diff = i.first;
    for (mcfile::blocks::BlockId id : i.second) {
      auto name = mcfile::blocks::Name(id, kJavaDataVersion).substr(10);
      code << "  case " << name << ":" << endl;
    }
    code << "    return " << diff << ";" << endl;
  }
  code << "  default:" << endl;
  code << "    return 15;" << endl;
  code << "  }" << endl;
  code << "}" << endl;
}

static void LightEmission() {
  auto dir = ProjectRootDir() / "test" / "data" / "je2be-test";
  mcfile::je::World world(dir);
  int const cz = 1;
  int const y = 64;
  map<int, set<mcfile::blocks::BlockId>> emissions;
  set<mcfile::blocks::BlockId> ids;
  for (int cx = 0; cx <= 19; cx++) {
    auto chunk = world.chunkAt(cx, cz);
    REQUIRE(chunk);
    mcfile::je::ChunkSection *section = nullptr;
    for (auto &s : chunk->fSections) {
      if (s && s->y() == y / 16) {
        section = s.get();
        break;
      }
    }
    REQUIRE(section);
    mcfile::Data4b3d blockLight({chunk->minBlockX(), section->y() * 16, chunk->minBlockZ()}, 16, 16, 16);
    blockLight.copyFrom(section->fBlockLight);
    for (int z = 1; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        Pos3i center(x + cx * 16, y, z + cz * 16);
        auto centerBlock = chunk->blockAt(center);
        if (centerBlock->fId == mcfile::blocks::minecraft::tinted_glass || centerBlock->fId == mcfile::blocks::minecraft::air) {
          continue;
        }
        Pos3i north(x + cx * 16, y, z - 1 + cz * 16);
        auto northBlock = chunk->blockAt(north);
        if (northBlock->fId == mcfile::blocks::minecraft::air || northBlock->fId == mcfile::blocks::minecraft::barrier) {
          ids.insert(centerBlock->fId);
          auto centerLight = blockLight.getUnchecked(center);
          auto northLight = blockLight.getUnchecked(north);
          if (centerLight == 0) {
            assert(northLight == 0);
          } else {
            assert(northLight + 1 == centerLight);
          }
          if (centerLight > 0) {
            emissions[centerLight].insert(centerBlock->fId);
          }
        }
      }
    }
  }
  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());
  code << "static u8 LightEmissionById(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  switch (id) {" << endl;
  for (auto const &i : emissions) {
    int emission = i.first;
    for (mcfile::blocks::BlockId id : i.second) {
      auto name = mcfile::blocks::Name(id, kJavaDataVersion).substr(10);
      code << "  case " << name << ":" << endl;
    }
    code << "    return " << emission << ";" << endl;
  }
  code << "  default:" << endl;
  code << "    return 0;" << endl;
  code << "  }" << endl;
  code << "}" << endl;

  for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
    if (ids.contains(id)) {
      continue;
    }
    auto name = mcfile::blocks::Name(id, kJavaDataVersion).substr(10);
    if (name.ends_with(u8"bed")) {
      continue;
    }
    if (name.ends_with(u8"banner")) {
      continue;
    }
    if (name.ends_with(u8"candle_cake")) {
      continue;
    }
    if (name.ends_with(u8"_bulb")) {
      continue;
    }
    using namespace mcfile::blocks::minecraft;
    switch (id) {
    case moving_piston:
    case tinted_glass:
    case cave_vines:
    case cave_vines_plant:
    case frogspawn:
      continue;
    }
    cerr << name << " is not tested" << endl;
  }
}

static void Heightmaps() {
  fs::path root("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/heightmaps");
  {
    fs::path name = root / "datapacks" / "kbinani" / "pack.mcmeta";
    Fs::CreateDirectories(name.parent_path());
    ofstream os(name.string());
    os << R"({
  "pack": {
    "pack_format": 57,
    "description": "datapack"
  }
})" << endl;
  }
  vector<pair<Pos2i, mcfile::blocks::BlockId>> blocks;
  {
    fs::path name = root / "datapacks" / "kbinani" / "data" / "je2be" / "function" / "place_blocks.mcfunction";
    Fs::CreateDirectories(name.parent_path());
    ofstream os(name.string());
    int cx = 0;
    int const cz = 0;
    int lx = 0;
    int z = 0;
    int const y = -60;

    os << "fill -1 -60 -1 -1 -60 16 barrier" << endl;
    os << "fill 0 -60 -1 16 -60 16 barrier" << endl;

    for (mcfile::blocks::BlockId id = 1; id < mcfile::blocks::minecraft::minecraft_max_block_id; id++) {
      auto name = mcfile::blocks::Name(id, kJavaDataVersion).substr(10);

      os << "setblock " << (cx * 16 + lx) << " " << y << " " << z << " " << name << endl;
      blocks.push_back(make_pair(Pos2i(cx * 16 + lx, z), id));

      lx += 2;
      if (lx >= 16) {
        z++;
        if (z % 2 == 0) {
          lx = 0;
        } else {
          lx = 1;
        }
        if (z >= 16) {
          cx++;
          lx = 0;
          z = 0;
          os << "fill " << (cx * 16) << " " << y << " -1 " << (cx * 16 + 16) << " " << y << " 16 barrier" << endl;
        }
      }
    }
  }

  blocks.push_back(make_pair(Pos2i{-16, 0}, mcfile::blocks::minecraft::brain_coral_block));
  blocks.push_back(make_pair(Pos2i{-14, 0}, mcfile::blocks::minecraft::bubble_column));
  blocks.push_back(make_pair(Pos2i{-12, 0}, mcfile::blocks::minecraft::bubble_coral_block));
  blocks.push_back(make_pair(Pos2i{-16, 2}, mcfile::blocks::minecraft::cactus));
  blocks.push_back(make_pair(Pos2i{-10, 0}, mcfile::blocks::minecraft::chorus_flower));
  blocks.push_back(make_pair(Pos2i{-8, 0}, mcfile::blocks::minecraft::chorus_plant));
  blocks.push_back(make_pair(Pos2i{-6, 0}, mcfile::blocks::minecraft::fire));
  blocks.push_back(make_pair(Pos2i{-4, 0}, mcfile::blocks::minecraft::fire_coral_block));
  blocks.push_back(make_pair(Pos2i{-2, 0}, mcfile::blocks::minecraft::horn_coral_block));
  // blocks.push_back(make_pair(Pos2i{, }, mcfile::blocks::minecraft::kelp_plant));
  blocks.push_back(make_pair(Pos2i{-15, 1}, mcfile::blocks::minecraft::sugar_cane));
  blocks.push_back(make_pair(Pos2i{-13, 1}, mcfile::blocks::minecraft::tube_coral_block));
  // blocks.push_back(make_pair(Pos2i{, }, mcfile::blocks::minecraft::weeping_vines));
  // blocks.push_back(make_pair(Pos2i{, }, mcfile::blocks::minecraft::twisting_vines_plant));
  // blocks.push_back(make_pair(Pos2i{, }, mcfile::blocks::minecraft::weeping_vines_plant));
  blocks.push_back(make_pair(Pos2i{-7, 1}, mcfile::blocks::minecraft::soul_fire));
  // blocks.push_back(make_pair(Pos2i{, }, mcfile::blocks::minecraft::big_dripleaf_stem));
  // blocks.push_back(make_pair(Pos2i{, }, mcfile::blocks::minecraft::cave_vines));
  // blocks.push_back(make_pair(Pos2i{, }, mcfile::blocks::minecraft::cave_vines_plant));

  mcfile::je::World world(root);
  shared_ptr<mcfile::je::WritableChunk> chunk;

  HeightmapV2 motionBlocking;
  HeightmapV2 motionBlockingNoLeaves;
  HeightmapV2 oceanFloor;
  HeightmapV2 worldSurface;

  set<mcfile::blocks::BlockId> motionBlockingBlocks;
  set<mcfile::blocks::BlockId> motionBlockingNoLeavesBlocks;
  set<mcfile::blocks::BlockId> oceanFloorBlocks;
  set<mcfile::blocks::BlockId> worldSurfaceBlocks;

  for (auto const &it : blocks) {
    Pos2i p = it.first;
    mcfile::blocks::BlockId id = it.second;
    auto name = mcfile::blocks::Name(id, kJavaDataVersion).substr(10);

    int cx = mcfile::Coordinate::ChunkFromBlock(p.fX);
    int cz = mcfile::Coordinate::ChunkFromBlock(p.fZ);
    if (!chunk || chunk->fChunkX != cx || chunk->fChunkZ != cz) {
      chunk = world.writableChunkAt(cx, cz);
      REQUIRE(chunk);
      auto heightMaps = chunk->fRoot->compoundTag(u8"Heightmaps");
      REQUIRE(heightMaps);
      if (auto tag = heightMaps->longArrayTag(u8"MOTION_BLOCKING"); tag) {
        motionBlocking.copyFrom(tag->fValue);
      } else {
        REQUIRE(false);
      }
      if (auto tag = heightMaps->longArrayTag(u8"MOTION_BLOCKING_NO_LEAVES"); tag) {
        motionBlockingNoLeaves.copyFrom(tag->fValue);
      } else {
        REQUIRE(false);
      }
      if (auto tag = heightMaps->longArrayTag(u8"OCEAN_FLOOR"); tag) {
        oceanFloor.copyFrom(tag->fValue);
      } else {
        REQUIRE(false);
      }
      if (auto tag = heightMaps->longArrayTag(u8"WORLD_SURFACE"); tag) {
        worldSurface.copyFrom(tag->fValue);
      } else {
        REQUIRE(false);
      }
    }
    int lx = p.fX - cx * 16;
    int lz = p.fZ - cz * 16;
    auto block = chunk->blockAt({p.fX, -60, p.fZ});
    if (!block) {
      cerr << name << " not set" << endl;
      continue;
    }
    if (block->fId != id) {
      cerr << name << " not set" << endl;
      continue;
    }
    if (motionBlocking.getUnchecked(lx, lz) == 5) {
      motionBlockingBlocks.insert(id);
    }
    if (motionBlockingNoLeaves.getUnchecked(lx, lz) == 5) {
      motionBlockingNoLeavesBlocks.insert(id);
    }
    if (oceanFloor.getUnchecked(lx, lz) == 5) {
      oceanFloorBlocks.insert(id);
    }
    if (worldSurface.getUnchecked(lx, lz) == 5) {
      worldSurfaceBlocks.insert(id);
    }
  }

  fs::path self = fs::path(__FILE__).parent_path();
  ofstream code((self / "code.hpp").string());
  code << "static bool IsMotionBlockingById(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  switch (id) {" << endl;
  for (auto id : motionBlockingBlocks) {
    auto name = mcfile::blocks::Name(id, kJavaDataVersion).substr(10);
    code << "  case " << name << ":" << endl;
  }
  code << "    return true;" << endl;
  code << "  default:" << endl;
  code << "    return false;" << endl;
  code << "  }" << endl;
  code << "}" << endl;
  code << endl;

  code << "static bool IsMotionBlockingNoLeavesById(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  switch (id) {" << endl;
  for (auto id : motionBlockingNoLeavesBlocks) {
    auto name = mcfile::blocks::Name(id, kJavaDataVersion).substr(10);
    code << "  case " << name << ":" << endl;
  }
  code << "    return true;" << endl;
  code << "  default:" << endl;
  code << "    return false;" << endl;
  code << "  }" << endl;
  code << "}" << endl;
  code << endl;

  code << "static bool IsOceanFloorById(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  switch (id) {" << endl;
  for (auto id : oceanFloorBlocks) {
    auto name = mcfile::blocks::Name(id, kJavaDataVersion).substr(10);
    code << "  case " << name << ":" << endl;
  }
  code << "    return true;" << endl;
  code << "  default:" << endl;
  code << "    return false;" << endl;
  code << "  }" << endl;
  code << "}" << endl;
  code << endl;

  code << "static bool IsWorldSurfaceById(mcfile::blocks::BlockId id) {" << endl;
  code << "  using namespace mcfile::blocks::minecraft;" << endl;
  code << "  switch (id) {" << endl;
  for (auto id : worldSurfaceBlocks) {
    auto name = mcfile::blocks::Name(id, kJavaDataVersion).substr(10);
    code << "  case " << name << ":" << endl;
  }
  code << "    return true;" << endl;
  code << "  default:" << endl;
  code << "    return false;" << endl;
  code << "  }" << endl;
  code << "}" << endl;
  code << endl;
}

static void BEFormat0() {
  {
    unique_ptr<leveldb::DB> db(Open("1.2.10.2-upgrade-test"));
    int id = 0;
    uint8_t fill = 0x14;
    int chunkY = 0;
    while (id < 256) {
      vector<uint8_t> data;
      data.push_back(0);
      for (int i = 0; i < 4096; i++) {
        data.push_back(fill);
      }
      for (int x = 0; x < 15; x++) {
        for (int z = 0; z < 15; z++) {
          for (int y = 0; y < 15 && id < 256; y++) {
            if (x % 2 == 1 && y % 2 == 1 && z % 2 == 1) {
              data[1 + (x * 16 + z) * 16 + y] = id++;
            }
          }
        }
      }
      for (int i = 0; i < 2048; i++) {
        data.push_back(0);
      }
      auto key = mcfile::be::DbKey::SubChunk(0, chunkY, 0, mcfile::Dimension::Overworld);
      string value;
      value.assign((char const *)data.data(), data.size());
      db->Put({}, key, value);
      chunkY++;
    }
  }
  {
    unique_ptr<leveldb::DB> db(Open("after"));
    auto chunk = mcfile::be::Chunk::Load(0, 0, mcfile::Dimension::Overworld, db.get(), mcfile::Encoding::LittleEndian);
    int id = 0;
    for (int x = 0; x < 15; x++) {
      for (int z = 0; z < 15; z++) {
        for (int y = 0; y < 15 && id < 256; y++) {
          if (x % 2 == 1 && y % 2 == 1 && z % 2 == 1) {
            auto block = chunk->blockAt(x, y, z);
            cout << "u8\"" << block->fName << "\"," << endl;
            id++;
          }
        }
      }
    }
  }
}

#if 0
TEST_CASE("research") {
  // Heightmaps();
  // LightEmission();
  // WallConnectable();
  // RedstoneWire();
  // LightTransmission1();
  // NoteBlock();
  // FenceGlassPaneIronBarsConnectable();
}
#endif
