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

static void DumpChunkKey(string const &dbDir, int cx, int cz, Dimension d, Key::Tag tag) {
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
  cerr << R"("dump.exe [world-dir] block at [x] [y] [z] of ["overworld" | "nether" | "end"])" << endl;
  cerr << R"("dump.exe [world-dir] block entity at [x] [y] [z] of ["overworld" | "nether" | "end"])" << endl;
  cerr << R"("dump.exe [world-dir] entity in [chunkX] [chunkZ] of ["overworld" | "nether" | "end"])" << endl;
  cerr << R"("dump.exe [world-dir] pending ticks in [chunkX] [chunkZ] of ["overworld" | "nether" | "end"])" << endl;
  cerr << R"("dump.exe [world-dir] level.dat)" << endl;
  cerr << R"("dump.exe [world-dir] <any-string>)" << endl;
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
  } else if (verb == "entity") {
    if (argc != 8) {
      PrintHelpMessage();
      return 1;
    }
    if (args[3] == "in" && args[6] == "of") {
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
      DumpChunkKey(dir, *chunkX, *chunkZ, *dimension, Key::Tag::Entity);
    } else {
      PrintHelpMessage();
      return 1;
    }
  } else if (verb == "level.dat") {
    return DumpLevelDat(dir) ? 0 : 1;
  } else if (verb == "pending") {
    if (argc != 9) {
      PrintHelpMessage();
      return 1;
    }
    if (args[3] != "ticks" || args[4] != "in" || args[7] != "of") {
      PrintHelpMessage();
      return 1;
    }
    auto cx = strings::Toi(args[5]);
    if (!cx) {
      cerr << "Error: invalid chunk x: " << args[5] << endl;
    }
    auto cz = strings::Toi(args[6]);
    if (!cz) {
      cerr << "Error: invalid chunk z: " << args[6] << endl;
    }
    auto dimension = DimensionFromString(args[8]);
    if (!dimension) {
      cerr << "Error: invalid dimension: " << args[7] << endl;
      PrintHelpMessage();
      return 1;
    }
    DumpChunkKey(dir, *cx, *cz, *dimension, Key::Tag::PendingTicks);
  } else if (args.size() == 3) {
    auto key = verb;
    DumpKey(dir, key);
    return 0;
  } else {
    PrintHelpMessage();
    return 1;
  }
}
