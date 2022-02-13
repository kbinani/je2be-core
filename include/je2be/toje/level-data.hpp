#pragma once

namespace je2be::toje {

class LevelData {
public:
  struct GameRules {
    static std::shared_ptr<CompoundTag> Import(CompoundTag const &b) {
      auto ret = std::make_shared<CompoundTag>();
      CompoundTag &j = *ret;
#define B(__nameJ, __nameB, __default) j[#__nameJ] = props::String(b.boolean(#__nameB, __default) ? "true" : "false");
#define I(__nameJ, __nameB, __default) j[#__nameJ] = props::String(std::to_string(b.int32(#__nameB, __default)));
      // announceAdvancements
      B(commandBlockOutput, commandblockoutput, true);
      // disableElytraMovementCheck
      // disableRaids
      B(doDaylightCycle, dodaylightcycle, true);
      B(doEntityDrops, doentitydrops, true);
      B(doFireTick, dofiretick, true);
      B(doImmediateRespawn, doimmediaterespawn, false);
      B(doInsomnia, doinsomnia, true);
      // doLimitedCrafting
      B(doMobLoot, domobloot, true);
      B(doMobSpawning, domobspawning, true);
      // doPatrolSpawning
      B(doTileDrops, dotiledrops, true);
      // doTraderSpawning
      B(doWeatherCycle, doweathercycle, true);
      B(drowningDamage, drowningdamage, true);
      B(fallDamage, falldamage, true);
      B(fireDamage, firedamage, true);
      // forgiveDeadPlayers
      B(freezeDamage, freezedamage, true);
      B(keepInventory, keepinventory, false);
      // logAdminCommands
      I(maxCommandChainLength, maxcommandchainlength, 65536);
      // maxEntityCramming
      B(mobGriefing, mobgriefing, true);
      B(naturalRegeneration, naturalregeneration, true);
      I(randomTickSpeed, randomtickspeed, 3);
      // reducedDebugInfo
      B(sendCommandFeedback, sendcommandfeedback, true);
      B(showDeathMessages, showdeathmessages, true);
      I(spawnRadius, spawnradius, 10);
      // spectatorsGenerateChunks
      // universalAnger
#undef B
#undef I
      return ret;
    }
  };

public:
  struct LevelDataInit {
    int DataVersion = mcfile::je::Chunk::kDataVersion;
    int version = 19133;
  };

