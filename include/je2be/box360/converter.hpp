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

    if (!Savegame::ExtractSavagameFromSaveBin(inputSaveBin, savegame)) {
      return false;
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
    if (!CopyLevelDat(*temp, outputDirectory)) {
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
  static bool CopyLevelDat(std::filesystem::path const &inputDirectory, std::filesystem::path const &outputDirectory) {
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

    out->set("BorderCenterX", Double(0));
    out->set("BorderCenterZ", Double(0));
    out->set("BorderSize", Double(862));
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
