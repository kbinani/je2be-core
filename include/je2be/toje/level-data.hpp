#pragma once

namespace je2be::toje {

class LevelData {
private:
  int32_t fDataVersion = mcfile::je::Chunk::kDataVersion;
  std::string fLevelName;
  int32_t fVersion = 19133;
  int32_t fSpawnX = 0;
  int32_t fSpawnY = 64;
  int32_t fSpawnZ = 0;
  bool fAllowCommands = false;
  int64_t fDayTime = 0;
  int8_t fDifficulty = 2;
  std::shared_ptr<CompoundTag> fGameRules;

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
  static std::optional<LevelData> Import(std::filesystem::path levelDatFile) {
    using namespace std;
    using namespace mcfile::stream;
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
    d.fDayTime = t->int64("Time", d.fDayTime);
    d.fDifficulty = t->int32("Difficulty", d.fDifficulty);
    d.fGameRules = GameRules::Import(*t);
    return d;
  }

  std::shared_ptr<CompoundTag> toCompoundTag() const {
    using namespace std;
    using namespace mcfile;
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
    t["DayTime"] = Long(fDayTime);
    t["Difficulty"] = Byte(fDifficulty);
    {
      auto dataPacks = make_shared<CompoundTag>();
      dataPacks->set("Disabled", make_shared<ListTag>(Tag::Type::String));
      auto enabled = make_shared<ListTag>(Tag::Type::String);
      enabled->push_back(String("vanilla"));
      dataPacks->set("Enabled", enabled);
      t["DataPacks"] = dataPacks;
    }
    t["GameRules"] = fGameRules->copy();
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
