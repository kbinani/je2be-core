#include <iostream>
#include <je2be.hpp>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#include <shlobj_core.h>
#endif

using namespace std;
using namespace leveldb;
using namespace je2be;
using namespace mcfile::stream;
namespace fs = filesystem;

static optional<mcfile::Dimension> DimensionFromString(string const &s) {
  if (s == "overworld" || s == "o" || s == "0") {
    return mcfile::Dimension::Overworld;
  } else if (s == "nether" || s == "n" || s == "-1") {
    return mcfile::Dimension::Nether;
  } else if (s == "end" || s == "e" || s == "1") {
    return mcfile::Dimension::End;
  }
  return nullopt;
}

static DB *Open(fs::path dir) {
  Options o;
  o.compression = kZlibRawCompression;
  DB *db = nullptr;
  auto st = DB::Open(o, dir, &db);
  if (!st.ok()) {
    return nullptr;
  }
  return db;
}

static void DumpBlock(fs::path const &dbDir, int x, int y, int z, mcfile::Dimension d) {
  unique_ptr<DB> db(Open(dbDir));
  if (!db) {
    return;
  }

  ReadOptions ro;
  mcfile::nbt::JsonPrintOptions jopt;
  jopt.fTypeHint = true;

  int cx = mcfile::Coordinate::ChunkFromBlock(x);
  int cy = mcfile::Coordinate::ChunkFromBlock(y);
  int cz = mcfile::Coordinate::ChunkFromBlock(z);
  auto key = mcfile::be::DbKey::SubChunk(cx, cy, cz, d);
  string value;
  auto st = db->Get(ro, key, &value);
  if (!st.ok()) {
    return;
  }
  auto section = mcfile::be::SubChunk::Parse(value, cy, mcfile::Endian::Little);
  if (!section) {
    return;
  }

  int lx = x - cx * 16;
  int ly = y - cy * 16;
  int lz = z - cz * 16;
  auto block = section->blockAt(lx, ly, lz);
  if (!block) {
    return;
  }

  auto tag = Compound();
  tag->set("name", String(block->fName));
  auto states = Compound();
  states = block->fStates;
  tag->set("states", states);
  tag->set("version", Int(block->fVersion));
  mcfile::nbt::PrintAsJson(cout, *tag, jopt);
  if (section->fWaterPaletteIndices.size() == 4096) {
    auto index = mcfile::be::SubChunk::BlockIndex(lx, ly, lz);
    auto idx = section->fWaterPaletteIndices[index];
    cout << "layer2 ---" << endl;
    auto layer2Block = section->fWaterPalette[idx];
    auto t = Compound();
    t->set("name", String(layer2Block->fName));
    t->set("states", layer2Block->fStates);
    t->set("version", Int(layer2Block->fVersion));
    mcfile::nbt::PrintAsJson(cout, *t, jopt);
  }
}

static void DumpBlockEntity(fs::path const &dbDir, int x, int y, int z, mcfile::Dimension d) {
  unique_ptr<DB> db(Open(dbDir));
  if (!db) {
    cerr << "Error: cannot open db: dbDir=" << dbDir << endl;
    return;
  }

  ReadOptions ro;
  mcfile::nbt::JsonPrintOptions jopt;
  jopt.fTypeHint = true;

  int cx = mcfile::Coordinate::ChunkFromBlock(x);
  int cy = mcfile::Coordinate::ChunkFromBlock(y);
  int cz = mcfile::Coordinate::ChunkFromBlock(z);
  auto key = mcfile::be::DbKey::BlockEntity(cx, cz, d);

  string value;
  auto st = db->Get(ro, key, &value);
  if (!st.ok()) {
    cerr << "Error: cannot get Key::BlockEntity(" << cx << ", " << cz << ", " << (int)d << ")" << endl;
    return;
  }
  vector<uint8_t> buffer;
  copy(value.begin(), value.end(), back_inserter(buffer));
  auto stream = make_shared<ByteStream>(buffer);
  InputStreamReader sr(stream, mcfile::Endian::Little);
  while (true) {
    uint8_t type;
    if (!sr.read(&type)) {
      break;
    }
    string name;
    if (!sr.read(name)) {
      break;
    }
    auto tag = Compound();
    tag->read(sr);

    auto bx = tag->int32("x");
    auto by = tag->int32("y");
    auto bz = tag->int32("z");
    if (bx && by && bz) {
      if (*bx == x && *by == y && *bz == z) {
        PrintAsJson(cout, *tag, jopt);
      }
    }
  }
}

