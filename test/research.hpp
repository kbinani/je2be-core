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

static bool ExtractRecursive(je2be::box360::StfsPackage &pkg, je2be::box360::StfsFileListing &listing, std::filesystem::path dir) {
  for (auto &file : listing.fileEntries) {
    auto p = dir / file.name;
    try {
      pkg.ExtractFile(&file, p.string());
    } catch (...) {
      return false;
    }
  }
  for (auto &folder : listing.folderEntries) {
    auto sub = dir / folder.folder.name;
    if (!ExtractRecursive(pkg, folder, sub)) {
      return false;
    }
  }
  return true;
}

static std::shared_ptr<mcfile::je::Block const> Box360BlockFromBytes(uint8_t v1, uint8_t v2) {
  using namespace std;
  using namespace mcfile::je;
  uint8_t t1 = v1 >> 4;
  uint8_t t2 = 0xf & v1;
  uint8_t t3 = v2 >> 4;
  uint8_t t4 = 0xf & v2;

  uint16_t blockId = t4 << 4 | t1;
  uint8_t data = t3 << 4 | t2;
  if (t3 != 0) {
    string name;
    map<string, string> props;
    switch (blockId) {
    case 14:
      switch (t3) {
      case 9:
        switch (t2) {
        case 0:
          name = "seagrass";
          break;
        case 1:
          name = "tall_seagrass";
          props["half"] = "upper";
          break;
        case 2:
          name = "tall_seagrass";
          props["half"] = "lower";
          break;
        }
        break;
      }
      break;
    }
    if (!name.empty()) {
      return make_shared<Block const>("minecraft:" + name, props);
    }
    auto p = mcfile::je::Flatten::DoFlatten(blockId, data);
    return p;
  } else {
    return mcfile::je::Flatten::DoFlatten(blockId, data);
  }
}

static bool Box360ParsePalette(uint8_t const *buffer, std::vector<std::shared_ptr<mcfile::je::Block const>> &palette, int maxSize) {
  palette.clear();
  for (uint16_t i = 0; i < maxSize; i++) {
    uint8_t v1 = buffer[i * 2];
    uint8_t v2 = buffer[i * 2 + 1];
    auto block = Box360BlockFromBytes(v1, v2);
    if (!block) {
      return false;
    }
    palette.push_back(block);
  }
  return true;
}

static bool Box360ParseGridFormat0(uint8_t const *buffer, std::vector<std::shared_ptr<mcfile::je::Block const>> &palette, std::vector<uint16_t> &index) {
  // TODO: Verify if this guess is correct.
  uint8_t v1 = buffer[0];
  uint8_t v2 = buffer[1];
  auto block = Box360BlockFromBytes(v1, v2);
  if (!block) {
    return false;
  }
  palette.clear();
  palette.push_back(0);
  index.resize(64, 0);
  std::fill(index.begin(), index.end(), 0);
  return true;
}

static bool Box360ParseGridFormatF(uint8_t const *buffer, std::vector<std::shared_ptr<mcfile::je::Block const>> &palette, std::vector<uint16_t> &index) {
  if (!Box360ParsePalette(buffer, palette, 64)) {
    return false;
  }
  index.resize(64, 0);
  for (uint16_t i = 0; i < 64; i++) {
    index[i] = i;
  }
  return true;
}

template <size_t Bits>
static bool Box360ParseGridFormatGeneric(uint8_t const *buffer, std::vector<std::shared_ptr<mcfile::je::Block const>> &palette, std::vector<uint16_t> &index) {
  int size = 1 << Bits;
  if (!Box360ParsePalette(buffer, palette, size)) {
    return false;
  }
  index.resize(64, 0);
  for (int i = 0; i < 8; i++) {
    uint8_t v[Bits];
    for (int j = 0; j < Bits; j++) {
      v[j] = buffer[size * 2 + i + j * 8];
    }
    for (int j = 0; j < 8; j++) {
      uint8_t mask = (uint8_t)0x80 >> j;
      uint16_t idx = 0;
      for (int k = 0; k < Bits; k++) {
        idx |= ((v[k] & mask) >> (7 - j)) << k;
      }
      if (idx >= palette.size()) [[unlikely]] {
        return false;
      }
      index[i * 8 + j] = idx;
    }
  }
  return true;
}

