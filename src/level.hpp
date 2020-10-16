#pragma once

namespace j2e {

class Level {
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
            a->fValue = {
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
            };
            return a;
        }
    };

    Abilities fAbilities;
    Version fMinimumCompatibleClientVersion = Version(1, 16, 0, 0, 0);
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
    std::string fInventoryVersion = "1.16.40";
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
    int32_t fNetworkVersion = 408;
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
    int32_t fStorageVersion = 8;
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
    Version fLastOpenedWithVersion = Version(1, 16, 40, 2, 0);
    float fLightningLevel = 0;
    int32_t fLightningTime = 0;
    int32_t fLimitedWorldDepth = 16;
    int32_t fLimitedWorldWidth = 16;
    int32_t fMaxCommandChainLength = 65535;

    std::shared_ptr<mcfile::nbt::CompoundTag> toCompoundTag() const {
        using namespace props;

        auto root = std::make_shared<mcfile::nbt:: CompoundTag>();
        root->fValue = {
            {"abilities", fAbilities.toCompoundTag()},
            {"MinimumCompatibleClientVersion", fMinimumCompatibleClientVersion.toListTag()},
            {"baseGameVersion", String(fBaseGameVersion)},
            {"BiomeOverride", String(fBiomeOverride)},
            {"bonusChestEnabled", Bool(fBonusChestEnabled)},
            {"bonusChestSpawned", Bool(fBonusChestSpawned)},
            {"CenterMapsToOrigin", ByteV(fCenterMapsToOrigin)},
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
        };
        return root;
    }

    void write(std::string path) const {
        auto stream = std::make_shared<mcfile::stream::FileOutputStream>(path);
        mcfile::stream::OutputStreamWriter w(stream, { .fLittleEndian = true });
        w.write((uint32_t)8);
        w.write((uint32_t)0);
        w.write(mcfile::nbt::Tag::TAG_Compound);
        w.write(std::string(""));
        auto tag = this->toCompoundTag();
        tag->write(w);
        long pos = stream->pos();
        stream->seek(4);
        w.write((uint32_t)pos - 8);
        stream->seek(pos);
        w.write((uint8_t)0);
    }
};

}