static void DumpKey(fs::path const &dbDir, string const &key) {
  unique_ptr<DB> db(Open(dbDir));
  if (!db) {
    return;
  }

  ReadOptions ro;
  mcfile::nbt::JsonPrintOptions jopt;
  jopt.fTypeHint = true;

  string value;
  auto st = db->Get(ro, key, &value);
  if (!st.ok()) {
    return;
  }
  vector<uint8_t> buffer;
  copy(value.begin(), value.end(), back_inserter(buffer));
  auto stream = make_shared<ByteStream>(buffer);
  InputStreamReader sr(stream, mcfile::Endian::Little);
  while (true) {
    uint8_t type;
    if (!sr.read(&type)) {
      break;
    }
    string name;
    if (!sr.read(name)) {
      break;
    }
    auto tag = Compound();
    tag->read(sr);

    PrintAsJson(cout, *tag, jopt);
  }
}

static void DumpBinaryKey(fs::path const &dbDir, std::string const &key) {
  unique_ptr<DB> db(Open(dbDir));
  if (!db) {
    return;
  }

  ReadOptions ro;
  mcfile::nbt::JsonPrintOptions jopt;
  jopt.fTypeHint = true;

  string value;
  auto st = db->Get(ro, key, &value);
  if (!st.ok()) {
    return;
  }
#if defined(_WIN32)
  _setmode(_fileno(stdout), _O_BINARY);
#endif
  std::cout << value;
}

static void DumpVersionKey(fs::path const &dbDir, std::string const &key) {
  unique_ptr<DB> db(Open(dbDir));
  if (!db) {
    return;
  }

  ReadOptions ro;
  mcfile::nbt::JsonPrintOptions jopt;
  jopt.fTypeHint = true;

  string value;
  if (!db->Get(ro, key, &value).ok()) {
    return;
  }
  if (value.size() != 1) {
    return;
  }
  char b = value[0];
  uint8_t v = *(uint8_t *)&b;
  std::cout << (int)v << std::endl;
}

static void DumpChunkKey(fs::path const &dbDir, int cx, int cz, mcfile::Dimension d, uint8_t tag) {
  auto key = mcfile::be::DbKey::ComposeChunkKey(cx, cz, d, tag);
  using Tag = mcfile::be::DbKey::Tag;
  switch (tag) {
  case static_cast<uint8_t>(Tag::StructureBounds):
  case static_cast<uint8_t>(Tag::ChecksumsLegacy):
  case static_cast<uint8_t>(Tag::Data3D):
  case static_cast<uint8_t>(Tag::Data2D):
  case static_cast<uint8_t>(Tag::FinalizedState):
  case static_cast<uint8_t>(Tag::UnknownTag3f):
    DumpBinaryKey(dbDir, key);
    break;
  case static_cast<uint8_t>(Tag::Version):
    DumpVersionKey(dbDir, key);
    break;
  default:
    DumpKey(dbDir, key);
    break;
  }
}

static bool DumpLevelDat(fs::path const &dbDir) {
  auto datFile = dbDir.parent_path() / L"level.dat";
  if (!fs::is_regular_file(datFile)) {
    cerr << "Error: " << datFile << " does not exist" << endl;
    return false;
  }
  auto stream = make_shared<FileInputStream>(datFile);
  InputStreamReader reader(stream, mcfile::Endian::Little);
  auto tag = Compound();
  stream->seek(8);
  tag->read(reader);

  mcfile::nbt::JsonPrintOptions o;
  o.fTypeHint = true;
  PrintAsJson(cout, *tag, o);
  return true;
}

static void DumpAllKeys(fs::path const &dbDir) {
  unique_ptr<DB> db(Open(dbDir));
  if (!db) {
    return;
  }

  ReadOptions ro;
  unique_ptr<Iterator> itr(db->NewIterator(ro));
  for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
    auto key = itr->key().ToString();
    auto parsed = mcfile::be::DbKey::Parse(key);
    if (!parsed) {
      cout << "unknown key: ";
      for (size_t i = 0; i < key.size(); i++) {
        char ch = key[i];
        cout << hex << (int)ch << dec << (0 < i && i < key.size() - 1 ? ", " : "");
      }
      cout << endl;
    } else {
      cout << parsed->toString() << endl;
    }
  }

  itr.reset();
  db.reset();
}

