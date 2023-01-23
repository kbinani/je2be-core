#pragma once

#include <je2be/nbt.hpp>
#include <je2be/strings.hpp>
#include <je2be/tobe/options.hpp>

#include <cstdint>
#include <optional>
#include <unordered_map>

namespace je2be::tobe {

class JavaEditionMap {
public:
  explicit JavaEditionMap(std::unordered_map<i32, i8> const &lookupTable) : fScaleLookupTable(lookupTable) {}
  JavaEditionMap(std::filesystem::path const &input, Options const &opt) : fScaleLookupTable(CreateScaleLookupTable(input, opt)) {}

  std::optional<i8> scale(i32 mapId) const {
    auto found = fScaleLookupTable.find(mapId);
    if (found == fScaleLookupTable.end()) {
      return std::nullopt;
    }
    return found->second;
  }

  [[nodiscard]] bool each(std::function<bool(i32 /* javaMapId */)> cb) {
    for (auto it : fScaleLookupTable) {
      if (!cb(it.first)) {
        return false;
      }
    }
    return true;
  }

  static CompoundTagPtr Read(std::filesystem::path const &input, Options const &opt, i32 mapId) {
    using namespace std;
    namespace fs = std::filesystem;
    using namespace mcfile::stream;

    auto jeFilePath = opt.getDataDirectory(input) / ("map_" + to_string(mapId) + ".dat");
    error_code ec;
    if (!fs::is_regular_file(jeFilePath, ec)) {
      return nullptr;
    }
    if (ec) {
      return nullptr;
    }
    auto tag = Compound();
    auto s = make_shared<GzFileInputStream>(jeFilePath);
    InputStreamReader r(s);
    if (!tag->read(r)) {
      return nullptr;
    }
    return tag;
  }

private:
  static std::unordered_map<i32, i8> CreateScaleLookupTable(std::filesystem::path const &input, Options const &opt) {
    namespace fs = std::filesystem;
    std::unordered_map<i32, i8> table;

    auto dataDir = opt.getDataDirectory(input);
    if (!fs::exists(dataDir)) {
      return table;
    }

    std::error_code ec;
    fs::directory_iterator itr(dataDir, ec);
    if (ec) {
      return table;
    }
    for (auto const &f : itr) {
      ec.clear();
      if (!fs::is_regular_file(f.path(), ec)) {
        continue;
      }
      if (ec) {
        continue;
      }
      auto name = f.path().filename().string();
      if (!name.starts_with("map_") || !name.ends_with(".dat")) {
        continue;
      }
      auto numberStr = strings::Trim("map_", name, ".dat");
      auto number = strings::Toi(numberStr);
      if (!number) {
        continue;
      }

      auto map = Read(input, opt, *number);
      if (!map) {
        continue;
      }
      auto data = map->query("/data")->asCompound();
      if (!data) {
        continue;
      }

      auto scale = data->byte("scale");
      if (!scale) {
        continue;
      }

      table[*number] = *scale;
    }

    return table;
  }

public:
  std::unordered_map<i32, i8> const fScaleLookupTable;
};

} // namespace je2be::tobe
