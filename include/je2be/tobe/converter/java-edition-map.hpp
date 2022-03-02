#pragma once

namespace je2be::tobe {

class JavaEditionMap {
public:
  explicit JavaEditionMap(std::unordered_map<int32_t, int8_t> const &lookupTable) : fScaleLookupTable(lookupTable) {}
  JavaEditionMap(std::filesystem::path const &input, Options const &opt) : fScaleLookupTable(CreateScaleLookupTable(input, opt)) {}

  std::optional<int8_t> scale(int32_t mapId) const {
    auto found = fScaleLookupTable.find(mapId);
    if (found == fScaleLookupTable.end()) {
      return std::nullopt;
    }
    return found->second;
  }

  [[nodiscard]] bool each(std::function<bool(int32_t /* javaMapId */)> cb) {
    for (auto it : fScaleLookupTable) {
      if (!cb(it.first)) {
        return false;
      }
    }
    return true;
  }

  static CompoundTagPtr Read(std::filesystem::path const &input, Options const &opt, int32_t mapId) {
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

    vector<uint8_t> buffer;
    {
      vector<char> buf(512);
      auto p = jeFilePath;
      gzFile f = mcfile::File::GzOpen(p, mcfile::File::Mode::Read);
      if (!f) {
        return nullptr;
      }
      while (true) {
        int read = gzread(f, buf.data(), buf.size());
        if (read <= 0) {
          break;
        }
        copy_n(buf.begin(), read, back_inserter(buffer));
        if (read < buf.size()) {
          break;
        }
      }
      gzclose(f);
    }

    auto root = Compound();
    auto bs = make_shared<ByteStream>(buffer);
    InputStreamReader r(bs);
    if (!root->read(r)) {
      return nullptr;
    }

    return root;
  }

private:
  static std::unordered_map<int32_t, int8_t> CreateScaleLookupTable(std::filesystem::path const &input, Options const &opt) {
    namespace fs = std::filesystem;
    std::unordered_map<int32_t, int8_t> table;

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
  std::unordered_map<int32_t, int8_t> const fScaleLookupTable;
};

} // namespace je2be::tobe
