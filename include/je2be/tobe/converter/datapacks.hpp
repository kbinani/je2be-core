#pragma once

namespace je2be::tobe {

class Datapacks {
public:
  struct Pack {
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

    vector<pair<std::string, Uuid>> modules;

    for (auto const &item : *itr) {
      if (!item.is_directory()) {
        continue;
      }
      string moduleName = item.path().filename().string();
      if (!Fs::CreateDirectories(packDir / "functions" / moduleName)) {
        return false;
      }
      auto mcfunctionItr = Fs::DirectoryIterator(item.path() / "functions");
      if (!mcfunctionItr) {
        continue;
      }
      bool hasMcfunction = false;
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
        auto to = packDir / "functions" / moduleName / path.filename();
        if (!ConvertFunction(path, to)) {
          return false;
        }
        hasMcfunction = true;
      }
      if (hasMcfunction) {
        size_t hash = std::hash<std::string>()(moduleName);
        Uuid moduleId = Uuid::GenWithSeed(hash);
        modules.push_back(make_pair(moduleName, moduleId));
      }
    }

    if (modules.empty()) {
      return true;
    }

    string description = packName.string();
    auto mcmeta = packRootDir / "pack.mcmeta";
    auto desc = ReadDescriptionFromMcmeta(mcmeta);
    if (desc) {
      description = *desc;
    }

    size_t packNameHash = std::hash<std::string>()(packName.string());
    auto packId = Uuid::GenWithSeed(packNameHash);

    Pack pack;
    pack.fId = packId;
    pack.fVersion[0] = 0;
    pack.fVersion[1] = 0;
    pack.fVersion[2] = 1;
    packs.push_back(pack);
    ostringstream s;
    s << R"({)" << endl;
    s << R"(  "format_version": 2,)" << endl;
    s << R"(  "header": {)" << endl;
    s << R"(    "description": ")" << description << R"(",)" << endl;
    s << R"(    "name": ")" << packName.string() << R"(",)" << endl;
    s << R"(    "uuid": ")" << packId.toString() << R"(",)" << endl;
    s << R"(    "version": [)" << pack.fVersion[0] << ", " << pack.fVersion[1] << ", " << pack.fVersion[2] << R"(],)" << endl;
    s << R"(    "min_engine_version": [1, 17, 40])" << endl;
    s << R"(  },)" << endl;
    s << R"(  "modules": [)" << endl;
    for (size_t i = 0; i < modules.size(); i++) {
      auto const &it = modules[i];
      string name = it.first;
      Uuid moduleId = it.second;
      s << R"(    {)" << endl;
      s << R"(      "description": ")" << name << R"(",)" << endl;
      s << R"(      "type": "data",)" << endl;
      s << R"(      "uuid": ")" << moduleId.toString() << R"(",)" << endl;
      s << R"(      "version": [0, 0, 1])" << endl;
      s << R"(    })";
      if (i + 1 < modules.size()) {
        s << ",";
      }
      s << endl;
    }
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
    return true;
  }

  static std::optional<std::string> ReadDescriptionFromMcmeta(std::filesystem::path mcmeta) {
    using namespace std;
    std::vector<uint8_t> buffer;
    if (!file::GetContents(mcmeta, buffer)) {
      return nullopt;
    }
    nlohmann::json json;
    try {
      json = nlohmann::json::parse(buffer.begin(), buffer.end());
      return json["pack"]["description"];
    } catch (...) {
      return nullopt;
    }
  }

  static bool ConvertFunction(std::filesystem::path from, std::filesystem::path to) {
    using namespace std;
    using namespace mcfile;

    if (Fs::Exists(to)) {
      if (!Fs::Delete(to)) {
        return false;
      }
    }
    vector<uint8_t> buffer;
    if (!file::GetContents(from, buffer)) {
      return false;
    }
    std::string content(buffer.begin(), buffer.end());
    buffer.clear();
    vector<string> lines = String::Split(content, '\x0a');
    ostringstream s;
    for (auto const &line : lines) {
      auto hasCr = line.ends_with("\x0d");
      s << command::Command::TranspileJavaToBedrock(line);
      s << "\x0a";
    }
    string transpiled = s.str();
    ScopedFile fp(File::Open(to, File::Mode::Write));
    if (!fp) {
      return false;
    }
    if (!File::Fwrite(transpiled.c_str(), transpiled.size(), 1, fp)) {
      return false;
    }
    return true;
  }
};

} // namespace je2be::tobe
