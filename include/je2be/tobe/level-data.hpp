#pragma once

namespace je2be::tobe {

class LevelData {
  using CompoundTag = mcfile::nbt::CompoundTag;

public:
  class Abilities {
  public:
    bool fAttackMobs = true;
    bool fAttackPlayers = true;
    bool fBuild = true;
    bool fDoorsAndSwitches = true;
    bool fFlying = false;
    float fFlySpeed = 0.05f;
    bool fInstabuild = false;
    bool fInvulnerable = false;
    bool fLightning = false;
    bool fMayFly = false;
    bool fMine = true;
    bool fOp = false;
    bool fOpenContainers = true;
    int fPermissionsLevel = 0;
    int fPlayerPermissionsLevel = 1;
    bool fTeleport = false;
    float fWalkSpeed = 0.1f;

    std::shared_ptr<mcfile::nbt::CompoundTag> toCompoundTag() const {
      using namespace props;

      auto a = std::make_shared<mcfile::nbt::CompoundTag>();
      a->insert({
          {"attackmobs", Bool(fAttackMobs)},
          {"attackplayers", Bool(fAttackPlayers)},
          {"build", Bool(fBuild)},
          {"doorsandswitches", Bool(fDoorsAndSwitches)},
          {"flying", Bool(fFlying)},
          {"flySpeed", Float(fFlySpeed)},
          {"instabuild", Bool(fInstabuild)},
          {"invulnerable", Bool(fInvulnerable)},
          {"lightning", Bool(fLightning)},
          {"mayfly", Bool(fMayFly)},
          {"mine", Bool(fMine)},
          {"op", Bool(fOp)},
          {"opencontainers", Bool(fOpenContainers)},
          {"permissionsLevel", Int(fPermissionsLevel)},
          {"playerPermissionsLevel", Int(fPlayerPermissionsLevel)},
          {"teleport", Bool(fTeleport)},
          {"walkSpeed", Float(fWalkSpeed)},
      });
      return a;
    }
  };

  Abilities fAbilities;
  Version fMinimumCompatibleClientVersion = kMinimumCompatibleClientVersion;
  std::string fBaseGameVersion = "*";
  std::string fBiomeOverride = "";
  bool fBonusChestEnabled = false;
  bool fBonusChestSpawned = false;
  int8_t fCenterMapsToOrigin = -7;
  bool fCommandBlockOutput = true;
  bool fCommandblocksEnabled = true;
  bool fCommandsEnabled = false;
  bool fConfirmedPlatformLockedContent = false;
  int64_t fCurrentTick = 0;
  int32_t fDifficulty = 0;
  std::string fFlatWorldLayers = "null\x0a";
  bool fDoDaylightCycle = true;
  bool fDoEntityDrops = true;
  bool fDoFireTick = true;
  bool fDoImmediateRespawn = false;
  bool fDoInsomnia = true;
  bool fDoMobLoot = true;
  bool fDoMobSpawning = true;
  bool fDoTileDrops = true;
  bool fDoWeatherCycle = true;
  bool fDrowningDamage = true;
  bool fEducationFeaturesEnabled = false;
  int32_t fEduOffer = 0;
  bool fExperimentalGamePlay = false;
  bool fFallDamage = true;
  bool fFireDamage = true;
  bool fForceGameType = false;
  int32_t fFunctionCommandLimit = 10000;
  int32_t fGameType = 1;
  int32_t fGenerator = 1;
  bool fHasBeenLoadedInCreative = false;
  bool fHasLockedBehaviorPack = false;
  bool fHasLockedResourcePack = false;
  bool fImmutableWorld = false;
  std::string fInventoryVersion = kInventoryVersion;
  bool fLANBroadcast = true;
  bool fLANBroadcastIntent = true;
  int64_t fLastPlayed = 0;
  std::string fLevelName = "";
  int32_t fLimitedWorldOriginX = 1852;
  int32_t fLimitedWorldOriginY = 32767;
  int32_t fLimitedWorldOriginZ = 4;
  bool fMultiplayerGame = true;
  bool fMultiplayerGameIntent = true;
  bool fNaturalRegeneration = true;
  int32_t fNetherScale = 8;
  int32_t fNetworkVersion = kNetworkVersion;
  int32_t fPlatform = 2;
  int32_t fPlatformBroadcastIntent = 3;
  std::string fPrid = "";
  bool fPvp = true;
  float fRainLevel = 0;
  int32_t fRainTime = 0;
  int64_t fRandomSeed = 0;
  int32_t fRandomTickSpeed = 1;
  bool fRequiresCopiedPackRemovalCheck = false;
  bool fSendCommandFeedback = true;
  int32_t fServerChunkTickRange = 0;
  bool fShowCoordinates = false;
  bool fShowDeathMessages = true;
  bool fShowTags = true;
  bool fSpawnMobs = true;
  int32_t fSpawnRadius = 5;
  bool fSpawnV1Villagers = false;
  int32_t fSpawnX = 0;
  int32_t fSpawnY = 0;
  int32_t fSpawnZ = 0;
  bool fStartWithMapEnabled = false;
  int32_t fStorageVersion = kStorageVersion;
  bool fTexturePacksRequired = false;
  int64_t fTime = 0;
  bool fTntExplodes = true;
  bool fUseMsaGamertagsOnly = false;
  int64_t fWorldStartCount = 0;
  int32_t fXBLBroadcastIntent = 3;
  bool fIsFromLockedTemplate = false;
  bool fIsFromWorldTemplate = false;
  bool fIsSingleUseWorld = false;
  bool fIsWorldTemplateOptionLocked = false;
  bool fKeepInventory = false;
  bool fMobGriefing = true;
  Version fLastOpenedWithVersion = kSupportVersion;
  float fLightningLevel = 0;
  int32_t fLightningTime = 0;
  int32_t fLimitedWorldDepth = 16;
  int32_t fLimitedWorldWidth = 16;
  int32_t fMaxCommandChainLength = 65535;

