#pragma once

#include <je2be/fs.hpp>
#include <je2be/uuid.hpp>

#include "_directory-iterator.hpp"
#include "_file.hpp"
#include "_props.hpp"
#include "command/_command.hpp"

namespace je2be::java {

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

    std::vector<Pack> packs;
    for (DirectoryIterator itr(datapacks); itr.valid(); itr.next()) {
      if (!itr->is_directory()) {
        continue;
      }
      if (!ImportPack(itr->path(), beRoot, packs)) {
        return false;
      }
    }
    if (packs.empty()) {
      return true;
    }
    u8string json;
    json += u8"[\xa";
    for (size_t i = 0; i < packs.size(); i++) {
      Pack const &pack = packs[i];
      json += u8"  {\xa";
      json += u8"    \"pack_id\": \"" + pack.fId.toString() + u8"\",\xa";
      json += u8"    \"version\": [" + mcfile::String::ToString(pack.fVersion[0]) + u8", " + mcfile::String::ToString(pack.fVersion[1]) + u8", " + mcfile::String::ToString(pack.fVersion[2]) + u8"]\xa";
      json += u8"  }";
      if (i + 1 < packs.size()) {
        json += u8",";
      }
      json += u8"\xa";
    }
    json += u8"]\xa";
    mcfile::ScopedFile fp(mcfile::File::Open(beRoot / "world_behavior_packs.json", mcfile::File::Mode::Write));
    if (!fp) {
      return false;
    }
    if (!mcfile::File::Fwrite(json.c_str(), json.size(), 1, fp.get())) {
      return false;
    }
    return true;
  }

private:
  Datapacks() = delete;

  static bool ImportPack(std::filesystem::path packRootDir, std::filesystem::path beRoot, std::vector<Pack> &packs) {
    using namespace std;
    using namespace props;

    if (!Fs::Exists(packRootDir / "data")) {
      return true;
    }

    auto packsDir = beRoot / "behavior_packs";
    if (!Fs::Exists(packsDir)) {
      if (!Fs::CreateDirectories(packsDir)) {
        return false;
      }
    }

    auto packName = packRootDir.filename();
    auto packDir = packsDir / packName;
    if (!Fs::CreateDirectories(packDir / "functions")) {
      return false;
    }

    vector<pair<std::u8string, Uuid>> modules;

    for (DirectoryIterator itr(packRootDir / "data"); itr.valid(); itr.next()) {
      if (!itr->is_directory()) {
        continue;
      }
      u8string moduleName = itr->path().filename().u8string();
      if (!Fs::CreateDirectories(packDir / "functions" / moduleName)) {
        return false;
      }
      bool hasMcfunction = false;
      for (DirectoryIterator mcfunctionItr(itr->path() / "function"); mcfunctionItr.valid(); mcfunctionItr.next()) {
        if (!mcfunctionItr->is_regular_file()) {
          continue;
        }
        auto path = mcfunctionItr->path();
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
        size_t hash = std::hash<std::u8string>()(moduleName);
        Uuid moduleId = Uuid::GenWithSeed(hash);
        modules.push_back(make_pair(moduleName, moduleId));
      }
    }

    if (modules.empty()) {
      return true;
    }

    u8string description = packName.u8string();
    auto mcmeta = packRootDir / "pack.mcmeta";
    auto desc = ReadDescriptionFromMcmeta(mcmeta);
    if (desc) {
      description = *desc;
    }

    size_t packNameHash = std::hash<std::u8string>()(packName.u8string());
    auto packId = Uuid::GenWithSeed(packNameHash);

    Pack pack;
    pack.fId = packId;
    pack.fVersion[0] = 0;
    pack.fVersion[1] = 0;
    pack.fVersion[2] = 1;
    packs.push_back(pack);

    Json root;
    root["format_version"] = 2;

    Json header;
    SetJsonString(header, u8"description", description);
    SetJsonString(header, u8"name", packName.u8string());
    SetJsonString(header, u8"uuid", packId.toString());
    Json version = Json::array();
    version.push_back(pack.fVersion[0]);
    version.push_back(pack.fVersion[1]);
    version.push_back(pack.fVersion[2]);
    header["version"] = version;

    Json minEngineVersion = Json::array();
    minEngineVersion.push_back(1);
    minEngineVersion.push_back(17);
    minEngineVersion.push_back(40);
    header["min_engine_version"] = minEngineVersion;

    root["header"] = header;

    Json modulesArray = Json::array();
    for (size_t i = 0; i < modules.size(); i++) {
      auto const &it = modules[i];
      u8string name = it.first;
      Uuid moduleId = it.second;

      Json moduleObj;
      SetJsonString(moduleObj, u8"description", name);
      moduleObj["type"] = "data";
      SetJsonString(moduleObj, u8"uuid", moduleId.toString());

      Json moduleVersion = Json::array();
      moduleVersion.push_back(0);
      moduleVersion.push_back(0);
      moduleVersion.push_back(1);
      moduleObj["version"] = moduleVersion;

      modulesArray.push_back(moduleObj);
    }
    root["modules"] = modulesArray;

    u8string json = StringFromJson(root);
    mcfile::ScopedFile fp(mcfile::File::Open(packDir / "manifest.json", mcfile::File::Mode::Write));
    if (!fp) {
      return false;
    }
    if (!mcfile::File::Fwrite(json.c_str(), json.size(), 1, fp.get())) {
      return false;
    }
    return true;
  }

  static std::optional<std::u8string> ReadDescriptionFromMcmeta(std::filesystem::path mcmeta) {
    using namespace std;
    std::vector<u8> buffer;
    if (!file::GetContents(mcmeta, buffer)) {
      return nullopt;
    }
    props::Json json;
    try {
      json = props::Json::parse(buffer.begin(), buffer.end());
      return props::GetJsonStringValue(json["pack"]["description"]);
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
    vector<u8> buffer;
    if (!file::GetContents(from, buffer)) {
      return false;
    }
    std::u8string content(buffer.begin(), buffer.end());
    buffer.clear();
    vector<u8string> lines = String::Split(content, u8'\x0a');
    u8string transpiled;
    for (auto const &line : lines) {
      transpiled += command::Command::TranspileJavaToBedrock(line);
      transpiled += u8"\x0a";
    }
    ScopedFile fp(File::Open(to, File::Mode::Write));
    if (!fp) {
      return false;
    }
    if (!File::Fwrite(transpiled.c_str(), transpiled.size(), 1, fp.get())) {
      return false;
    }
    return true;
  }
};

} // namespace je2be::java
