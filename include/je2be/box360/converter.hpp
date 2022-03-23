#pragma once

namespace je2be::box360 {

class Converter {
  Converter() = delete;

public:
  static bool Run(std::filesystem::path const &inputSaveBin, std::filesystem::path const &outputDirectory, unsigned int concurrency, Options const &options) {
    using namespace std;
    namespace fs = std::filesystem;

    auto tempRoot = options.fTempDirectory ? *options.fTempDirectory : fs::temp_directory_path();
    auto temp = mcfile::File::CreateTempDir(tempRoot);
    if (!temp) {
      return false;
    }
    defer {
      Fs::DeleteAll(*temp);
    };
    fs::path savegame = *temp / "savegame.dat";

    optional<chrono::system_clock::time_point> lastPlayed;
    {
      auto savegameInfo = Savegame::ExtractSavagameFromSaveBin(inputSaveBin, savegame);
      if (!savegameInfo) {
        return false;
      }
      if (auto thumbnail = savegameInfo->fThumbnailImage; thumbnail) {
        auto iconPath = outputDirectory / "icon.png";
        auto icon = make_shared<mcfile::stream::FileOutputStream>(iconPath);
        if (!icon->write(thumbnail->data(), thumbnail->size())) {
          return false;
        }
      }
      lastPlayed = savegameInfo->fCreatedTime;
    }
    vector<uint8_t> buffer;
    if (!Savegame::DecompressSavegame(savegame, buffer)) {
      return false;
    }
    if (!Savegame::ExtractFilesFromDecompressedSavegame(buffer, *temp)) {
      return false;
    }
    if (!CopyMapFiles(*temp, outputDirectory)) {
      return false;
    }
    if (!CopyLevelDat(*temp, outputDirectory, lastPlayed)) {
      return false;
    }
    if (!SetupResourcePack(outputDirectory)) {
      return false;
    }
    vector<uint8_t>().swap(buffer);
    for (auto dimension : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
      if (!options.fDimensionFilter.empty()) {
        if (options.fDimensionFilter.find(dimension) == options.fDimensionFilter.end()) {
          continue;
        }
      }
      if (!World::Convert(*temp, outputDirectory, dimension, concurrency, options)) {
        return false;
      }
    }
    return true;
  }

private:
  static bool SetupResourcePack(std::filesystem::path const &outputDirectory) {
    using namespace std;

    auto resourcesZipPath = outputDirectory / "resources.zip";
    Fs::Delete(resourcesZipPath);

    void *handle = nullptr;
    if (!mz_zip_create(&handle)) {
      return false;
    }
    defer {
      mz_zip_delete(&handle);
    };

    void *stream = nullptr;
    if (!mz_stream_os_create(&stream)) {
      return false;
    }
    defer {
      mz_stream_os_delete(&stream);
    };

    string resourcesZip = resourcesZipPath.string();
    if (MZ_OK != mz_stream_os_open(stream, resourcesZip.c_str(), MZ_OPEN_MODE_CREATE)) {
      return false;
    }
    if (MZ_OK != mz_zip_open(handle, stream, MZ_OPEN_MODE_WRITE)) {
      return false;
    }

    {
      unsigned char transparent16x16_png[] = {
          0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
          0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
          0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff, 0x61, 0x00, 0x00, 0x00,
          0x12, 0x49, 0x44, 0x41, 0x54, 0x38, 0xcb, 0x63, 0x60, 0x18, 0x05, 0xa3,
          0x60, 0x14, 0x8c, 0x02, 0x08, 0x00, 0x00, 0x04, 0x10, 0x00, 0x01, 0x85,
          0x3f, 0xaa, 0x72, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae,
          0x42, 0x60, 0x82};
      unsigned int transparent16x16_png_len = 75;
      vector<uint8_t> forcefield;
      copy_n(transparent16x16_png, transparent16x16_png_len, back_inserter(forcefield));
      if (!AddBufferToZip(handle, forcefield, "assets/minecraft/textures/misc/forcefield.png")) {
        return false;
      }
    }
    {
      nlohmann::json obj;
      nlohmann::json pack;
      pack["pack_format"] = 7;
      pack["description"] = "invisible worldborder by je2be";
      obj["pack"] = pack;
      auto str = nlohmann::to_string(obj);
      vector<uint8_t> mcmeta;
      copy(str.begin(), str.end(), back_inserter(mcmeta));
      if (!AddBufferToZip(handle, mcmeta, "pack.mcmeta")) {
        return false;
      }
    }

    if (MZ_OK != mz_zip_close(handle)) {
      return false;
    }
    if (MZ_OK != mz_stream_close(stream)) {
      return false;
    }

    return true;
  }

  static bool AddBufferToZip(void *zip, std::vector<uint8_t> const &buffer, std::string const &filename) {
    mz_zip_file s = {0};
    s.version_madeby = MZ_VERSION_MADEBY;
    s.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
    s.filename = filename.c_str();
    int16_t compressLevel = 9;
    uint8_t raw = 0;
    if (MZ_OK != mz_zip_entry_write_open(zip, &s, compressLevel, raw, nullptr)) {
      return false;
    }
    int32_t totalWritten = 0;
    int32_t err = MZ_OK;
    int32_t remaining = buffer.size();
    do {
      int32_t code = mz_zip_entry_write(zip, buffer.data() + totalWritten, remaining);
      if (code < 0) {
        err = code;
      } else {
        totalWritten += code;
        remaining -= code;
      }
    } while (err == MZ_OK && remaining > 0);

    return err == MZ_OK && mz_zip_entry_close(zip) == MZ_OK;
  }

