#pragma once

namespace je2be::toje {

class LevelData {
public:
  struct GameRules {
    static std::shared_ptr<CompoundTag> Import(CompoundTag const &b, leveldb::DB &db) {
      auto ret = nbt::Compound();
      CompoundTag &j = *ret;
#define B(__nameJ, __nameB, __default) j[#__nameJ] = nbt::String(b.boolean(#__nameB, __default) ? "true" : "false");
#define I(__nameJ, __nameB, __default) j[#__nameJ] = nbt::String(std::to_string(b.int32(#__nameB, __default)));
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

      std::string mobEventsData;
      if (auto st = db.Get(leveldb::ReadOptions{}, mcfile::be::DbKey::MobEvents(), &mobEventsData); st.ok()) {
        if (auto mobEvents = CompoundTag::Read(mobEventsData, {.fLittleEndian = true}); mobEvents) {
          auto doPatrolSpawning = mobEvents->boolean("minecraft:pillager_patrols_event", true);
          auto doTraderSpawning = mobEvents->boolean("minecraft:wandering_trader_event", true);

          ret->set("doPatrolSpawning", nbt::String(doPatrolSpawning ? "true" : "false"));
          ret->set("doTraderSpawning", nbt::String(doTraderSpawning ? "true" : "false"));
        }
      }

      return ret;
    }
  };

public:
  static std::shared_ptr<CompoundTag> Read(std::filesystem::path levelDatFile) {
    using namespace std;
    using namespace mcfile::stream;
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
    auto tag = nbt::Compound();
    if (!tag->read(isr)) {
      return nullptr;
    }
    return tag->compoundTag("");
  }

  static std::shared_ptr<CompoundTag> Import(CompoundTag const &b, leveldb::DB &db, Options opt, Context &ctx) {
    using namespace std;
    using namespace je2be::nbt;

    auto data = nbt::Compound();
    CompoundTag &j = *data;

    CopyStringValues(b, j, {{"LevelName"}});
    CopyIntValues(b, j, {{"SpawnX"}, {"SpawnY"}, {"SpawnZ"}, {"GameType"}, {"rainTime"}, {"lightningTime", "thunderTime"}});
    CopyBoolValues(b, j, {{"commandsEnabled", "allowCommands", false}});
    CopyLongValues(b, j, {{"Time", "DayTime"}, {"currentTick", "Time"}});

    j["Difficulty"] = Byte(b.int32("Difficulty", 2));
    j["GameRules"] = GameRules::Import(b, db);
    j["LastPlayed"] = Long(b.int64("LastPlayed", 0) * 1000);

    j["DataVersion"] = Int(mcfile::je::Chunk::kDataVersion);
    j["version"] = Int(kLevelVersion);

    {
      auto dataPacks = nbt::Compound();
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
      auto version = nbt::Compound();
      version->set("Id", Int(mcfile::je::Chunk::kDataVersion));
      version->set("Name", String(kVersionString));
      version->set("Series", String("main"));
      version->set("Snapshot", Byte(0));
      j["Version"] = version;
    }
    {
      auto worldGenSettings = nbt::Compound();
      CopyBoolValues(b, *worldGenSettings, {{"bonusChestEnabled", "bonus_chest"}});
      worldGenSettings->set("generate_features", Bool(true));
      if (auto seed = b.int64("RandomSeed"); seed) {
        worldGenSettings->set("seed", Long(*seed));
        auto dimensions = nbt::Compound();
        {
          auto overworld = nbt::Compound();
          auto generator = nbt::Compound();
          auto biomeSource = nbt::Compound();
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
          auto end = nbt::Compound();
          auto generator = nbt::Compound();
          auto biomeSource = nbt::Compound();
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
          auto nether = nbt::Compound();
          auto generator = nbt::Compound();
          auto biomeSource = nbt::Compound();
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

    j["raining"] = Bool(b.float32("rainLevel", 0) >= 1);
    j["thundering"] = Bool(b.float32("lightningLevel", 0) >= 1);

    if (auto dragonFight = DragonFight(db); dragonFight) {
      j["DragonFight"] = dragonFight;
    }

    if (auto playerData = Player(db, ctx, opt.fLocalPlayer); playerData) {
      ctx.setLocalPlayerIds(playerData->fEntityIdBedrock, playerData->fEntityIdJava);
      j["Player"] = playerData->fEntity;
    }

    auto root = nbt::Compound();
    root->set("Data", data);
    return root;
  }

  static std::optional<Entity::LocalPlayerData> Player(leveldb::DB &db, Context &ctx, std::optional<Uuid> uuid) {
    std::string str;
    if (auto st = db.Get(leveldb::ReadOptions{}, mcfile::be::DbKey::LocalPlayer(), &str); !st.ok()) {
      return std::nullopt;
    }

    auto tag = CompoundTag::Read(str, {.fLittleEndian = true});
    if (!tag) {
      return std::nullopt;
    }

    return Entity::LocalPlayer(*tag, ctx, uuid);
  }

  static std::shared_ptr<CompoundTag> DragonFight(leveldb::DB &db) {
    std::string str;
    if (auto st = db.Get(leveldb::ReadOptions{}, mcfile::be::DbKey::TheEnd(), &str); !st.ok()) {
      return nullptr;
    }
    auto tag = CompoundTag::Read(str, {.fLittleEndian = true});
    if (!tag) {
      return nullptr;
    }
    auto data = tag->compoundTag("data");
    if (!data) {
      return nullptr;
    }
    auto fightB = data->compoundTag("DragonFight");
    if (!fightB) {
      return nullptr;
    }
    auto fightJ = nbt::Compound();
    if (auto exitPortalLocation = props::GetPos3iFromListTag(*fightB, "ExitPortalLocation"); exitPortalLocation) {
      auto exitPortalLocationJ = nbt::Compound();
      exitPortalLocationJ->set("X", nbt::Int(exitPortalLocation->fX));
      exitPortalLocationJ->set("Y", nbt::Int(exitPortalLocation->fY));
      exitPortalLocationJ->set("Z", nbt::Int(exitPortalLocation->fZ));
      fightJ->set("ExitPortalLocation", exitPortalLocationJ);
    }
    CopyBoolValues(*fightB, *fightJ, {{"DragonKilled"}, {"PreviouslyKilled"}});
    if (auto dragonUidB = fightB->int64("DragonUUID"); dragonUidB) {
      auto dragonUidJ = Uuid::GenWithI64Seed(*dragonUidB);
      fightJ->set("Dragon", dragonUidJ.toIntArrayTag());
    }
    if (auto gateways = fightB->listTag("Gateways"); gateways) {
      fightJ->set("Gateways", gateways);
    }
    fightJ->set("NeedsStateScanning", nbt::Bool(false));
    return fightJ;
  }

  [[nodiscard]] static bool Write(CompoundTag const &tag, std::filesystem::path file) {
    auto gzs = std::make_shared<mcfile::stream::GzFileOutputStream>(file);
    mcfile::stream::OutputStreamWriter osw(gzs);
    return tag.writeAsRoot(osw);
  }
};

} // namespace je2be::toje
