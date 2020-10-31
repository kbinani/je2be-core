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
            int i = 0;
            int j = 0;
            vector<uint8_t> const& colorsArray = colors->value();
            for (int y = 0; y < 128; y++) {
                for (int x = 0; x < 128; x++, i++) {
                    uint8_t colorId = colorsArray[i];
                    Rgba color = RgbaFromId(colorId);
                    outColors[j] = color.fR;
                    outColors[j + 1] = color.fG;
                    outColors[j + 2] = color.fB;
                    outColors[j + 3] = color.fA;
                    j += 4;
                }
            }
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
    static Rgba RgbaFromId(uint8_t colorId) {
        static std::vector<Rgba> const mapping = {
            Rgba(0, 0, 0, 0),
            Rgba(127, 178, 56),
            Rgba(247, 233, 163),
            Rgba(199, 199, 199),
            Rgba(255, 0, 0),
            Rgba(160, 160, 255),
            Rgba(167, 167, 167),
            Rgba(0, 124, 0),
            Rgba(255, 255, 255),
            Rgba(164, 168, 184),
            Rgba(151, 109, 77),
            Rgba(112, 112, 112),
            Rgba(64, 64, 255),
            Rgba(143, 119, 72),
            Rgba(255, 252, 245),
            Rgba(216, 127, 51),
            Rgba(178, 76, 216),
            Rgba(102, 153, 216),
            Rgba(229, 229, 51),
            Rgba(127, 204, 25),
            Rgba(242, 127, 165),
            Rgba(76, 76, 76),
            Rgba(153, 153, 153),
            Rgba(76, 127, 153),
            Rgba(127, 63, 178),
            Rgba(51, 76, 178),
            Rgba(102, 76, 51),
            Rgba(102, 127, 51),
            Rgba(153, 51, 51),
            Rgba(25, 25, 25),
            Rgba(250, 238, 77),
            Rgba(92, 219, 213),
            Rgba(74, 128, 255),
            Rgba(0, 217, 58),
            Rgba(129, 86, 49),
            Rgba(112, 2, 0),
            Rgba(209, 177, 161),
            Rgba(159, 82, 36),
            Rgba(149, 87, 108),
            Rgba(112, 108, 138),
            Rgba(186, 133, 36),
            Rgba(103, 117, 53),
            Rgba(160, 77, 78),
            Rgba(57, 41, 35),
            Rgba(135, 107, 98),
            Rgba(87, 92, 92),
            Rgba(122, 73, 88),
            Rgba(76, 62, 92),
            Rgba(76, 50, 35),
            Rgba(76, 82, 42),
            Rgba(142, 60, 46),
            Rgba(37, 22, 16),
            Rgba(189, 48, 49),
            Rgba(148, 63, 97),
            Rgba(92, 25, 29),
            Rgba(22, 126, 134),
            Rgba(58, 142, 140),
            Rgba(86, 44, 62),
            Rgba(20, 180, 133),
        };
        uint8_t variant = 0x3 & colorId;
        uint8_t index = colorId / 4;
        if (index >= mapping.size()) {
            return mapping[0];
        }
        Rgba base = mapping[index];
        int32_t mul = 255;
        switch (variant) {
        case 0:
            mul = 180;
            break;
        case 1:
            mul = 220;
            break;
        case 3:
            mul = 135;
            break;
        case 2:
        default:
            mul = 255;
            break;
        }
        uint8_t r = (uint8_t)((int32_t)base.fR * mul / 255);
        uint8_t g = (uint8_t)((int32_t)base.fG * mul / 255);
        uint8_t b = (uint8_t)((int32_t)base.fB * mul / 255);
        return Rgba(r, g, b, base.fA);
    }

private:
	Map() = delete;
};

}
