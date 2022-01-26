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
using namespace mcfile;
using namespace nbt;
using namespace stream;
namespace fs = filesystem;

static optional<Dimension> DimensionFromString(string const &s) {
  if (s == "overworld" || s == "o" || s == "0") {
    return Dimension::Overworld;
  } else if (s == "nether" || s == "n" || s == "-1") {
    return Dimension::Nether;
  } else if (s == "end" || s == "e" || s == "1") {
    return Dimension::End;
  }
  return nullopt;
}

static DB *Open(fs::path dir) {
  Options o;
  o.compression = kZlibRawCompression;
  DB *db = nullptr;
  Status st = DB::Open(o, dir, &db);
  if (!st.ok()) {
    return nullptr;
  }
  return db;
}

static void DumpBlock(fs::path const &dbDir, int x, int y, int z, Dimension d) {
  unique_ptr<DB> db(Open(dbDir));
  if (!db) {
    return;
  }

  ReadOptions ro;
  nbt::JsonPrintOptions jopt;
  jopt.fTypeHint = true;

  int cx = Coordinate::ChunkFromBlock(x);
  int cy = Coordinate::ChunkFromBlock(y);
  int cz = Coordinate::ChunkFromBlock(z);
  auto key = mcfile::be::DbKey::SubChunk(cx, cy, cz, d);
  string value;
  Status st = db->Get(ro, key, &value);
  if (!st.ok()) {
    return;
  }
  auto section = mcfile::be::SubChunk::Parse(value, cy);
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

  auto tag = make_shared<CompoundTag>();
  tag->set("name", props::String(block->fName));
  auto states = make_shared<CompoundTag>();
  states = block->fStates;
  tag->set("states", states);
  tag->set("version", props::Int(block->fVersion));
  nbt::PrintAsJson(cout, *tag, jopt);
  if (section->fWaterPaletteIndices.size() == 4096) {
    auto index = mcfile::be::SubChunk::BlockIndex(lx, ly, lz);
    auto idx = section->fWaterPaletteIndices[index];
    cout << "layer2 ---" << endl;
    auto layer2Block = section->fWaterPalette[idx];
    auto t = make_shared<CompoundTag>();
    t->set("name", props::String(layer2Block->fName));
    t->set("states", layer2Block->fStates);
    t->set("version", props::Int(layer2Block->fVersion));
    nbt::PrintAsJson(cout, *t, jopt);
  }
}

static void DumpBlockEntity(fs::path const &dbDir, int x, int y, int z, Dimension d) {
  unique_ptr<DB> db(Open(dbDir));
  if (!db) {
    cerr << "Error: cannot open db: dbDir=" << dbDir << endl;
    return;
  }

  ReadOptions ro;
  nbt::JsonPrintOptions jopt;
  jopt.fTypeHint = true;

  int cx = Coordinate::ChunkFromBlock(x);
  int cy = Coordinate::ChunkFromBlock(y);
  int cz = Coordinate::ChunkFromBlock(z);
  auto key = mcfile::be::DbKey::BlockEntity(cx, cz, d);

  string value;
  Status st = db->Get(ro, key, &value);
  if (!st.ok()) {
    cerr << "Error: cannot get Key::BlockEntity(" << cx << ", " << cz << ", " << (int)d << ")" << endl;
    return;
  }
  vector<uint8_t> buffer;
  copy(value.begin(), value.end(), back_inserter(buffer));
  auto stream = make_shared<ByteStream>(buffer);
  stream::InputStreamReader sr(stream, {.fLittleEndian = true});
  while (true) {
    uint8_t type;
    if (!sr.read(&type)) {
      break;
    }
    string name;
    if (!sr.read(name)) {
      break;
    }
    auto tag = make_shared<CompoundTag>();
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
  nbt::JsonPrintOptions jopt;
  jopt.fTypeHint = true;

  string value;
  Status st = db->Get(ro, key, &value);
  if (!st.ok()) {
    return;
  }
  vector<uint8_t> buffer;
  copy(value.begin(), value.end(), back_inserter(buffer));
  auto stream = make_shared<ByteStream>(buffer);
  stream::InputStreamReader sr(stream, {.fLittleEndian = true});
  while (true) {
    uint8_t type;
    if (!sr.read(&type)) {
      break;
    }
    string name;
    if (!sr.read(name)) {
      break;
    }
    auto tag = make_shared<CompoundTag>();
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
  nbt::JsonPrintOptions jopt;
  jopt.fTypeHint = true;

  string value;
  Status st = db->Get(ro, key, &value);
  if (!st.ok()) {
    return;
  }
#if defined(_WIN32)
  _setmode(_fileno(stdout), _O_BINARY);
#endif
  std::cout << value;
}

static void DumpChunkKey(fs::path const &dbDir, int cx, int cz, Dimension d, uint8_t tag) {
  auto key = mcfile::be::DbKey::ComposeChunkKey(cx, cz, d, tag);
  using Tag = mcfile::be::DbKey::Tag;
  if (tag == static_cast<uint8_t>(Tag::StructureBounds) || tag == static_cast<uint8_t>(Tag::ChecksumsLegacy) || tag == static_cast<uint8_t>(Tag::Data2D) || tag == static_cast<uint8_t>(Tag::Data2DLegacy) || tag == static_cast<uint8_t>(Tag::Version)) {
    DumpBinaryKey(dbDir, key);
  } else {
    DumpKey(dbDir, key);
  }
}

static bool DumpLevelDat(fs::path const &dbDir) {
  auto datFile = dbDir.parent_path() / L"level.dat";
  if (!fs::is_regular_file(datFile)) {
    cerr << "Error: " << datFile << " does not exist" << endl;
    return false;
  }
  auto stream = make_shared<FileInputStream>(datFile);
  InputStreamReader reader(stream, {.fLittleEndian = true});
  auto tag = make_shared<CompoundTag>();
  stream->seek(8);
  tag->read(reader);

  JsonPrintOptions o;
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
    auto parsed = be::DbKey::Parse(key);
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