  static std::shared_ptr<CompoundTag> Import(std::filesystem::path levelDatFile, LevelDataInit init = {}) {
    using namespace std;
    using namespace mcfile::stream;
    using namespace props;
    auto fis = make_shared<FileInputStream>(levelDatFile);
    if (!fis) {
      return nullptr;
    }
    InputStreamReader isr(fis, {.fLittleEndian = true});
    uint32_t v0;
    if (!isr.read(&v0)) {
      return nullptr;
    }
    uint32_t v1;
    if (!isr.read(&v1)) {
      return nullptr;
    }
    auto tag = make_shared<CompoundTag>();
    if (!tag->read(isr)) {
      return nullptr;
    }
    auto b = tag->compoundTag("");
    if (!b) {
      return nullptr;
    }

    auto data = std::make_shared<CompoundTag>();
    CompoundTag &j = *data;

    CopyStringValues(*b, j, {{"LevelName"}});
    CopyIntValues(*b, j, {{"SpawnX"}, {"SpawnY"}, {"SpawnZ"}, {"GameType"}, {"rainTime"}, {"lightningTime", "thunderTime"}});
    CopyBoolValues(*b, j, {{"commandsEnabled", "allowCommands", false}});
    CopyLongValues(*b, j, {{"Time", "DayTime"}, {"currentTick", "Time"}});

    j["Difficulty"] = Byte(b->int32("Difficulty", 2));
    j["GameRules"] = GameRules::Import(*b);
    j["LastPlayed"] = Long(b->int64("LastPlayed", 0) * 1000);

    j["DataVersion"] = Int(init.DataVersion);
    j["version"] = Int(init.version);

    {
      auto dataPacks = make_shared<CompoundTag>();
      dataPacks->set("Disabled", make_shared<ListTag>(Tag::Type::String));
      auto enabled = make_shared<ListTag>(Tag::Type::String);
      enabled->push_back(String("vanilla"));
      dataPacks->set("Enabled", enabled);
      j["DataPacks"] = dataPacks;
    }
    {
      auto brands = make_shared<ListTag>(Tag::Type::String);
      brands->push_back(String("vanilla"));
      j["ServerBrands"] = brands;
    }
    {
      auto version = make_shared<CompoundTag>();
      version->set("Id", Int(mcfile::je::Chunk::kDataVersion));
      version->set("Name", String(kVersion));
      version->set("Series", String("main"));
      version->set("Snapshot", Byte(0));
      j["Version"] = version;
    }
    {
      auto worldGenSettings = make_shared<CompoundTag>();
      CopyBoolValues(*b, *worldGenSettings, {{"bonusChestEnabled", "bonus_chest"}});
      worldGenSettings->set("generate_features", Bool(true));
      if (auto seed = b->int64("RandomSeed"); seed) {
        worldGenSettings->set("seed", Long(*seed));
        auto dimensions = make_shared<CompoundTag>();
        {
          auto overworld = make_shared<CompoundTag>();
          auto generator = make_shared<CompoundTag>();
          auto biomeSource = make_shared<CompoundTag>();
          biomeSource->set("preset", String("minecraft:overworld"));
          biomeSource->set("type", String("minecraft:multi_noise"));
          generator->set("biome_source", biomeSource);
          generator->set("seed", Long(*seed));
          generator->set("settings", String("minecraft:overworld"));
          generator->set("type", String("minecraft:noise"));
          overworld->set("generator", generator);
          overworld->set("type", String("minecraft:overworld"));
          dimensions->set("minecraft:overworld", overworld);
        }
        {
          auto end = make_shared<CompoundTag>();
          auto generator = make_shared<CompoundTag>();
          auto biomeSource = make_shared<CompoundTag>();
          biomeSource->set("seed", Long(*seed));
          biomeSource->set("type", String("minecraft:the_end"));
          generator->set("biome_source", biomeSource);
          generator->set("seed", Long(*seed));
          generator->set("settings", String("minecraft:end"));
          generator->set("type", String("minecraft:noise"));
          end->set("generator", generator);
          end->set("type", String("minecraft:the_end"));
          dimensions->set("minecraft:the_end", end);
        }
        {
          auto nether = make_shared<CompoundTag>();
          auto generator = make_shared<CompoundTag>();
          auto biomeSource = make_shared<CompoundTag>();
          biomeSource->set("preset", String("minecraft:nether"));
          biomeSource->set("type", String("minecraft:multi_noise"));
          generator->set("biome_source", biomeSource);
          generator->set("seed", Long(*seed));
          generator->set("settings", String("minecraft:nether"));
          generator->set("type", String("minecraft:noise"));
          nether->set("generator", generator);
          nether->set("type", String("minecraft:the_nether"));
          dimensions->set("minecraft:the_nether", nether);
        }
        worldGenSettings->set("dimensions", dimensions);
      }
      j["WorldGenSettings"] = worldGenSettings;
    }

    j["raining"] = Bool(b->float32("rainLevel", 0) >= 1);
    j["thundering"] = Bool(b->float32("lightningLevel", 0) >= 1);

    auto root = std::make_shared<CompoundTag>();
    root->set("Data", data);
    return root;
  }

  [[nodiscard]] static bool Write(CompoundTag const &tag, std::filesystem::path file) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;

    auto bs = make_shared<ByteStream>();
    OutputStreamWriter osw(bs);
    if (!tag.writeAsRoot(osw)) {
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