static bool Box360ParseGridFormat9(uint8_t const *buffer, std::vector<std::shared_ptr<mcfile::je::Block const>> &palette, std::vector<uint16_t> &index) {
  /*
  case 1: (000-fill-bedrock-under-sea-level, chunk=[25, 25], grid=[1, 1, 1], sectionY=2)
  90 00 D0 00  10 00 20 06        80 00         22 06                21 06              E0 10 00 00 E0 90
  water gravel stone stone_bricks flowing_water cracked_stone_bricks mossy_stone_bricks ?     air   ?
  FF FF FF FF FF FF FF FF FF FF FF FF 08 84 0C C4 08 C4 4C 04 00 08 00 08 00 08 88 88 00 00 00 00 00 00 80 80 00 00 04 40 08 40 44 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 08 00 44 00 FF FF FF FF F7 FF BB FF

  block=[404, 36, 404] ~ [407, 39, 407]

  stone gravel seagrass water  stone gravel water water  sand gravel water water  coal_ore gravel water water
  stone iron_ore gravel water  ? gravel water water      ? gravel water water     ? gravel water water
  stone gravel water water     ? ? gravel water          ? gravel seagrass water  ? gravel water water
  stone gravel water water ...

  index
  21?0 2100 ?

  block id
  1 D ? 9  1 D 9 9  C D 9 9  10 D 9 9
  1 F D 9  ? D 9 9  ? D 9 9  ? D 9 9
  1 D 9 9 ...

  */
  // TODO:
  return false;
}

static bool Box360ParseGridFormatE(uint8_t const *buffer, std::vector<std::shared_ptr<mcfile::je::Block const>> &palette, std::vector<uint16_t> &index) {
  /*
  D0 00 90 00 90 00 90 00 D0 00 D0 00 90 00 90 00 D0 00 D0 00 90 00 90 00 D0 00 10 00 D0 00 2B 90 E0 90 90 00 90 00 90 00 D0 00 90 00 90 00 90 00 10 00 D0 00 E0 90 90 00 10 00 10 00 D0 00 E0 90 E0 90 90 00 90 00 90 00 90 00 90 00 90 00 90 00 D0 00 90 00 90 00 90 00 10 00 D0 00 90 00 90 00 90 00 90 00 90 00 90 00 29 90 2A 90 2B 90 2C 90 21 90 22 90 23 90 24 90 D0 00 E0 90 90 00 90 00
*/
  // TODO:
  return false;
}

static void FillAir(mcfile::je::Chunk &chunk) {
  auto air = make_shared<mcfile::je::Block const>("minecraft:air");
  for (int y = chunk.minBlockY(); y <= chunk.maxBlockY(); y++) {
    for (int z = chunk.minBlockZ(); z <= chunk.maxBlockZ(); z++) {
      for (int x = chunk.minBlockX(); x <= chunk.maxBlockX(); x++) {
        chunk.setBlockAt(x, y, z, air);
      }
    }
  }
}