  static bool CopyLevelDat(std::filesystem::path const &inputDirectory, std::filesystem::path const &outputDirectory, std::optional<std::chrono::system_clock::time_point> lastPlayed) {
    using namespace std;
    namespace fs = std::filesystem;
    auto datFrom = inputDirectory / "level.dat";
    auto datTo = outputDirectory / "level.dat";
    if (!Fs::Exists(datFrom)) {
      return false;
    }

    auto inStream = make_shared<mcfile::stream::GzFileInputStream>(datFrom);
    auto inRoot = CompoundTag::Read(inStream, endian::big);
    inStream.reset();
    if (!inRoot) {
      return false;
    }
    auto in = inRoot->compoundTag("Data");
    if (!in) {
      return false;
    }

    JavaLevelDat::Options o;
    o.fBonusChestEnabled = in->boolean("spawnBonusChest");
    o.fDataVersion = Chunk::kTargetDataVersion;
    o.fRandomSeed = in->int64("RandomSeed");
    o.fVersionString = Chunk::TargetVersionString();
    o.fFlatWorldSettings = FlatWorldSettingsForOverworldOuterRegion();
    auto out = JavaLevelDat::TemplateData(o);

    CopyBoolValues(*in, *out, {{"allowCommands"}, {"DifficultyLocked"}, {"hardcore"}, {"initialized"}, {"raining"}, {"thundering"}});
    CopyByteValues(*in, *out, {{"Difficulty"}});
    CopyIntValues(*in, *out, {{"rainTime"}, {"GameType"}, {"SpawnX"}, {"SpawnY"}, {"SpawnZ"}, {"thunderTime"}, {"clearWeatherTime"}});
    CopyLongValues(*in, *out, {{"DayTime"}, {"Time"}});
    CopyStringValues(*in, *out, {{"LevelName"}});

    if (lastPlayed) {
      // NOTE: LastPlayed in the level.dat from xbox360 edition is wired. Use timestamp of savegame.dat for it
      int64_t ms = chrono::duration_cast<chrono::milliseconds>(lastPlayed->time_since_epoch()).count();
      out->set("LastPlayed", Long(ms));
    }

    out->set("BorderCenterX", Double(0));
    out->set("BorderCenterZ", Double(0));
    out->set("BorderSize", Double(992));
    out->set("BorderWarningBlocks", Double(0));

    auto outRoot = Compound();
    outRoot->set("Data", out);
    auto outStream = make_shared<mcfile::stream::GzFileOutputStream>(datTo);
    mcfile::stream::OutputStreamWriter writer(outStream, endian::big);
    return outRoot->writeAsRoot(writer);
  }

  static CompoundTagPtr FlatWorldSettingsForOverworldOuterRegion() {
    auto flatSettings = Compound();
    flatSettings->set("biome", String("minecraft:ocean"));
    flatSettings->set("features", Bool(false));
    flatSettings->set("lakes", Bool(false));
    auto structures = Compound();
    structures->set("structures", Compound());
    flatSettings->set("structures", structures);

    auto layers = List<Tag::Type::Compound>();

    auto air = Compound();
    air->set("block", String("minecraft:air"));
    air->set("height", Int(64));
    layers->push_back(air);

    auto bedrock = Compound();
    bedrock->set("block", String("minecraft:bedrock"));
    bedrock->set("height", Int(1));
    layers->push_back(bedrock);

    auto stone = Compound();
    stone->set("block", String("minecraft:stone"));
    stone->set("height", Int(53));
    layers->push_back(stone);

    auto water = Compound();
    water->set("block", String("minecraft:water"));
    water->set("height", Int(9));
    layers->push_back(water);

    flatSettings->set("layers", layers);

    return flatSettings;
  }

  static bool CopyMapFiles(std::filesystem::path const &inputDirectory, std::filesystem::path const &outputDirectory) {
    namespace fs = std::filesystem;

    auto dataFrom = inputDirectory / "data";
    auto dataTo = outputDirectory / "data";
    if (!Fs::Exists(dataFrom)) {
      return true;
    }
    if (!Fs::CreateDirectories(dataTo)) {
      return false;
    }
    std::error_code ec;
    for (auto it : fs::directory_iterator(dataFrom, ec)) {
      if (!it.is_regular_file()) {
        continue;
      }
      auto fileName = it.path().filename();
      auto fileNameString = fileName.string();
      if (!fileNameString.starts_with("map_") || !fileNameString.ends_with(".dat")) {
        continue;
      }
      auto numberString = strings::RTrim(strings::LTrim(fileNameString, "map_"), ".dat");
      auto number = strings::Toi(numberString);
      if (!number) {
        continue;
      }
      std::error_code ec1;
      fs::copy_options o = fs::copy_options::overwrite_existing;
      fs::copy_file(dataFrom / fileNameString, dataTo / fileNameString, o, ec1);
      if (ec1) {
        return false;
      }
    }
    return true;
  }
};

} // namespace je2be::box360
