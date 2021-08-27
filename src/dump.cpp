#include <iostream>
#include <je2be.hpp>

#if __has_include(<shlobj_core.h>)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <shlobj_core.h>
#endif

using namespace std;
using namespace leveldb;
using namespace j2b;
using namespace mcfile;
using namespace nbt;
using namespace stream;
namespace fs = filesystem;

static optional<j2b::Dimension> DimensionFromString(string const &s) {
  if (s == "overworld" || s == "o" || s == "0") {
    return j2b::Dimension::Overworld;
  } else if (s == "nether" || s == "n" || s == "-1") {
    return j2b::Dimension::Nether;
  } else if (s == "end" || s == "e" || s == "1") {
    return j2b::Dimension::End;
  }
  return nullopt;
}

static void DumpBlock(string const &dbDir, int x, int y, int z, j2b::Dimension d) {
  Options o;
  o.compression = kZlibRawCompression;
  DB *db;
  Status st = DB::Open(o, dbDir, &db);
  if (!st.ok()) {
    return;
  }

  ReadOptions ro;
  nbt::JsonPrintOptions jopt;
  jopt.fTypeHint = true;

  int cx = Coordinate::ChunkFromBlock(x);
  int cy = Coordinate::ChunkFromBlock(y);
  int cz = Coordinate::ChunkFromBlock(z);
  auto key = Key::SubChunk(cx, cy, cz, d);
  string value;
  st = db->Get(ro, key, &value);
  if (!st.ok()) {
    return;
  }
  vector<uint8_t> buffer;
  copy(value.begin(), value.end(), back_inserter(buffer));
  auto stream = make_shared<ByteStream>(buffer);
  stream::InputStreamReader sr(stream, {.fLittleEndian = true});

  uint8_t version;
  if (!sr.read(&version)) {
    return;
  }

  uint8_t numLayers;
  if (!sr.read(&numLayers)) {
    return;
  }

  uint8_t bitsPerBlock;
  if (!sr.read(&bitsPerBlock)) {
    return;
  }
  bitsPerBlock = bitsPerBlock / 2;

  int blocksPerWord = 32 / bitsPerBlock;
  int numWords;
  if (4096 % blocksPerWord == 0) {
    numWords = 4096 / blocksPerWord;
  } else {
    numWords = (int)ceilf(4096.0 / blocksPerWord);
  }
  int numBytes = numWords * 4;
  vector<uint8_t> indexBuffer(numBytes);
  if (!sr.read(indexBuffer)) {
    return;
  }

  uint32_t const mask = ~((~((uint32_t)0)) << bitsPerBlock);
  vector<uint16_t> index;
  index.reserve(4096);
  auto indexBufferStream = make_shared<ByteStream>(indexBuffer);
  InputStreamReader sr2(indexBufferStream, {.fLittleEndian = true});
  for (int i = 0; i < numWords; i++) {
    uint32_t word;
    sr2.read(&word);
    for (int j = 0; j < blocksPerWord && index.size() < 4096; j++) {
      uint16_t v = word & mask;
      index.push_back(v);
      word = word >> bitsPerBlock;
    }
  }
  assert(index.size() == 4096);

  uint32_t numPaletteEntries;
  if (!sr.read(&numPaletteEntries)) {
    return;
  }

  vector<shared_ptr<CompoundTag>> palette;
  palette.reserve(numPaletteEntries);

  for (uint32_t i = 0; i < numPaletteEntries; i++) {
    auto tag = make_shared<CompoundTag>();
    uint8_t type;
    sr.read(&type);
    string empty;
    sr.read(empty);
    tag->read(sr);
    palette.push_back(tag);
  }

  int lx = x - cx * 16;
  int ly = y - cy * 16;
  int lz = z - cz * 16;
  size_t idx = (lx * 16 + lz) * 16 + ly;
  uint16_t paletteIndex = index[idx];
  auto tag = palette[paletteIndex];
  nbt::PrintAsJson(cout, *tag, jopt);

  delete db;
}