  std::shared_ptr<mcfile::nbt::CompoundTag> toCompoundTag() const {
    using namespace props;
    using props::Byte;

    auto root = std::make_shared<mcfile::nbt::CompoundTag>();
    root->insert({
        {"abilities", fAbilities.toCompoundTag()},
        {"MinimumCompatibleClientVersion", fMinimumCompatibleClientVersion.toListTag()},
        {"baseGameVersion", String(fBaseGameVersion)},
        {"BiomeOverride", String(fBiomeOverride)},
        {"bonusChestEnabled", Bool(fBonusChestEnabled)},
        {"bonusChestSpawned", Bool(fBonusChestSpawned)},
        {"CenterMapsToOrigin", Byte(fCenterMapsToOrigin)},
        {"commandblockoutput", Bool(fCommandBlockOutput)},
        {"commandblocksenabled", Bool(fCommandblocksEnabled)},
        {"commandsEnabled", Bool(fCommandsEnabled)},
        {"ConfirmedPlatformLockedContent", Bool(fConfirmedPlatformLockedContent)},
        {"currentTick", Long(fCurrentTick)},
        {"Difficulty", Int(fDifficulty)},
        {"FlatWorldLayers", String(fFlatWorldLayers)},
        {"dodaylightcycle", Bool(fDoDaylightCycle)},
        {"doentitydrops", Bool(fDoEntityDrops)},
        {"dofiretick", Bool(fDoFireTick)},
        {"doimmediaterespawn", Bool(fDoImmediateRespawn)},
        {"doinsomnia", Bool(fDoInsomnia)},
        {"domobloot", Bool(fDoMobLoot)},
        {"domobspawning", Bool(fDoMobSpawning)},
        {"dotiledrops", Bool(fDoTileDrops)},
        {"doweathercycle", Bool(fDoWeatherCycle)},
        {"drowningdamage", Bool(fDrowningDamage)},
        {"educationFeaturesEnabled", Bool(fEducationFeaturesEnabled)},
        {"eduOffer", Int(fEduOffer)},
        {"experimentalgameplay", Bool(fExperimentalGamePlay)},
        {"falldamage", Bool(fFallDamage)},
        {"firedamage", Bool(fFireDamage)},
        {"ForceGameType", Bool(fForceGameType)},
        {"functioncommandlimit", Int(fFunctionCommandLimit)},
        {"GameType", Int(fGameType)},
        {"Generator", Int(fGenerator)},
        {"hasBeenLoadedInCreative", Bool(fHasBeenLoadedInCreative)},
        {"hasLockedBehaviorPack", Bool(fHasLockedBehaviorPack)},
        {"hasLockedResourcePack", Bool(fHasLockedResourcePack)},
        {"immutableWorld", Bool(fImmutableWorld)},
        {"InventoryVersion", String(fInventoryVersion)},
        {"LANBroadcast", Bool(fLANBroadcast)},
        {"LANBroadcastIntent", Bool(fLANBroadcastIntent)},
        {"LastPlayed", Long(fLastPlayed)},
        {"LevelName", String(fLevelName)},
        {"LimitedWorldOriginX", Int(fLimitedWorldOriginX)},
        {"LimitedWorldOriginY", Int(fLimitedWorldOriginY)},
        {"LimitedWorldOriginZ", Int(fLimitedWorldOriginZ)},
        {"MultiplayerGame", Bool(fMultiplayerGame)},
        {"MultiplayerGameIntent", Bool(fMultiplayerGameIntent)},
        {"naturalregeneration", Bool(fNaturalRegeneration)},
        {"NetherScale", Int(fNetherScale)},
        {"NetworkVersion", Int(fNetworkVersion)},
        {"Platform", Int(fPlatform)},
        {"PlatformBroadcastIntent", Int(fPlatformBroadcastIntent)},
        {"prid", String(fPrid)},
        {"pvp", Bool(fPvp)},
        {"rainLevel", Float(fRainLevel)},
        {"rainTime", Int(fRainTime)},
        {"RandomSeed", Long(fRandomSeed)},
        {"randomtickspeed", Int(fRandomTickSpeed)},
        {"requiresCopiedPackRemovalCheck", Bool(fRequiresCopiedPackRemovalCheck)},
        {"sendcommandfeedback", Bool(fSendCommandFeedback)},
        {"serverChunkTickRange", Int(fServerChunkTickRange)},
        {"showcoordinates", Bool(fShowCoordinates)},
        {"showdeathmessages", Bool(fShowDeathMessages)},
        {"showtags", Bool(fShowTags)},
        {"spawnMobs", Bool(fSpawnMobs)},
        {"spawnradius", Int(fSpawnRadius)},
        {"SpawnV1Villagers", Bool(fSpawnV1Villagers)},
        {"SpawnX", Int(fSpawnX)},
        {"SpawnY", Int(fSpawnY)},
        {"SpawnZ", Int(fSpawnZ)},
        {"startWithMapEnabled", Bool(fStartWithMapEnabled)},
        {"StorageVersion", Int(fStorageVersion)},
        {"texturePacksRequired", Bool(fTexturePacksRequired)},
        {"Time", Long(fTime)},
        {"tntexplodes", Bool(fTntExplodes)},
        {"useMsaGamertagsOnly", Bool(fUseMsaGamertagsOnly)},
        {"worldStartCount", Long(fWorldStartCount)},
        {"XBLBroadcastIntent", Int(fXBLBroadcastIntent)},
        {"isFromLockedTemplate", Bool(fIsFromLockedTemplate)},
        {"isFromWorldTemplate", Bool(fIsFromWorldTemplate)},
        {"isSingleUseWorld", Bool(fIsSingleUseWorld)},
        {"isWorldTemplateOptionLocked", Bool(fIsWorldTemplateOptionLocked)},
        {"keepinventory", Bool(fKeepInventory)},
        {"mobgriefing", Bool(fMobGriefing)},
        {"lastOpenedWithVersion", fLastOpenedWithVersion.toListTag()},
        {"lightningLevel", Float(fLightningLevel)},
        {"lightningTime", Int(fLightningTime)},
        {"limitedWorldDepth", Int(fLimitedWorldDepth)},
        {"limitedWorldWidth", Int(fLimitedWorldWidth)},
        {"maxcommandchainlength", Int(fMaxCommandChainLength)},
    });
    return root;
  }

