#pragma once

namespace je2be::tobe {

class Datapacks {
public:
  struct Pack {
    std::string fNamespace;
    Uuid fId;
    int fVersion[3];
  };

  static bool Import(std::filesystem::path jeRoot, std::filesystem::path beRoot) {
    using namespace std;
    namespace fs = std::filesystem;
    auto datapacks = jeRoot / "datapacks";
    auto exists = Fs::Exists(datapacks);
    if (!exists) {
      return true;
    }

    auto itr = Fs::DirectoryIterator(datapacks);
    if (!itr) {
      return false;
    }
    std::vector<Pack> packs;
    for (auto const &item : *itr) {
      if (!item.is_directory()) {
        continue;
      }
      if (!ImportPack(item.path(), beRoot, packs)) {
        return false;
      }
    }
    if (packs.empty()) {
      return true;
    }
    ostringstream s;
    s << "[" << endl;
    for (size_t i = 0; i < packs.size(); i++) {
      Pack const &pack = packs[i];
      s << "  {" << endl;
      s << "    \"pack_id\": \"" << pack.fId.toString() << "\"," << endl;
      s << "    \"version\": [" << pack.fVersion[0] << ", " << pack.fVersion[1] << ", " << pack.fVersion[2] << "]" << endl;
      s << "  }";
      if (i + 1 < packs.size()) {
        s << ",";
      }
      s << endl;
    }
    s << "]" << endl;
    ScopedFile fp(mcfile::File::Open(beRoot / "world_behavior_packs.json", mcfile::File::Mode::Write));
    if (!fp) {
      return false;
    }
    string json = s.str();
    if (!mcfile::File::Fwrite(json.c_str(), json.size(), 1, fp)) {
      return false;
    }
    return true;
  }

private:
  Datapacks() = delete;

  static bool ImportPack(std::filesystem::path packRootDir, std::filesystem::path beRoot, std::vector<Pack> &packs) {
    using namespace std;

    if (!Fs::Exists(packRootDir / "data")) {
      return true;
    }

    auto packsDir = beRoot / "behavior_packs";
    if (!Fs::Exists(packsDir)) {
      if (!Fs::CreateDirectories(packsDir)) {
        return false;
      }
    }

    auto itr = Fs::DirectoryIterator(packRootDir / "data");
    if (!itr) {
      return false;
    }

    auto packName = packRootDir.filename();
    auto packDir = packsDir / packName;
    if (!Fs::CreateDirectories(packDir / "functions")) {
      return false;
    }

    std::mt19937 mt;
    for (auto const &item : *itr) {
      if (!item.is_directory()) {
        continue;
      }
      string nameSpace = item.path().filename().string();
      if (!Fs::CreateDirectories(packDir / "functions" / nameSpace)) {
        return false;
      }
      auto mcfunctionItr = Fs::DirectoryIterator(item.path() / "functions");
      if (!mcfunctionItr) {
        continue;
      }
      auto packId = Uuid::Gen();
      auto moduleId = Uuid::Gen();
      for (auto const &mcfunction : *mcfunctionItr) {
        if (!mcfunction.is_regular_file()) {
          continue;
        }
        auto path = mcfunction.path();
        auto fileName = path.filename();
        if (!fileName.has_extension()) {
          continue;
        }
        string ext = fileName.extension().string();
        if (!strings::Iequals(ext, ".mcfunction")) {
          continue;
        }
        auto to = packDir / "functions" / nameSpace / path.filename();
        if (Fs::Exists(to)) {
          if (!Fs::Delete(to)) {
            return false;
          }
        }
        if (!Fs::CopyFile(path, to)) {
          return false;
        }
      }
      Pack pack;
      pack.fNamespace = nameSpace;
      pack.fId = packId;
      pack.fVersion[0] = 0;
      pack.fVersion[1] = 0;
      pack.fVersion[2] = 1;
      packs.push_back(pack);
      ostringstream s;
      s << R"({)" << endl;
      s << R"(  "format_version": 2,)" << endl;
      s << R"(  "header": {)" << endl;
      s << R"(    "description": ")" << nameSpace << R"(",)" << endl;
      s << R"(    "name": ")" << nameSpace << R"(",)" << endl;
      s << R"(    "uuid": ")" << packId.toString() << R"(",)" << endl;
      s << R"(    "version": [)" << pack.fVersion[0] << ", " << pack.fVersion[1] << ", " << pack.fVersion[2] << R"(],)" << endl;
      s << R"(    "min_engine_version": [1, 17, 40])" << endl;
      s << R"(  },)" << endl;
      s << R"(  "modules": [)" << endl;
      s << R"(    {)" << endl;
      s << R"(      "description": ")" << nameSpace << R"(",)" << endl;
      s << R"(      "type": "data",)" << endl;
      s << R"(      "uuid": ")" << moduleId.toString() << R"(",)" << endl;
      s << R"(      "version": [0, 0, 1])" << endl;
      s << R"(    })" << endl;
      s << R"(  ])" << endl;
      s << R"(})" << endl;
      string json = s.str();
      ScopedFile fp(mcfile::File::Open(packDir / "manifest.json", mcfile::File::Mode::Write));
      if (!fp) {
        return false;
      }
      if (!mcfile::File::Fwrite(json.c_str(), json.size(), 1, fp)) {
        return false;
      }
    }
    return true;
  }
};

} // namespace je2be::tobe
