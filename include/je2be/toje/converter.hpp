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

  bool run(unsigned concurrency) {
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

    auto levelDat = LevelData::Import(fInput / "level.dat");
    if (!levelDat) {
      return false;
    }
    if (!levelDat->write(fOutput / "level.dat")) {
      return false;
    }

    for (Dimension d : {Dimension::Overworld, Dimension::Nether, Dimension::End}) {
      if (!World::Convert(d, *db, fOutput, concurrency)) {
        return false;
      }
    }

    return true;
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