static int DumpEntities(fs::path const &dbDir, int chunkX, int chunkZ, mcfile::Dimension dimension) {
  unique_ptr<DB> db(Open(dbDir));
  if (!db) {
    return 1;
  }

  auto entitiesKey = mcfile::be::DbKey::Entity(chunkX, chunkZ, dimension);
  string entitiesValue;
  ReadOptions ro;
  if (db->Get(ro, entitiesKey, &entitiesValue).ok()) {
    CompoundTag::ReadUntilEos(entitiesValue, mcfile::Endian::Little, [](CompoundTagPtr const &e) {
      mcfile::nbt::PrintAsJson(cout, *e, {.fTypeHint = true});
    });
  }

  auto digpKey = mcfile::be::DbKey::Digp(chunkX, chunkZ, dimension);
  string digpValue;
  if (db->Get(ro, digpKey, &digpValue).ok()) {
    mcfile::be::DbKey::EnumerateActorprefixKeys(digpValue, [&db, ro](int index, string const &key, bool &stop) {
      string actor;
      if (db->Get(ro, mcfile::be::DbKey::Actorprefix(key), &actor).ok()) {
        if (auto c = CompoundTag::Read(actor, mcfile::Endian::Little); c) {
          mcfile::nbt::PrintAsJson(cout, *c, {.fTypeHint = true});
        }
      }
    });
  }
  return 0;
}

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

