#include <minecraft-file.hpp>
#include <leveldb/db.h>
#include <leveldb/zlib_compressor.h>
#include <leveldb/decompress_allocator.h>
#include <string>
#include <iostream>
#include <memory>
#include <sys/stat.h>

static void WriteBELevelDat(mcfile::nbt::CompoundTag const& tag, std::string const& path) {
    auto stream = std::make_shared<mcfile::stream::FileOutputStream>(path);
    mcfile::stream::OutputStreamWriter w(stream, { .fLittleEndian = true });
    w.write((uint32_t)8);
    w.write((uint32_t)0);
    w.write(mcfile::nbt::Tag::TAG_Compound);
    w.write(std::string(""));
    tag.write(w);
    long pos = stream->pos();
    stream->seek(4);
    w.write((uint32_t)pos - 8);
    stream->seek(pos);
    w.write((uint8_t)0);
}

static std::shared_ptr<mcfile::nbt::ByteTag> Bool(bool b) {
    return std::make_shared<mcfile::nbt::ByteTag>(b ? 1 : 0);
}

static std::shared_ptr<mcfile::nbt::ByteTag> ByteV(int8_t v) {
    uint8_t t = *(uint8_t *)&v;
    return std::make_shared<mcfile::nbt::ByteTag>(t);
}

static std::shared_ptr<mcfile::nbt::IntTag> Int(int32_t v) {
    return std::make_shared<mcfile::nbt::IntTag>(v);
}

static std::shared_ptr<mcfile::nbt::LongTag> Long(int64_t v) {
    return std::make_shared<mcfile::nbt::LongTag>(v);
}

static std::shared_ptr<mcfile::nbt::StringTag> String(char const* v) {
    return std::make_shared<mcfile::nbt::StringTag>(v);
}

static std::shared_ptr<mcfile::nbt::FloatTag> Float(float v) {
    return std::make_shared<mcfile::nbt::FloatTag>(v);
}

static std::shared_ptr<mcfile::nbt::CompoundTag> Abilities() {
    auto a = std::make_shared<mcfile::nbt::CompoundTag>();
    a->fValue = {
        {"attackmobs", Bool(true)},
        {"attackplayers", Bool(true)},
        {"build", Bool(true)},
        {"doorsandswitches", Bool(true)},
        {"flying", Bool(false)},
        {"flySpeed", Float(0.05f)},
        {"instabuild", Bool(false)},
        {"invulnerable", Bool(false)},
        {"lightning", Bool(false)},
        {"mayfly", Bool(false)},
        {"mine", Bool(true)},
        {"op", Bool(false)},
        {"opencontainers", Bool(true)},
        {"permissionsLevel", Int(0)},
        {"playerPermissionsLevel", Int(1)},
        {"teleport", Bool(false)},
        {"walkSpeed", Float(0.1f)},
    };
    return a;
}

static std::shared_ptr<mcfile::nbt::ListTag> MinimumCompatibleClientVersion() {
    auto l = std::make_shared<mcfile::nbt::ListTag>();
    l->fType = mcfile::nbt::Tag::TAG_Int;
    l->fValue = {
        Int(1),
        Int(16),
        Int(0),
        Int(0),
        Int(0),
    };
    return l;
}

static std::shared_ptr<mcfile::nbt::ListTag> LastOpenedWithVersion() {
    auto l = std::make_shared<mcfile::nbt::ListTag>();
    l->fType = mcfile::nbt::Tag::TAG_Int;
    l->fValue = {
        Int(1),
        Int(16),
        Int(40),
        Int(2),
        Int(0),
    };
    return l;
}

