#pragma once

namespace je2be::box360 {

class Converter {
  Converter() = delete;

public:
  static bool Run(std::filesystem::path const &inputSaveBin, std::filesystem::path const &outputDirectory, unsigned int concurrency, Options const &options, Progress *progress = nullptr) {
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

    optional<chrono::system_clock::time_point> lastPlayed;
    {
      vector<uint8_t> buffer;
      auto savegameInfo = Savegame::ExtractSavagameFromSaveBin(inputSaveBin, buffer);
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

      if (!Savegame::DecompressSavegame(buffer)) {
        return false;
      }
      if (!Savegame::ExtractFilesFromDecompressedSavegame(buffer, *temp)) {
        return false;
      }
    }
    Context ctx(TileEntity::Convert, Entity::MigrateName);
    if (!CopyMapFiles(*temp, outputDirectory)) {
      return false;
    }
    auto copyPlayersResult = CopyPlayers(*temp, outputDirectory, ctx, options);
    if (!copyPlayersResult) {
      return false;
    }
    if (!CopyLevelDat(*temp, outputDirectory, lastPlayed, copyPlayersResult->fLocalPlayer, ctx)) {
      return false;
    }
    if (!SetupResourcePack(outputDirectory)) {
      return false;
    }
    int progressChunksOffset = 0;
    for (auto dimension : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
      defer {
        progressChunksOffset += 4096;
      };
      if (!options.fDimensionFilter.empty()) {
        if (options.fDimensionFilter.find(dimension) == options.fDimensionFilter.end()) {
          continue;
        }
      }
      if (!World::Convert(*temp, outputDirectory, dimension, concurrency, ctx, options, progress, progressChunksOffset)) {
        return false;
      }
    }
    return true;
  }

private:
  struct CopyPlayerResult {
    CompoundTagPtr fLocalPlayer;
  };

  static std::optional<CopyPlayerResult> CopyPlayers(std::filesystem::path const &inputDirectory, std::filesystem::path const &outputDirectory, Context &ctx, Options const &options) {
    using namespace std;
    namespace fs = std::filesystem;

    auto playersFrom = inputDirectory / "players";
    auto playersTo = outputDirectory / "playerdata";

    if (!Fs::CreateDirectories(playersTo)) {
      return nullopt;
    }

    CopyPlayerResult r;
    vector<CompoundTagPtr> players;

    error_code ec;
    for (auto it : fs::directory_iterator(playersFrom, ec)) {
      if (!it.is_regular_file()) {
        continue;
      }
      auto player = CopyPlayer(it.path(), playersTo, ctx);
      if (!player) {
        continue;
      }
      if (r.fLocalPlayer) {
        players.push_back(player);
      } else {
        r.fLocalPlayer = player;
      }
    }

    for (auto &player : players) {
      auto uuidB = player->string("UUID");
      if (!uuidB) {
        return nullopt;
      }
      auto uuidJ = Entity::MigrateUuid(*uuidB, ctx);
      if (!uuidJ) {
        return nullopt;
      }
      player->set("UUID", uuidJ->toIntArrayTag());
    }
    if (r.fLocalPlayer && options.fLocalPlayer) {
      auto uuidB = r.fLocalPlayer->string("UUID");
      if (!uuidB) {
        return nullopt;
      }
      r.fLocalPlayer->set("UUID", options.fLocalPlayer->toIntArrayTag());
      players.push_back(r.fLocalPlayer);
      ctx.fPlayers.insert(make_pair(*uuidB, *options.fLocalPlayer));
    }

    for (auto const &player : players) {
      auto uuidJ = props::GetUuidWithFormatIntArray(*player, "UUID");
      if (!uuidJ) {
        return nullopt;
      }
      auto filename = uuidJ->toString() + ".dat";
      auto filePath = playersTo / filename;
      auto outputStream = make_shared<mcfile::stream::FileOutputStream>(filePath);
      mcfile::stream::OutputStreamWriter writer(outputStream, endian::big);
      if (!player->writeAsRoot(writer)) {
        return nullopt;
      }
    }

    return r;
  }

  static CompoundTagPtr CopyPlayer(std::filesystem::path const &inputFile, std::filesystem::path const &outputPlayerdata, Context const &ctx) {
    using namespace std;
    auto stream = make_shared<mcfile::stream::FileInputStream>(inputFile);
    auto in = CompoundTag::Read(stream, endian::big);
    stream.reset();
    if (!in) {
      // Not an nbt format. just skip this
      return nullptr;
    }

    auto out = in->copy();
    out->erase("GamePrivileges");
    out->erase("Sleeping");

    Entity::CopyItems(*in, *out, ctx, "EnderItems");
    Entity::CopyItems(*in, *out, ctx, "Inventory");

    if (auto dimensionB = in->int32("Dimension"); dimensionB) {
      if (auto dimension = DimensionFromBedrockDimension(*dimensionB); dimension) {
        out->set("Dimension", String(JavaStringFromDimension(*dimension)));
      }
    }

    out->set("DataVersion", Int(Chunk::kTargetDataVersion));

    if (auto rootVehicleB = in->compoundTag("RootVehicle"); rootVehicleB) {
      auto rootVehicleJ = Compound();
      if (auto entityB = rootVehicleB->compoundTag("Entity"); entityB) {
        if (auto entityJ = Entity::Convert(*entityB, ctx); entityJ && entityJ->fEntity) {
          rootVehicleJ->set("Entity", entityJ->fEntity);
        }
      }
      if (auto attachB = rootVehicleB->string("Attach"); attachB) {
        if (auto attachJ = Entity::MigrateUuid(*attachB, ctx); attachJ) {
          rootVehicleJ->set("Attach", attachJ->toIntArrayTag());
        }
      }
      out->set("RootVehicle", rootVehicleJ);
    }

    return out;
  }

  static bool SetupResourcePack(std::filesystem::path const &outputDirectory) {
    using namespace std;

    auto resourcesZipPath = outputDirectory / "resources.zip";
    Fs::Delete(resourcesZipPath);

    ZipFile resources(resourcesZipPath);
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
      if (!resources.store(forcefield, "assets/minecraft/textures/misc/forcefield.png")) {
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
      if (!resources.store(mcmeta, "pack.mcmeta")) {
        return false;
      }
    }

    return resources.close();
  }

  static bool CopyLevelDat(std::filesystem::path const &inputDirectory, std::filesystem::path const &outputDirectory, std::optional<std::chrono::system_clock::time_point> lastPlayed, CompoundTagPtr const &localPlayer, Context const &ctx) {
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

    if (auto dimensionData = in->compoundTag("DimensionData"); dimensionData) {
      if (auto theEnd = dimensionData->compoundTag("The End"); theEnd) {
        if (auto dragonFightB = theEnd->compoundTag("DragonFight"); dragonFightB) {
          auto dragonFightJ = dragonFightB->copy();
          if (auto dragonUuidB = dragonFightB->string("DragonUUID"); dragonUuidB) {
            if (auto dragonUuidJ = Entity::MigrateUuid(*dragonUuidB, ctx); dragonUuidJ) {
              dragonFightJ->set("Dragon", dragonUuidJ->toIntArrayTag());
              dragonFightJ->erase("DragonUUID");
            }
          }
          out->set("DragonFight", dragonFightJ);
        }
      }
    }

    if (localPlayer) {
      out->set("Player", localPlayer);
    }

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