static void PrintHelpMessage() {
  cerr << R"(dump.exe [world-dir] block at [x] [y] [z] of ["overworld" | "nether" | "end"])" << endl;
  cerr << R"(dump.exe [world-dir] block entity at [x] [y] [z] of ["overworld" | "nether" | "end"])" << endl;
  cerr << R"(dump.exe [world-dir] entities in [chunkX] [chunkZ] of ["overworld" | "nether" | "end"])" << endl;
  cerr << R"(dump.exe [world-dir] chunk-key [tag:uint8_t] in [chunkX] [chunkZ] of ["overworld" | "nether" | "end"])" << endl;
  cerr << "    tag:" << endl;
  cerr << "       43: Data2D" << endl;
  cerr << "       44: ChunkVersion" << endl;
  cerr << "       45: Data2DLegacy" << endl;
  cerr << "       48: LegacyTerrian" << endl;
  cerr << "       49: BlockEntity" << endl;
  cerr << "       50: Entity" << endl;
  cerr << "       51: PendingTicks" << endl;
  cerr << "       52: BlockExtraData" << endl;
  cerr << "       53: BiomeState" << endl;
  cerr << "       54: FinalizedState" << endl;
  cerr << "       56: BorderBlocks" << endl;
  cerr << "       57: StructureBounds" << endl;
  cerr << "       58: RandomTicks" << endl;
  cerr << "       59: ChecksumsLegacy" << endl;
  cerr << "       118: ChunkVersionLegacy" << endl;
  cerr << "dump.exe [world-dir] level.dat" << endl;
  cerr << "dump.exe [world-dir] keys" << endl;
  cerr << "dump.exe [world-dir] key [any-string]" << endl;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    PrintHelpMessage();
    return 1;
  }

  vector<string> args;
  for (int i = 0; i < argc; i++) {
    args.push_back(string(argv[i]));
  }

  string d = args[1];
  fs::path dir = d;
  if (!fs::exists(dir)) {
    dir = dir / L"db";
  }
  if (!fs::exists(dir)) {
    auto appDir = GetLocalApplicationDirectory(); // X:/Users/whoami/AppData/Local
    if (!appDir) {
      cerr << "Error: cannot get AppData directory" << endl;
      return 1;
    }
    auto p = fs::path(*appDir) / L"Packages" / L"Microsoft.MinecraftUWP_8wekyb3d8bbwe" / L"LocalState" / L"games" / L"com.mojang" / L"minecraftWorlds" / d / L"db";
    dir = p;
  }
  if (!fs::exists(dir)) {
    cerr << "Error: directory not found: " << d << endl;
    PrintHelpMessage();
    return 1;
  }

  string verb = args[2];

  if (verb == "block") {
    if (argc == 9 && args[3] == "at" && args[7] == "of") {
      auto x = strings::Toi(args[4]);
      if (!x) {
        cerr << "Error: invalid block x: " << args[4] << endl;
        PrintHelpMessage();
        return 1;
      }
      auto y = strings::Toi(args[5]);
      if (!y) {
        cerr << "Error: invalid block y: " << args[5] << endl;
        PrintHelpMessage();
        return 1;
      }
      auto z = strings::Toi(args[6]);
      if (!z) {
        cerr << "Error: invalid block z: " << args[6] << endl;
        PrintHelpMessage();
        return 1;
      }
      auto dimension = DimensionFromString(args[8]);
      if (!dimension) {
        cerr << "Error: invalid dimension: " << args[8] << endl;
        PrintHelpMessage();
        return 1;
      }
      DumpBlock(dir, *x, *y, *z, *dimension);
    } else if (argc == 10 && args[3] == "entity" && args[4] == "at" && args[8] == "of") {
      auto x = strings::Toi(args[5]);
      if (!x) {
        cerr << "Error: invalid block x: " << args[5] << endl;
        PrintHelpMessage();
        return 1;
      }
      auto y = strings::Toi(args[6]);
      if (!y) {
        cerr << "Error: invalid block y: " << args[6] << endl;
        PrintHelpMessage();
        return 1;
      }
      auto z = strings::Toi(args[7]);
      if (!z) {
        cerr << "Error: invalid block z: " << args[7] << endl;
        PrintHelpMessage();
        return 1;
      }
      auto dimension = DimensionFromString(args[9]);
      if (!dimension) {
        cerr << "Error: invalid dimension: " << args[8] << endl;
        PrintHelpMessage();
        return 1;
      }
      DumpBlockEntity(dir, *x, *y, *z, *dimension);
    } else {
      PrintHelpMessage();
      return 1;
    }
  } else if (verb == "entities") {
    if (argc != 8) {
      PrintHelpMessage();
      return 1;
    }
    if (args[3] != "in" && args[6] != "of") {
      PrintHelpMessage();
      return 1;
    }
    auto chunkX = strings::Toi(args[4]);
    if (!chunkX) {
      cerr << "Error: invalid chunk x: " << args[4] << endl;
      PrintHelpMessage();
      return 1;
    }
    auto chunkZ = strings::Toi(args[5]);
    if (!chunkZ) {
      cerr << "Error: invalid chunk z: " << args[5] << endl;
      PrintHelpMessage();
      return 1;
    }
    auto dimension = DimensionFromString(args[7]);
    if (!dimension) {
      cerr << "Error: invalid dimension: " << args[7] << endl;
      PrintHelpMessage();
      return 1;
    }
    return DumpEntities(dir, *chunkX, *chunkZ, *dimension);
  } else if (verb == "chunk-key") {
    if (argc != 9) {
      PrintHelpMessage();
      return 1;
    }
    if (args[4] == "in" && args[7] == "of") {
      auto tag = strings::Toi(args[3]);
      if (!tag) {
        cerr << "Error: invalid tag: " << args[3] << endl;
        PrintHelpMessage();
        return 1;
      }
      if (*tag < 0 || 255 < *tag) {
        cerr << "Error: invalid tag range: " << args[3] << endl;
        PrintHelpMessage();
        return 1;
      }
      auto chunkX = strings::Toi(args[5]);
      if (!chunkX) {
        cerr << "Error: invalid chunk x: " << args[5] << endl;
        PrintHelpMessage();
        return 1;
      }
      auto chunkZ = strings::Toi(args[6]);
      if (!chunkZ) {
        cerr << "Error: invalid chunk z: " << args[6] << endl;
        PrintHelpMessage();
        return 1;
      }
      auto dimension = DimensionFromString(args[8]);
      if (!dimension) {
        cerr << "Error: invalid dimension: " << args[8] << endl;
        PrintHelpMessage();
        return 1;
      }
      DumpChunkKey(dir, *chunkX, *chunkZ, *dimension, uint8_t(*tag));
      return 0;
    } else {
      PrintHelpMessage();
      return 1;
    }
  } else if (verb == "level.dat") {
    return DumpLevelDat(dir) ? 0 : 1;
  } else if (verb == "keys") {
    DumpAllKeys(dir);
    return 0;
  } else if (verb == "key") {
    if (args.size() < 4) {
      cerr << "Error: too few arguments" << endl;
      PrintHelpMessage();
      return 1;
    }
    auto key = args[3];
    DumpKey(dir, key);
    return 0;
  } else {
    PrintHelpMessage();
    return 1;
  }
}
