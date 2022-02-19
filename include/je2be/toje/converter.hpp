#pragma once

namespace je2be::toje {

class Converter {
public:
  Converter(std::string const &input, std::string const &output) = delete;
  Converter(std::string const &input, std::wstring const &output) = delete;
  Converter(std::wstring const &input, std::string const &output) = delete;
  Converter(std::wstring const &input, std::wstring const &output) = delete;
  Converter(std::filesystem::path const &input, std::filesystem::path const &output)
      : fInput(input), fOutput(output) {
  }

  bool run(unsigned concurrency, Progress *progress = nullptr) {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;
    namespace fs = std::filesystem;

    unique_ptr<DB> db(Open(fInput / "db"));
    if (!db) {
      return false;
    }

    if (!prepareOutputDirectory()) {
      return false;
    }

    auto dat = LevelData::Read(fInput / "level.dat");
    if (!dat) {
      return false;
    }

    int total = 0;
    map<Dimension, unordered_map<Pos2i, Context::ChunksInRegion, Pos2iHasher>> regions;
    auto bin = Context::Init(*db, regions, total);

    auto levelDat = LevelData::Import(*dat, *db, *bin);
    if (!levelDat) {
      return false;
    }
    if (!LevelData::Write(*levelDat, fOutput / "level.dat")) {
      return false;
    }

    atomic<int> done = 0;
    atomic<bool> cancelRequested = false;
    auto reportProgress = [progress, &done, total, &cancelRequested]() -> bool {
      int d = done.fetch_add(1);
      if (progress) {
        bool ok = progress->report(d, total);
        if (!ok) {
          cancelRequested = true;
        }
        return ok;
      } else {
        return true;
      }
    };

    for (Dimension d : {Dimension::Overworld, Dimension::Nether, Dimension::End}) {
      auto result = World::Convert(d, regions[d], *db, fOutput, concurrency, *bin, reportProgress);
      if (result) {
        result->mergeInto(*bin);
      } else {
        return false;
      }
      if (cancelRequested.load()) {
        return false;
      }
    }

    return bin->postProcess(fOutput, *db);
  }

private:
  bool prepareOutputDirectory() {
    using namespace std;
    namespace fs = std::filesystem;
    error_code ec;
    fs::create_directories(fOutput, ec);
    if (ec) {
      return false;
    }
    ec.clear();

    fs::directory_iterator iterator(fOutput, ec);
    if (ec) {
      return false;
    }
    ec.clear();
    for (auto it : iterator) {
      fs::remove_all(it.path(), ec);
      ec.clear();
    }
    return true;
  }

  static leveldb::DB *Open(std::filesystem::path name) {
    using namespace leveldb;
    Options o;
    o.compression = kZlibRawCompression;
    DB *db;
    Status st = DB::Open(o, name, &db);
    if (!st.ok()) {
      return nullptr;
    }
    return db;
  }

private:
  std::filesystem::path fInput;
  std::filesystem::path fOutput;
};

} // namespace je2be::toje