static std::shared_ptr<mcfile::nbt::CompoundTag> ConvertJELevelDat() {
    using namespace std;
    using namespace mcfile::nbt;
    auto root = std::make_shared<CompoundTag>();
    root->fValue = {
        {"abilities", Abilities()},
        {"MinimumCompatibleClientVersion", MinimumCompatibleClientVersion()},
        {"baseGameVersion", String("*")},
        {"BiomeOverride", String("")},
        {"bonusChestEnabled", Bool(false)},
        {"bonusChestSpawned", Bool(false)},
        {"CenterMapsToOrigin", ByteV(-7)},
        {"commandblockoutput", Bool(true)},
        {"commandblocksenabled", Bool(true)},
        {"commandsEnabled", Bool(false)},
        {"ConfirmedPlatformLockedContent", Bool(false)},
        {"currentTick", Long(122)},
        {"Difficulty", Int(0)},
        {"FlatWorldLayers", String("null\x0a")},
        {"dodaylightcycle", Bool(true)},
        {"doentitydrops", Bool(true)},
        {"dofiretick", Bool(true)},
        {"doimmediaterespawn", Bool(false)},
        {"doinsomnia", Bool(true)},
        {"domobloot", Bool(true)},
        {"domobspawning", Bool(true)},
        {"dotiledrops", Bool(true)},
        {"doweathercycle", Bool(true)},
        {"drowningdamage", Bool(true)},
        {"educationFeaturesEnabled", Bool(false)},
        {"eduOffer", Int(0)},
        {"experimentalgameplay", Bool(false)},
        {"falldamage", Bool(true)},
        {"firedamage", Bool(true)},
        {"ForceGameType", Bool(false)},
        {"functioncommandlimit", Int(10000)},
        {"GameType", Int(1)},
        {"Generator", Int(1)},
        {"hasBeenLoadedInCreative", Bool(false)},
        {"hasLockedBehaviorPack", Bool(false)},
        {"hasLockedResourcePack", Bool(false)},
        {"immutableWorld", Bool(false)},
        {"InventoryVersion", String("1.16.40")},
        {"LANBroadcast", Bool(true)},
        {"LANBroadcastIntent", Bool(true)},
        {"LastPlayed", Long(1602694933)},
        {"LevelName", String("-iyHXyBCAAA=")},
        {"LimitedWorldOriginX", Int(1852)},
        {"LimitedWorldOriginY", Int(32767)},
        {"LimitedWorldOriginZ", Int(4)},
        {"MultiplayerGame", Bool(true)},
        {"MultiplayerGameIntent", Bool(true)},
        {"naturalregeneration", Bool(true)},
        {"NetherScale", Int(8)},
        {"NetworkVersion", Int(408)},
        {"Platform", Int(2)},
        {"PlatformBroadcastIntent", Int(3)},
        {"prid", String("")},
        {"pvp", Bool(true)},
        {"rainLevel", Float(0)},
        {"rainTime", Int(5233)},
        {"RandomSeed", Long(123456)},
        {"randomtickspeed", Int(1)},
        {"requiresCopiedPackRemovalCheck", Bool(false)},
        {"sendcommandfeedback", Bool(true)},
        {"serverChunkTickRange", Int(10)},
        {"showcoordinates", Bool(true)},
        {"showdeathmessages", Bool(true)},
        {"showtags", Bool(true)},
        {"spawnMobs", Bool(true)},
        {"spawnradius", Int(5)},
        {"SpawnV1Villagers", Bool(false)},
        {"SpawnX", Int(251)},
        {"SpawnY", Int(85)},
        {"SpawnZ", Int(-91)},
        {"startWithMapEnabled", Bool(false)},
        {"StorageVersion", Int(8)},
        {"texturePacksRequired", Bool(false)},
        {"Time", Long(281942257)},
        {"tntexplodes", Bool(true)},
        {"useMsaGamertagsOnly", Bool(false)},
        {"worldStartCount", Long(4294967294)},
        {"XBLBroadcastIntent", Int(3)},
        {"isFromLockedTemplate", Bool(false)},
        {"isFromWorldTemplate", Bool(false)},
        {"isSingleUseWorld", Bool(false)},
        {"isWorldTemplateOptionLocked", Bool(false)},
        {"keepinventory", Bool(false)},
        {"mobgriefing", Bool(true)},
        {"lastOpenedWithVersion", LastOpenedWithVersion()},
        {"lightningLevel", Float(0)},
        {"lightningTime", Int(115427)},
        {"limitedWorldDepth", Int(16)},
        {"limitedWorldWidth", Int(16)},
        {"maxcommandchainlength", Int(65535)},
    };
    return root;
}

int main(int argc, char *argv[]) {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;
    using namespace mcfile::nbt;
    
    if (argc != 3) {
        return -1;
    }
    string const input = argv[1];
    string const output = argv[2];

    int const mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    std::string const dbRoot = output + "/db";
    ::mkdir(output.c_str(), mode);
    ::mkdir(dbRoot.c_str(), mode);
    auto const& root = ConvertJELevelDat();
    auto stream = make_shared<mcfile::stream::FileInputStream>(output.c_str());
    WriteBELevelDat(*root, output + "/level.dat");

    DB *db;
    Options options;
    options.compressors[0] = new ZlibCompressorRaw(-1);
    options.compressors[1] = new ZlibCompressor();
    Status status = DB::Open(options, dbRoot.c_str(), &db);
    if (!status.ok()) {
        cerr << status.ToString() << endl;
        return -1;
    }

    delete db;

    return 0;
}
