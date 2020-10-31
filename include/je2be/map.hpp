#pragma once

namespace j2b {

class Map {
    using CompoundTag = mcfile::nbt::CompoundTag;

public:
	static int64_t UUID(int32_t javaMapId, uint8_t scale) {
        XXH32_state_t* state = XXH32_createState();
        XXH32_hash_t seed = 0;
        XXH32_reset(state, seed);
        XXH32_update(state, &javaMapId, sizeof(javaMapId));
        XXH32_hash_t hash = XXH32_digest(state);
        XXH32_freeState(state);
        uint64_t s = (uint64_t)hash * 10 + (4 - scale);
        return *(int64_t*)&s;
	}

    static bool Convert(int32_t javaMapId, std::filesystem::path const& input, DbInterface &db) {
        using namespace std;
        namespace fs = std::filesystem;
        using namespace mcfile::stream;
        using namespace props;
        using namespace mcfile::nbt;

        auto root = JavaEditionMap::Read(input, javaMapId);
        if (!root) return false;

        auto data = root->query("/data")->asCompound();
        if (!data) return false;

        auto dimension = GetString(*data, "dimension");
        auto locked = GetBool(*data, "locked");
        auto scale = GetByte(*data, "scale");
        auto xCenter = GetInt(*data, "xCenter");
        auto zCenter = GetInt(*data, "zCenter");
        auto colors = data->query("colors")->asByteArray();
        auto unlimitedTracking = GetBool(*data, "unlimitedTracking");

        if (!dimension || !locked || !scale || !xCenter || !zCenter || !colors || !unlimitedTracking) return false;

        for (int beScale = 0; beScale <= 4; beScale++) {
            int64_t uuid = UUID(javaMapId, beScale);
            auto ret = make_shared<CompoundTag>();
            int8_t outDimension = 0;
            if (*dimension == "minecraft:overworld") {
                outDimension = 0;
            } else if (*dimension == "minecraft:the_nether") {
                outDimension = 1;
            } else if (*dimension == "minecraft:the_end") {
                outDimension = 2;
            }
            ret->fValue["dimension"] = Byte(outDimension);
            ret->fValue["fullyExplored"] = Bool(false); //?
            ret->fValue["height"] = Short(128);
            ret->fValue["mapId"] = Long(uuid);
            ret->fValue["mapLocked"] = Bool(*locked);
            if (beScale == 4) {
                ret->fValue["parentMapId"] = Long(-1);
            } else {
                int64_t parent = UUID(javaMapId, beScale + 1);
                ret->fValue["parentMapId"] = Long(parent);
            }
            ret->fValue["scale"] = Byte(beScale);
            ret->fValue["unlimitedTracking"] = Bool(*unlimitedTracking);
            ret->fValue["width"] = Short(128);
            ret->fValue["xCenter"] = Int(*xCenter);
            ret->fValue["zCenter"] = Int(*zCenter);

            std::vector<uint8_t> outColors(65536);
            //TODO:
            auto outColorsTag = make_shared<ByteArrayTag>(outColors);
            ret->fValue["colors"] = outColorsTag;

            auto decorations = make_shared<ListTag>();
            decorations->fType = Tag::TAG_Compound;
            //TODO:
            ret->fValue["decorations"] = decorations;

            vector<uint8_t> serialized;
            {
                auto out = make_shared<ByteStream>();
                OutputStreamWriter w(out, {.fLittleEndian = true});
                w.write((uint8_t)Tag::TAG_Compound);
                w.write(string());
                ret->write(w);
                w.write((uint8_t)Tag::TAG_End);
                out->drain(serialized);
            }
            auto key = Key::Map(uuid);
            db.put(key, leveldb::Slice((char*)serialized.data(), serialized.size()));
        }

        return true;
    }

private:
	Map() = delete;
};

}