static void DumpBlockEntity(string const &dbDir, int x, int y, int z, j2b::Dimension d) {
  Options o;
  o.compression = kZlibRawCompression;
  DB *db;
  Status st = DB::Open(o, dbDir, &db);
  if (!st.ok()) {
    cerr << "Error: cannot open db: dbDir=" << dbDir << endl;
    return;
  }

  ReadOptions ro;
  nbt::JsonPrintOptions jopt;
  jopt.fTypeHint = true;

  int cx = Coordinate::ChunkFromBlock(x);
  int cy = Coordinate::ChunkFromBlock(y);
  int cz = Coordinate::ChunkFromBlock(z);
  auto key = Key::BlockEntity(cx, cz, d);

  string value;
  st = db->Get(ro, key, &value);
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

  delete db;
}

static void DumpKey(string const &dbDir, string const &key) {
  Options o;
  o.compression = kZlibRawCompression;
  DB *db;
  Status st = DB::Open(o, dbDir, &db);
  if (!st.ok()) {
    return;
  }

  ReadOptions ro;
  nbt::JsonPrintOptions jopt;
  jopt.fTypeHint = true;

  string value;
  st = db->Get(ro, key, &value);
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

  delete db;
}

static void DumpChunkKey(string const &dbDir, int cx, int cz, Dimension d, uint8_t tag) {
  auto key = Key::ComposeChunkKey(cx, cz, d, tag);
  DumpKey(dbDir, key);
}

static bool DumpLevelDat(string const &dbDir) {
  auto datFile = fs::path(dbDir).parent_path() / "level.dat";
  if (!fs::is_regular_file(datFile)) {
    cerr << "Error: " << datFile << " does not exist" << endl;
    return false;
  }
  auto stream = make_shared<FileInputStream>(datFile.string());
  InputStreamReader reader(stream, {.fLittleEndian = true});
  auto tag = make_shared<CompoundTag>();
  stream->seek(8);
  tag->read(reader);

  JsonPrintOptions o;
  o.fTypeHint = true;
  PrintAsJson(cout, *tag, o);
}

static bool PrintChunkKeyDescription(uint8_t tag, int32_t cx, int32_t cz, string dimension) {
  switch (tag) {
  case 0x2c:
    cout << "ChunkVersion(0x2c) [" << cx << ", " << cz << "] " << dimension << endl;
    break;
  case 0x2d:
    cout << "Data2D(0x2d) [" << cx << ", " << cz << "] " << dimension << endl;
    break;
  case 0x31:
    cout << "BlockEntity(0x31) [" << cx << ", " << cz << "] " << dimension << endl;
    break;
  case 0x32:
    cout << "Entity(0x32) [" << cx << ", " << cz << "] " << dimension << endl;
    break;
  case 0x33:
    cout << "PendingTicks(0x33) [" << cx << ", " << cz << "] " << dimension << endl;
    break;
  case 0x35:
    cout << "BiomeState(0x35) [" << cx << ", " << cz << "] " << dimension << endl;
    break;
  case 0x36:
    cout << "FinalizedState(0x36) [" << cx << ", " << cz << "] " << dimension << endl;
    break;
  case 0x39:
    cout << "StructureBounds(0x39) [" << cx << ", " << cz << "] " << dimension << endl;
    break;
  case 0x3a:
    cout << "RandomTicks(0x3a) [" << cx << ", " << cz << "] " << dimension << endl;
    break;
  case 0x3b:
    cout << "Checksums(0x3b) [" << cx << ", " << cz << "] " << dimension << endl;
    break;
  case 0x76:
    cout << "ChunkVersionLegacy(0x76) [" << cx << ", " << cz << "] " << dimension << endl;
    break;
  default:
    return false;
  }
  return true;
}

static string StringFromDimension(int32_t d) {
  if (d == 1) {
    return "end";
  } else if (d == -1) {
    return "nether";
  } else {
    return "(unknown: " + to_string(d) + ")";
  }
}

