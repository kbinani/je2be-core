#pragma once

namespace je2be::toje {

class LevelData {
private:
  int32_t fDataVersion = 2865;
  std::string fLevelName;
  int32_t fVersion = 19133;
  int32_t fSpawnX = 0;
  int32_t fSpawnY = 64;
  int32_t fSpawnZ = 0;
  bool fAllowCommands = false;

public:
  static std::optional<LevelData> Import(std::filesystem::path levelDatFile) {
    using namespace std;
    using namespace mcfile::stream;
    using namespace mcfile::nbt;
    auto fis = make_shared<FileInputStream>(levelDatFile);
    if (!fis) {
      return nullopt;
    }
    InputStreamReader isr(fis, {.fLittleEndian = true});
    uint32_t v0;
    if (!isr.read(&v0)) {
      return nullopt;
    }
    uint32_t v1;
    if (!isr.read(&v1)) {
      return nullopt;
    }
    auto tag = make_shared<CompoundTag>();
    if (!tag->read(isr)) {
      return nullopt;
    }
    auto t = tag->compoundTag("");
    if (!t) {
      return nullopt;
    }

    LevelData d;
    d.fLevelName = t->string("LevelName", d.fLevelName);
    d.fSpawnX = t->int32("SpawnX", d.fSpawnX);
    d.fSpawnY = t->int32("SpawnY", d.fSpawnY);
    d.fSpawnZ = t->int32("SpawnZ", d.fSpawnZ);
    d.fAllowCommands = t->boolean("commandsEnabled", d.fAllowCommands);
    return d;
  }

  std::shared_ptr<mcfile::nbt::CompoundTag> toCompoundTag() const {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::nbt;
    using namespace props;
    auto ret = make_shared<CompoundTag>();
    CompoundTag &t = *ret;
    t["DataVersion"] = Int(fDataVersion);
    t["LevelName"] = String(fLevelName);
    t["version"] = Int(fVersion);
    t["SpawnX"] = Int(fSpawnX);
    t["SpawnY"] = Int(fSpawnY);
    t["SpawnZ"] = Int(fSpawnZ);
    t["allowCommands"] = Bool(fAllowCommands);
    auto root = make_shared<CompoundTag>();
    root->set("Data", ret);
    return root;
  }

  [[nodiscard]] bool write(std::filesystem::path const &file) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;

    auto tag = toCompoundTag();
    if (!tag) {
      return false;
    }

    auto bs = make_shared<ByteStream>();
    OutputStreamWriter osw(bs);
    if (!tag->writeAsRoot(osw)) {
      return false;
    }
    vector<uint8_t> buffer;
    bs->drain(buffer);

    gzFile fp = mcfile::File::GzOpen(file, mcfile::File::Mode::Write);
    if (!fp) {
      return false;
    }
    if (gzwrite(fp, buffer.data(), buffer.size()) == 0) {
      return false;
    }
    if (gzclose(fp) != Z_OK) {
      return false;
    }

    return true;
  }
};

} // namespace je2be::toje