static void Box360Chunk() {
  using namespace je2be::box360;
  fs::path dir("C:/Users/kbinani/Documents/Projects/je2be-gui/00000001");
  fs::path output("C:/Users/kbinani/AppData/Roaming/.minecraft/saves/_b2j-out");

  for (auto name : {
           // "1-Save20220305225836-000-pig-name=Yahoo.bin",
           // "1-Save20220305225836-001-pig-name=Yohoo.bin",
           // "1-Save20220305225836-002-put-cobblestone.bin",
           // "1-Save20220305225836-003-remove-cobblestone.bin",
           // "2-Save20220306005722-000-fill-bedrock-under-sea-level.bin",
           // "2-Save20220306005722-001-chunk-filled-by-bedrock.bin",
           // "abc-Save20220303092528.bin",
           // "2-Save20220306005722-002-c.25.25-section0-filled-with-bedrock.bin",
           // "2-Save20220306005722-003-heightmap-check.bin",
           // "2-Save20220306005722-004-place-dirt-lx=15-ly=15-lz=15.bin",
           // "2-Save20220306005722-005-place-dirt-lx=14-ly=15-lz=15.bin",
           // "2-Save20220306005722-006-place-dirt-lx=13-ly=15-lz=15.bin",
           // "2-Save20220306005722-007-place-dirt-lx=0-ly=15-lz=15.bin",
           // "2-Save20220306005722-008-fill-dirt-lz=15-ly=15.bin",
           // "2-Save20220306005722-009-reset-lz15_ly=15-fill-dirt-l0_ly15.bin",
           // "2-Save20220306005722-010-refill-bedrock-again.bin",
           // "2-Save20220306005722-011-preparing-empty-chunk-at-c.25.25.bin",
           // "2-Save20220306005722-012-empty-chunk-at-c.25.25.bin",
           // "2-Save20220306005722-013-just-resaved-after-012.bin",
           // "2-Save20220306005722-014-just-resaved-after-013.bin",
           // "2-Save20220306005722-015-setblock 0 1 1 bedrock.bin",
           // "2-Save20220306005722-016-setblock 1 1 0 bedrock.bin",
           // "2-Save20220306005722-017-setblock 2 1 0 bedrock.bin",
           // "2-Save20220306005722-018-setblock 0 1 1 bedrock.bin",
           // "2-Save20220306005722-019-setblock 0 2 0 bedrock.bin",
           // "2-Save20220306005722-020-setblock 0 3 0 bedrock.bin",
           // "2-Save20220306005722-021-fill 0 0 0 3 3 3 bedrock.bin",
           // "2-Save20220306005722-022-setblock 0 4 0 bedrock.bin",
           // "2-Save20220306005722-023-fill 0 1 0 3 4 3 air.bin",
           // "2-Save20220306005722-024-setblock 0 1 0 iron_block.bin",
           // "2-Save20220306005722-025-setblock 0 1 0 carved_pumpkin[facing=south].bin",
           // "2-Save20220306005722-026-setblock 0 1 0 carved_pumpkin[facing=east].bin",
           // "2-Save20220306005722-027-setblock 0 1 4 carved_pumpkin[facing=east].bin",
           // "2-Save20220306005722-028-setblock 4 1 0 carved_pumpkin[facing=south].bin",
           // "2-Save20220306005722-029-setblock 0 1 8 carved_pumpkin[facing=south].bin",
           // "2-Save20220306005722-030-setblock 0 1 12 iron_block.bin",
           // "2-Save20220306005722-031-setblock 4 1 0 gold_block.bin",
           // "2-Save20220306005722-032-setblock 4 1 4 dirt.bin",
           // "2-Save20220306005722-033-put bedrocks to grid corners under sea level.bin",
           // "2-Save20220306005722-034-resaved.bin",
           // "2-Save20220306005722-035-fill grid(1,0,0) with iron_block.bin",
           // "2-Save20220306005722-036-fill grid(1,1,0) with gold_block.bin",
           // "2-Save20220306005722-037-fill grid(0,0,0) with bedrock.bin",
           // "2-Save20220306005722-038-fill grid(0,1,0) with some blocks.bin",
           // "2-Save20220306005722-039-fill 0 4 0 3 4 3 bedrock.bin",
           // "2-Save20220306005722-040-gyazo-76ef1d3bf73d1094f76fb5af627b002a.bin",
           // "2-Save20220306005722-041-gyazo-fa8a1fab5f80678d98a7010fd61019bc.bin",
           // "2-Save20220306005722-042-gyazo-def2a9fdcd6f9c9e997328a38ecc401e.bin",
           // "2-Save20220306005722-043-gyazo-377bb6e38aa6d2d2eddfa3837f96cda4.bin",
           // "2-Save20220306005722-044-5a3fe7fb82e798160542986f94a0d3f9.bin",
           // "2-Save20220306005722-045-gyazo-038020972af102b51ce606638423941b.bin",
           // "2-Save20220306005722-046-gyazo-1dee95a946200236c0dbcb0c5e13ddbe.bin",
           //"2-Save20220306005722-047-gyazo-f91e998c6e4399613dc1119e915b3208.bin",
           //"2-Save20220306005722-048-gyazo-ed9c2429bbb2ca1f1ca44d63ee703c5c.bin",
           //"2-Save20220306005722-049-gyazo-393b51668ac72255b1c53ab8976b6d84.bin",
           //"2-Save20220306005722-050-gyazo-788e2cc3a60b792056877d235dc08f15.bin",
           "2-Save20220306005722-051-gyazo-6e2b17c1852b53ff19d1e0cccc0d9f46.bin",
       }) {
    cout << name << endl;
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
    for (int rz = -1; rz <= 0; rz++) {
      for (int rx = -1; rx <= 0; rx++) {
        if (rx != 0 || rz != 0) {
          continue;
        }
        auto region = *temp / "region" / ("r." + to_string(rx) + "." + to_string(rz) + ".mcr");
        CHECK(fs::exists(region));
        auto regionTempDir = *temp / "region" / ("r." + to_string(rx) + "." + to_string(rz));
        CHECK(Fs::CreateDirectories(regionTempDir));
        defer {
          Fs::Delete(regionTempDir);
        };
        for (int cz = 0; cz < 32; cz++) {
          for (int cx = 0; cx < 32; cx++) {
            if (cx != 24 || cz != 25) {
              continue;
            }
            auto chunk = mcfile::je::WritableChunk::MakeEmpty(rx * 32 + cx, 0, rz * 32 + cz);
            FillAir(*chunk);
            {
              auto f = make_shared<mcfile::stream::FileInputStream>(region);
              CHECK(Savegame::ExtractRawChunkFromRegionFile(*f, cx, cz, buffer));
            }
            if (buffer.empty()) {
              continue;
            }
            CHECK(buffer.size() > 0);
            CHECK(Savegame::DecompressRawChunk(buffer));

            string basename = fs::path(name).replace_extension().string();
            Savegame::DecodeDecompressedChunk(buffer);

            string data;
            data.assign((char const *)buffer.data(), buffer.size());
            vector<uint8_t>().swap(buffer);
            size_t offset = 0;

            while (offset < data.size()) {
              auto found = data.find(string("\x0a\x00\x00", 3), offset);
              CHECK(found != string::npos);
              auto sub = data.substr(found);
              auto bs = make_shared<mcfile::stream::ByteStream>(sub);
              auto tag = CompoundTag::Read(bs, endian::big);
              if (!tag || bs->pos() != sub.size()) {
                offset = found + 3;
                continue;
              }
              buffer.assign(data.begin(), data.begin() + found);
              break;
            }
            if (buffer.empty()) {
              cerr << "cannot find compound tag: region=[" << rx << ", " << rz << "]; chunk=[" << cx << ", " << cz << "]" << endl;
              continue;
            }

#if 1
            CHECK(make_shared<mcfile::stream::FileOutputStream>(dir / (basename + "-c." + to_string(cx) + "." + to_string(cz) + ".prefix.bin"))->write(buffer.data(), buffer.size()));
#endif

            uint8_t maybeEndTagMarker = buffer[0];       // 0x00. The presence of this tag prevents the file from being parsed as nbt.
            uint8_t maybeLongArrayTagMarker = buffer[1]; // 0x0c. Legacy parsers that cannot interpret the LongArrayTag will fail here.
            int32_t xPos = mcfile::I32FromBE(*(int32_t *)(buffer.data() + 0x2));
            int32_t zPos = mcfile::I32FromBE(*(int32_t *)(buffer.data() + 0x6));
            int64_t maybeLastUpdate = mcfile::I64FromBE(*(int64_t *)(buffer.data() + 0x0a));
            int64_t maybeInhabitedTime = mcfile::I64FromBE(*(int64_t *)(buffer.data() + 0x12));

            uint16_t maxSectionAddress = (uint16_t)buffer[0x1b] * 0x100;
            vector<uint16_t> sectionJumpTable;
            for (int section = 0; section < 16; section++) {
              uint16_t address = mcfile::U16FromBE(*(uint16_t *)(buffer.data() + 0x1c + section * sizeof(uint16_t)));
              sectionJumpTable.push_back(address);
            }

            vector<uint8_t> maybeNumBlockPaletteEntriesFor16Sections;
            for (int section = 0; section < 16; section++) {
              uint8_t numBlockPaletteEntries = buffer[0x3c + section];
              maybeNumBlockPaletteEntriesFor16Sections.push_back(numBlockPaletteEntries);
            }

            for (int section = 0; section < 16; section++) {
              uint16_t address = sectionJumpTable[section];

              if (address == maxSectionAddress) {
                break;
              }

              vector<uint8_t> gridJumpTable;                                             // "grid" is a cube of 4x4x4 blocks.
              copy_n(buffer.data() + 0x4c + address, 128, back_inserter(gridJumpTable)); // [0x4c, 0xcb]
              for (int gx = 0; gx < 4; gx++) {
                for (int gz = 0; gz < 4; gz++) {
                  for (int gy = 0; gy < 4; gy++) {
                    int gridIndex = gx * 16 + gz * 4 + gy;
                    int bx = gx * 4;
                    int by = gy * 4;
                    int bz = gz * 4;

                    uint8_t v1 = gridJumpTable[gridIndex * 2];
                    uint8_t v2 = gridJumpTable[gridIndex * 2 + 1];
                    uint16_t t1 = v1 >> 4;
                    uint16_t t2 = (uint16_t)0xf & v1;
                    uint16_t t3 = v2 >> 4;
                    uint16_t t4 = (uint16_t)0xf & v2;

                    uint16_t offset = (t4 << 8 | t1 << 4 | t2) * 4;
                    uint16_t format = t3;

                    // grid(gx, gy, gz) starts from 0xCC + offset

                    vector<shared_ptr<mcfile::je::Block const>> palette;
                    vector<uint16_t> index;

                    // When n bits per block:
                    //   Maximum palette entries = 2^n
                    //   Palette size in bytes: 2^n * 2
                    //   Index body in bytes: n * 8
                    //   Total bytes = 2^n * 2 + n * 8
                    // n = 1: 12 bytes   2
                    // n = 2: 24 bytes   4
                    // n = 3: 40 bytes   8
                    // n = 4: 64 bytes   16
                    // n = 5: 104 bytes  32
                    // ----------------
                    // n = 6: 176 bytes
                    // n = 7: 312 bytes
                    // n = 8: 576 bytes
                    //
                    // 96 = 2^n*2 +n*8

                    uint16_t gridPosition = 0x4c + address + 0x80 + offset;
                    if (format == 0) {
                      CHECK(Box360ParseGridFormat0(buffer.data() + gridPosition, palette, index));
                    } else if (format == 0xF) {
                      CHECK(gridPosition + 128 < buffer.size());
                      CHECK(Box360ParseGridFormatF(buffer.data() + gridPosition, palette, index));
                    } else if (format == 0x2) { // 1 bit
                      //OK
                      CHECK(gridPosition + 12 < buffer.size());
                      CHECK(Box360ParseGridFormatGeneric<1>(buffer.data() + gridPosition, palette, index));
                    } else if (format == 0x4) { // 2 bit
                      //OK
                      CHECK(gridPosition + 24 < buffer.size());
                      CHECK(Box360ParseGridFormatGeneric<2>(buffer.data() + gridPosition, palette, index));
                    } else if (format == 0x6) { // 3 bit
                      CHECK(gridPosition + 40 < buffer.size());
                      CHECK(Box360ParseGridFormatGeneric<3>(buffer.data() + gridPosition, palette, index));
                    } else if (format == 0x8) { // 4 bit
                      CHECK(gridPosition + 64 < buffer.size());
                      CHECK(Box360ParseGridFormatGeneric<4>(buffer.data() + gridPosition, palette, index));
                    } else {
                      uint8_t nextV1 = gridJumpTable[gridIndex * 2 + 2];
                      uint8_t nextV2 = gridJumpTable[gridIndex * 2 + 3];
                      uint16_t nextT1 = nextV1 >> 4;
                      uint16_t nextT2 = (uint16_t)0xf & nextV1;
                      uint16_t nextT3 = nextV2 >> 4;
                      uint16_t nextT4 = (uint16_t)0xf & nextV2;

                      uint16_t nextOffset = (nextT4 << 8 | nextT1 << 4 | nextT2) * 4;

                      int nextGridPosition = 0x4c + address + 0x80 + nextOffset;
                      int bx = (rx * 32 + cx) * 16 + gx * 4;
                      int by = section * 16 + gy * 4;
                      int bz = (rz * 32 + cz) * 16 + gz * 4;
                      cerr << "unknown format: 0x" << hex << (int)format << dec << "; chunk=[" << (rx * 32 + cx) << ", " << (rz * 32 + cz) << "] ; gridPosition=0x" << hex << gridPosition << "; nextGridPosition=0x" << nextGridPosition << "; sectionHead=0x" << (0x4c + address) << dec << "; block=[" << bx << ", " << by << ", " << bz << "]-[" << (bx + 3) << ", " << (by + 3) << ", " << (bz + 3) << "]" << endl;
                      if (format == 0x7) {
                        CHECK(Box360ParseGridFormatGeneric<3>(buffer.data() + gridPosition, palette, index));
                      }
                      // CHECK(false);
                    }

                    if (index.size() != 64) {
                      continue;
                    }

                    for (int lx = 0; lx < 4; lx++) {
                      for (int lz = 0; lz < 4; lz++) {
                        for (int ly = 0; ly < 4; ly++) {
                          int idx = lx * 16 + lz * 4 + ly;
                          uint16_t paletteIndex = index[idx];
                          auto block = palette[paletteIndex];
                          int bx = rx * 512 + cx * 16 + gx * 4 + lx;
                          int by = section * 16 + gy * 4 + ly;
                          int bz = rz * 512 + cz * 16 + gz * 4 + lz;
                          chunk->setBlockAt(bx, by, bz, block);
                        }
                      }
                    }
                  }
                }
              }
            }

            // Biomes: 256 bytes / chunk
            // HeightMap: 256 bytes / chunk

            // BlockLight: 2048 bytes / section: 4bit per block
            // Blocks: 4096 bytes / section: 1byte per block
            // Data: 2048 bytes / section: 4bit per block
            // SkyLight: 2048 bytes / section: 4bit per block
            // --
            // Total: 10240 bytes / section

            int heightMapStartPos = buffer.size() - 256 - 2 - 256;
            vector<uint8_t> heightMap;
            copy_n(buffer.data() + heightMapStartPos, 256, back_inserter(heightMap)); // When heightMap[x + z * 16] == 0, it means height = 256 at (x, z).

            uint16_t unknownMarkerA = mcfile::U16FromBE(*(uint16_t *)(buffer.data() + buffer.size() - 256 - 2)); // 0x07fe
            CHECK(unknownMarkerA == 0x07fe);
            if (unknownMarkerA != 0x07fe) {
              continue;
            }

            vector<mcfile::biomes::BiomeId> biomes;
            for (int i = 0; i < 256; i++) {
              auto raw = buffer[buffer.size() - 256 - 1];
              auto biome = mcfile::biomes::FromInt(raw);
              biomes.push_back(biome);
            }
            for (int z = 0; z < 16; z++) {
              for (int x = 0; x < 16; x++) {
                auto biome = biomes[x + z * 16];
                int bx = rx * 32 + cx * 16 + x;
                int bz = rz * 32 + cz * 16 + z;
                for (int by = 0; by < 256; by++) {
                  chunk->setBiomeAt(bx, by, bz, biome);
                }
              }
            }

            auto nbtz = regionTempDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
            auto stream = make_shared<mcfile::stream::FileOutputStream>(nbtz);
            CHECK(chunk->write(*stream));
          }
        }
        CHECK(mcfile::je::Region::ConcatCompressedNbt(rx, rz, regionTempDir, output / "region" / mcfile::je::Region::GetDefaultRegionFileName(rx, rz)));
      }
    }
  }
}

#if 1
TEST_CASE("research") {
  Box360Chunk();
}
#endif