  [[nodiscard]] bool write(std::filesystem::path path) const {
    auto stream = std::make_shared<mcfile::stream::FileOutputStream>(path);
    mcfile::stream::OutputStreamWriter w(stream, {.fLittleEndian = true});
    if (!w.write((uint32_t)8)) {
      return false;
    }
    if (!w.write((uint32_t)0)) {
      return false;
    }
    if (!w.write(static_cast<uint8_t>(mcfile::nbt::Tag::Type::Compound))) {
      return false;
    }
    if (!w.write(std::string(""))) {
      return false;
    }
    auto tag = this->toCompoundTag();
    if (!tag->write(w)) {
      return false;
    }
    uint64_t pos = stream->pos();
    if (!stream->seek(4)) {
      return false;
    }
    if (!w.write((uint32_t)pos - 8)) {
      return false;
    }
    if (!stream->seek(pos)) {
      return false;
    }
    return w.write((uint8_t)0);
  }

  static LevelData Import(CompoundTag const &tag) {
    LevelData ret;
    auto data = tag.query("/Data")->asCompound();
    if (!data) {
      return ret;
    }

#define I(__name, __type, __key) ret.__name = data->__type(__key, ret.__name)
    I(fCommandsEnabled, boolean, "allowCommands");
    I(fTime, int64, "DayTime");
    I(fDifficulty, byte, "Difficulty");
    I(fGameType, int32, "GameType");
    I(fLevelName, string, "LevelName");
    I(fRainTime, int32, "rainTime");
    I(fSpawnX, int32, "SpawnX");
    I(fSpawnY, int32, "SpawnY");
    I(fSpawnZ, int32, "SpawnZ");
    I(fLightningTime, int32, "thunderTime");
    I(fCurrentTick, int64, "Time");
    ret.fLastPlayed = data->int64("LastPlayed", ret.fLastPlayed * 1000) / 1000;
#undef I

    auto gameRules = data->compoundTag("GameRules");
    if (gameRules) {
#define S(__name, __var)                       \
  auto __name = gameRules->stringTag(#__name); \
  if (__name) {                                \
    __var = __name->fValue == "true";          \
  }
#define I(__name, __var)                       \
  auto __name = gameRules->stringTag(#__name); \
  if (__name) {                                \
    auto v = strings::Toi(__name->fValue);     \
    if (v) {                                   \
      __var = *v;                              \
    }                                          \
  }
      // announceAdvancements
      S(commandBlockOutput, ret.fCommandBlockOutput);
      // disableElytraMovementCheck
      // disableRaids
      S(doDaylightCycle, ret.fDoDaylightCycle);
      S(doEntityDrops, ret.fDoEntityDrops);
      S(doFireTick, ret.fDoFireTick);
      S(doImmediateRespawn, ret.fDoImmediateRespawn);
      S(doInsomnia, ret.fDoInsomnia);
      // doLimitedCrafting
      S(doMobLoot, ret.fDoMobLoot);
      S(doMobSpawning, ret.fDoMobSpawning);
      S(doTileDrops, ret.fDoTileDrops);
      S(doWeatherCycle, ret.fDoWeatherCycle);
      S(drowningDamage, ret.fDrowningDamage);
      S(fallDamage, ret.fFallDamage);
      S(fireDamage, ret.fFireDamage);
      // forgiveDeadPlayers
      S(keepInventory, ret.fKeepInventory);
      // logAdminCommands
      I(maxCommandChainLength, ret.fMaxCommandChainLength);
      // maxEntityCramming
      S(mobGriefing, ret.fMobGriefing);
      S(naturalRegeneration, ret.fNaturalRegeneration);
      I(randomTickSpeed, ret.fRandomTickSpeed);
      // reducedDebugInfo
      S(sendCommandFeedback, ret.fSendCommandFeedback);
      S(showDeathMessages, ret.fShowDeathMessages);
      I(spawnRadius, ret.fSpawnRadius);
      // spectatorsGenerateChunks
      // universalAnger
#undef S
#undef I

      auto worldGenSettings = data->compoundTag("WorldGenSettings");
      if (worldGenSettings) {
        ret.fRandomSeed = worldGenSettings->int64("seed", ret.fRandomSeed);
      }
    }
    return ret;
  }

  static std::optional<std::string> TheEndData(CompoundTag const &tag, size_t numAutonomousEntities, std::unordered_set<Pos3i, Pos3iHasher> const &endPortalsInEndDimension) {
    using namespace props;
    using namespace mcfile::nbt;

    auto data = tag.query("/Data")->asCompound();
    if (!data) {
      return std::nullopt;
    }

    CompoundTag const *dragonFight = data->query("DragonFight")->asCompound();
    if (!dragonFight) {
      dragonFight = data->query("DimensionData/1/DragonFight")->asCompound();
    }
    if (!dragonFight) {
      return std::nullopt;
    }

    auto fight = std::make_shared<CompoundTag>();

    fight->set("DragonFightVersion", Byte(kDragonFightVersion));

    auto killed = dragonFight->boolean("DragonKilled", false);
    fight->set("DragonKilled", Bool(numAutonomousEntities == 0 && killed));

    auto previouslyKilled = dragonFight->byteTag("PreviouslyKilled");
    if (previouslyKilled) {
      fight->set("PreviouslyKilled", Bool(previouslyKilled->fValue != 0));
    }

    auto uuid = GetUUID(*dragonFight, {.fIntArray = "Dragon"});
    if (uuid) {
      fight->set("DragonUUID", Long(*uuid));
      fight->set("DragonSpawned", Bool(true));
    } else {
      fight->set("DragonUUID", Long(-1));
      fight->set("DragonSpawned", Bool(true));
      fight->set("IsRespawning", Bool(false));
    }

    auto gateways = dragonFight->listTag("Gateways");
    if (gateways) {
      auto v = std::make_shared<ListTag>(Tag::Type::Int);
      for (auto const &it : *gateways) {
        auto p = it->asInt();
        if (!p) {
          v = nullptr;
          break;
        }
        v->push_back(Int(p->fValue));
      }
      if (v) {
        fight->set("Gateways", v);
      }
    }

    auto exitPortalLocation = dragonFight->compoundTag("ExitPortalLocation");
    std::optional<Pos3i> exitLocation;
    if (exitPortalLocation) {
      auto x = exitPortalLocation->int32("X");
      auto y = exitPortalLocation->int32("Y");
      auto z = exitPortalLocation->int32("Z");
      if (x && y && z) {
        exitLocation = Pos3i(*x, *y, *z);
      }
    }
    if (!exitLocation) {
      auto pos = ExitPortalAltitude(endPortalsInEndDimension);
      if (pos) {
        exitLocation = *pos;
      }
    }
    if (exitLocation) {
      auto v = std::make_shared<ListTag>(Tag::Type::Int);
      v->push_back(Int(exitLocation->fX));
      v->push_back(Int(exitLocation->fY));
      v->push_back(Int(exitLocation->fZ));
      fight->set("ExitPortalLocation", v);
    }

    auto root = std::make_shared<CompoundTag>();
    auto d = std::make_shared<CompoundTag>();
    d->set("DragonFight", fight);
    root->set("data", d);

    auto s = std::make_shared<mcfile::stream::ByteStream>();
    mcfile::stream::OutputStreamWriter w(s, {.fLittleEndian = true});
    if (!root->writeAsRoot(w)) {
      return std::nullopt;
    }

    std::vector<uint8_t> buffer;
    s->drain(buffer);
    std::string str((char const *)buffer.data(), buffer.size());
    return str;
  }

  static std::optional<std::string> MobEvents(CompoundTag const &tag) {
    using namespace std;
    using namespace props;
    using namespace mcfile::nbt;

    auto gameRulesTag = tag.query("/Data/GameRules");
    if (!gameRulesTag) {
      return nullopt;
    }
    auto gameRules = gameRulesTag->asCompound();
    if (!gameRules) {
      return nullopt;
    }
    auto ret = make_shared<CompoundTag>();
    ret->set("events_enabled", Bool(true));
    ret->set("minecraft:ender_dragon_event", Bool(true));
    ret->set("minecraft:pillager_patrols_event", Bool(gameRules->boolean("doPatrolSpawning", true)));
    ret->set("minecraft:wandering_trader_event", Bool(gameRules->boolean("doTraderSpawning", true)));

    auto s = std::make_shared<mcfile::stream::ByteStream>();
    mcfile::stream::OutputStreamWriter w(s, {.fLittleEndian = true});
    if (!ret->writeAsRoot(w)) {
      return std::nullopt;
    }

    std::vector<uint8_t> buffer;
    s->drain(buffer);
    std::string str((char const *)buffer.data(), buffer.size());
    return str;
  }

  static std::shared_ptr<CompoundTag> Read(std::filesystem::path const &javaEditionLevelDat) {
    using namespace std;
    namespace fs = std::filesystem;
    using namespace mcfile::stream;

    error_code ec;
    if (!fs::is_regular_file(javaEditionLevelDat, ec)) {
      return nullptr;
    }
    if (ec) {
      return nullptr;
    }

    vector<uint8_t> buffer;
    {
      vector<char> buf(512);
      gzFile f = mcfile::File::GzOpen(javaEditionLevelDat, mcfile::File::Mode::Read);
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

    auto root = make_shared<mcfile::nbt::CompoundTag>();
    auto bs = make_shared<ByteStream>(buffer);
    InputStreamReader r(bs);
    if (!root->read(r)) {
      return nullptr;
    }

    return root;
  }

private:
  static std::optional<Pos3i> ExitPortalAltitude(std::unordered_set<Pos3i, Pos3iHasher> const &blocks) {
    std::vector<Pos3i> candidates;
    for (auto const &pos : blocks) {
      if (IsValidExitPortal(pos, blocks)) {
        candidates.push_back(Pos3i(pos.fX + 1, pos.fY, pos.fZ + 2));
      }
    }
    if (candidates.empty()) {
      return std::nullopt;
    }
    if (candidates.size() > 1) {
      Pos3i origin(0, 0, 0);
      std::sort(candidates.begin(), candidates.end(), [origin](Pos3i const &a, Pos3i const &b) {
        auto distanceA = Pos3i::DistanceSquare(a, origin);
        auto distanceB = Pos3i::DistanceSquare(b, origin);
        return distanceA < distanceB;
      });
    }
    return candidates[0];
  }

  static bool IsValidExitPortal(Pos3i p, std::unordered_set<Pos3i, Pos3iHasher> const &blocks) {
    static std::vector<Pos3i> const portalPlacement = {
        // line1
        Pos3i(0, 0, 0),
        Pos3i(1, 0, 0),
        Pos3i(2, 0, 0),
        // line2
        Pos3i(-1, 0, 1),
        Pos3i(0, 0, 1),
        Pos3i(1, 0, 1),
        Pos3i(2, 0, 1),
        Pos3i(3, 0, 1),
        // line3
        Pos3i(-1, 0, 2),
        Pos3i(0, 0, 2),
        Pos3i(2, 0, 2),
        Pos3i(3, 0, 2),
        // line4
        Pos3i(-1, 0, 3),
        Pos3i(0, 0, 3),
        Pos3i(1, 0, 3),
        Pos3i(2, 0, 3),
        Pos3i(3, 0, 3),
        // line5
        Pos3i(0, 0, 4),
        Pos3i(1, 0, 4),
        Pos3i(2, 0, 4),
    };
    if (blocks.size() < portalPlacement.size()) {
      return false;
    }
    for (auto const &it : portalPlacement) {
      Pos3i test(p.fX + it.fX, p.fY + it.fY, p.fZ + it.fZ);
      if (blocks.find(test) == blocks.end()) {
        return false;
      }
    }
    return true;
  }
};

} // namespace je2be::tobe