static void DumpAllKeys(string const &dbDir) {
  Options o;
  o.compression = kZlibRawCompression;
  DB *db;
  Status st = DB::Open(o, dbDir, &db);
  if (!st.ok()) {
    return;
  }

  ReadOptions ro;
  Iterator *itr = db->NewIterator(ro);
  for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
    auto key = itr->key();
    bool chunkTag = true;
    switch (key.size()) {
    case 9: {
      uint8_t tag = key[8];
      int32_t cx = *(int32_t *)key.data();
      int32_t cz = *(int32_t *)(key.data() + 4);
      chunkTag = PrintChunkKeyDescription(tag, cx, cz, "overworld");
      break;
    }
    case 10: {
      uint8_t tag = key[8];
      if (tag == 0x2f) {
        int32_t cx = *(int32_t *)key.data();
        int32_t cz = *(int32_t *)(key.data() + 4);
        uint8_t y = key[9];
        cout << "SubChunk(0x2f) [" << cx << ", " << cz << "] y=" << (int)y << " overworld" << endl;
      } else {
        chunkTag = false;
      }
      break;
    }
    case 13: {
      uint8_t tag = key[12];
      int32_t cx = *(int32_t *)key.data();
      int32_t cz = *(int32_t *)(key.data() + 4);
      string dimension = StringFromDimension(*(int32_t *)(key.data() + 8));
      chunkTag = PrintChunkKeyDescription(tag, cz, cz, dimension);
      break;
    }
    case 14: {
      uint8_t tag = key[12];
      if (tag == 0x2f) {
        int32_t cx = *(int32_t *)key.data();
        int32_t cz = *(int32_t *)(key.data() + 4);
        string dimension = StringFromDimension(*(int32_t *)(key.data() + 8));
        uint8_t y = key[13];
        cout << "SubChunk(0x2f) [" << cx << ", " << cz << "] y=" << (int)y << " " << dimension << endl;
      } else {
        chunkTag = false;
      }
      break;
    }
    default:
      chunkTag = false;
      break;
    }
    if (!chunkTag) {
      cout << key.ToString() << endl;
    }
  }
  delete itr;
}

static optional<string> GetLocalApplicationDirectory() {
#if __has_include(<shlobj_core.h>)
  int csidType = CSIDL_LOCAL_APPDATA;
  char path[MAX_PATH + 256];

  if (SHGetSpecialFolderPathA(nullptr, path, csidType, FALSE)) {
    return string(path);
  }
#endif
  return nullopt;
}

static void PrintHelpMessage() {
  cerr << R"(dump.exe [world-dir] block at [x] [y] [z] of ["overworld" | "nether" | "end"])" << endl;
  cerr << R"(dump.exe [world-dir] block entity at [x] [y] [z] of ["overworld" | "nether" | "end"])" << endl;
  cerr << R"(dump.exe [world-dir] chunk-key [tag:uint8_t] in [chunkX] [chunkZ] of ["overworld" | "nether" | "end"])" << endl;
  cerr << "    tag:" << endl;
  cerr << "       44: ChunkVersion" << endl;
  cerr << "       45: Data2D" << endl;
  cerr << "       46: Data2DLegacy" << endl;
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
  cerr << "       59: Checksums" << endl;
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
  string dir = d;
  if (!fs::exists(fs::path(dir))) {
    dir = d + "/db";
  }
  if (!fs::exists(fs::path(dir))) {
    auto appDir = GetLocalApplicationDirectory(); // X:/Users/whoami/AppData/Local
    if (!appDir) {
      cerr << "Error: cannot get AppData directory" << endl;
      return 1;
    }
    auto p = fs::path(*appDir) / "Packages" / "Microsoft.MinecraftUWP_8wekyb3d8bbwe" / "LocalState" / "games" / "com.mojang" / "minecraftWorlds" / d / "db";
    dir = p.string();
  }
  if (!fs::exists(fs::path(dir))) {
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