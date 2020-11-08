#pragma once

namespace j2b {

class JavaEditionMap {
public:
  explicit JavaEditionMap(std::filesystem::path const &input)
      : fScaleLookupTable(CreateScaleLookupTable(input)) {}

  std::optional<int8_t> scale(int32_t mapId) const {
    auto found = fScaleLookupTable.find(mapId);
    if (found == fScaleLookupTable.end()) {
      return std::nullopt;
    }
    return found->second;
  }

  void each(std::function<void(int32_t /* javaMapId */)> cb) {
    for (auto it : fScaleLookupTable) {
      cb(it.first);
    }
  }

  static std::shared_ptr<mcfile::nbt::CompoundTag>
  Read(std::filesystem::path const &input, int32_t mapId) {
    using namespace std;
    namespace fs = std::filesystem;
    using namespace mcfile::stream;

    auto jeFilePath = input / "data" / ("map_" + to_string(mapId) + ".dat");
    if (!fs::is_regular_file(jeFilePath))
      return nullptr;

    vector<uint8_t> buffer;
    {
      vector<char> buf(512);
      auto p = jeFilePath.string();
      gzFile f = gzopen(p.c_str(), "rb");
      if (!f)
        return false;
      while (true) {
        int read = gzread(f, buf.data(), buf.size());
        if (read <= 0)
          break;
        copy_n(buf.begin(), read, back_inserter(buffer));
        if (read < buf.size()) {
          break;
        }
      }
      gzclose(f);
    }

    auto root = make_shared<mcfile::nbt::CompoundTag>();
    auto bs = make_shared<ByteStream>(buffer);
    InputStreamReader r(bs);
    root->read(r);

    return root;
  }

private:
  static std::unordered_map<int32_t, int8_t>
  CreateScaleLookupTable(std::filesystem::path const &input) {
    namespace fs = std::filesystem;
    std::unordered_map<int32_t, int8_t> table;

    auto dataDir = input / "data";
    if (!fs::exists(dataDir)) {
      return table;
    }

    for (auto const &f : fs::directory_iterator(dataDir)) {
      if (!f.is_regular_file())
        continue;
      auto name = f.path().filename().string();
      if (!name.starts_with("map_") || !name.ends_with(".dat"))
        continue;
      auto numberStr = strings::RTrim(strings::LTrim(name, "map_"), ".dat");
      auto number = strings::Toi(numberStr);
      if (!number)
        continue;

      auto map = Read(input, *number);
      if (!map)
        continue;
      auto data = map->query("/data")->asCompound();
      if (!data)
        continue;

      auto scale = props::GetByte(*data, "scale");
      if (!scale)
        continue;

      table[*number] = *scale;
    }

    return table;
  }

private:
  std::unordered_map<int32_t, int8_t> const fScaleLookupTable;
};

} // namespace j2b
