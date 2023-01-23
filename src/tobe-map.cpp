#include "tobe/_map.hpp"

#include <je2be/nbt.hpp>

#include "db/_db-interface.hpp"
#include "item/_map-color.hpp"
#include "tobe/_java-edition-map.hpp"

#include <xxhash32.h>

#include <optional>
#include <tuple>

namespace je2be::tobe {

class Map::Impl {
  Impl() = delete;

public:
  static i64 UUID(i32 javaMapId, u8 scale) {
    u32 const seed = 0;
    u32 hash = XXHash32::hash(&javaMapId, sizeof(javaMapId), seed);
    u64 s = (u64)hash * 10 + (4 - scale);
    return *(i64 *)&s;
  }

  static bool Convert(i32 javaMapId, CompoundTag const &item, std::filesystem::path const &input, Options const &opt, DbInterface &db) {
    using namespace std;
    namespace fs = std::filesystem;
    using namespace mcfile::stream;

    auto root = JavaEditionMap::Read(input, opt, javaMapId);
    if (!root) {
      return false;
    }

    auto data = root->query("/data")->asCompound();
    if (!data) {
      return false;
    }

    auto dimensionString = data->string("dimension");
    auto dimensionInt = data->int32("dimension");
    auto dimensionByte = data->byte("dimension");

    i8 outDimension = 0;
    if (dimensionString) {
      if (*dimensionString == "minecraft:overworld") {
        outDimension = 0;
      } else if (*dimensionString == "minecraft:the_nether") {
        outDimension = 1;
      } else if (*dimensionString == "minecraft:the_end") {
        outDimension = 2;
      }
    } else if (dimensionInt) {
      // <= 1.15
      outDimension = *dimensionInt;
    } else if (dimensionByte) {
      outDimension = *dimensionByte;
    }
    auto locked = data->boolean("locked", false);
    auto scale = data->byte("scale");
    auto xCenter = data->int32("xCenter");
    auto zCenter = data->int32("zCenter");
    auto colors = data->byteArrayTag("colors");
    auto unlimitedTracking = data->boolean("unlimitedTracking", false);

    if (!scale || !xCenter || !zCenter || !colors) {
      return false;
    }

    for (u8 beScale = 0; beScale <= 4; beScale++) {
      i64 uuid = UUID(javaMapId, beScale);
      auto ret = Compound();
      ret->set("dimension", Byte(outDimension));
      ret->set("fullyExplored", Bool(false)); //?
      ret->set("height", Short(128));
      ret->set("mapId", Long(uuid));
      ret->set("mapLocked", Bool(locked));
      if (beScale == 4) {
        ret->set("parentMapId", Long(-1));
      } else {
        i64 parent = UUID(javaMapId, beScale + 1);
        ret->set("parentMapId", Long(parent));
      }
      ret->set("scale", Byte(beScale));
      ret->set("unlimitedTracking", Bool(unlimitedTracking));
      ret->set("width", Short(128));
      ret->set("xCenter", Int(*xCenter));
      ret->set("zCenter", Int(*zCenter));

      std::vector<u8> outColors(65536);
      auto decorations = List<Tag::Type::Compound>();

      if (beScale == *scale) {
        int i = 0;
        int j = 0;
        vector<u8> const &colorsArray = colors->value();
        for (int y = 0; y < 128; y++) {
          for (int x = 0; x < 128; x++, i++) {
            u8 colorId = colorsArray[i];
            Rgba color = MapColor::RgbaFromId(colorId);
            outColors[j] = color.fR;
            outColors[j + 1] = color.fG;
            outColors[j + 2] = color.fB;
            outColors[j + 3] = color.fA;
            j += 4;
          }
        }

        auto frames = data->listTag("frames");
        if (frames) {
          for (auto const &it : *frames) {
            auto frame = it->asCompound();
            if (!frame) {
              continue;
            }
            auto pos = frame->compoundTag("Pos");
            auto rotation = frame->int32("Rotation");
            if (!pos || !rotation) {
              continue;
            }
            auto x = pos->int32("X");
            auto y = pos->int32("Y");
            auto z = pos->int32("Z");
            if (!x || !y || !z) {
              continue;
            }

            i32 rot = 0;
            if (*rotation == -90) {
              rot = 0; // top; arrow facing to east in Java, arrow facing to
                       // south in Bedrock
            } else if (*rotation == 180) {
              rot = 8; // north; arrow facing to north
            } else if (*rotation == 270) {
              rot = 12; // east; arrow facing to east
            } else if (*rotation == 0) {
              rot = 0; // south; arrow facing to south
            } else {
              // nop
              // west; *rotation == -90; arrow facing to west
              // bottom; *rotation == -90; arrow facing to east in Java, arrow
              // facing to south in Bedrock
            }

            auto frameData = Compound();
            frameData->set("rot", Int(rot));
            frameData->set("type", Int(1));
            auto [markerX, markerY] = MarkerPosition(*x, *z, *xCenter, *zCenter, *scale);
            if (markerX < -128 || 128 < markerX || markerY < -128 || 128 < markerY) {
              continue;
            }
            frameData->set("x", Int(markerX));
            frameData->set("y", Int(markerY));

            auto key = Compound();
            key->set("blockX", Int(*x));
            key->set("blockY", Int(*y));
            key->set("blockZ", Int(*z));
            key->set("type", Int(1));

            auto decoration = Compound();
            decoration->set("data", frameData);
            decoration->set("key", key);

            decorations->push_back(decoration);
          }
        }

        auto inDecorations = item.query("tag/Decorations")->asList();
        if (inDecorations) {
          for (auto const &d : *inDecorations) {
            auto e = d->asCompound();
            if (!e) {
              continue;
            }
            auto type = e->byte("type");
            auto x = e->float64("x");
            auto z = e->float64("z");
            if (!type || !x || !z) {
              continue;
            }
            i32 outType = 1;
            if (*type == 9) {
              outType = 15; // id = "+", monument
            } else if (*type == 8) {
              outType = 14; // id = "+", mansion
            } else if (*type == 26) {
              outType = 4; // id = "+", buried treasure
            }

            auto frameData = Compound();
            frameData->set("rot", Int(8));
            frameData->set("type", Int(outType));
            auto [markerX, markerY] = MarkerPosition(*x, *z, *xCenter, *zCenter, *scale);
            if (markerX < -128 || 128 < markerX || markerY < -128 || 128 < markerY) {
              continue;
            }
            frameData->set("x", Int(markerX));
            frameData->set("y", Int(markerY));

            auto key = Compound();
            key->set("blockX", Int((i32)*x));
            key->set("blockZ", Int((i32)*z));
            key->set("blockY", Int(64)); // fixed value?
            key->set("type", Int(1));    //?

            auto decoration = Compound();
            decoration->set("data", frameData);
            decoration->set("key", key);

            decorations->push_back(decoration);
          }
        }
      }
      auto outColorsTag = make_shared<ByteArrayTag>(outColors);
      ret->set("colors", outColorsTag);

      ret->set("decorations", decorations);

      auto serialized = CompoundTag::Write(*ret, mcfile::Endian::Little);
      if (!serialized) {
        return false;
      }
      auto key = mcfile::be::DbKey::Map(uuid);
      db.put(key, leveldb::Slice(*serialized));
    }

    return true;
  }

private:
  static std::tuple<i32, i32> MarkerPosition(i32 blockX, i32 blockZ, i32 xCenter, i32 zCenter, i32 scale) {
    i32 size = 128 * (i32)ceil(pow(2, scale));
    /*
     * i32 x0 = xCenter - size / 2;
     * i32 z0 = zCenter - size / 2;
     * marker (-128, -128) corresponds to block (x0, z0)
     * marker (128, 128) corresponds to block (x0 + size, z0 + size)
     */
    i32 markerX = (blockX - xCenter) * 256 / size;
    i32 markerY = (blockZ - zCenter) * 256 / size;
    return std::make_tuple(markerX, markerY);
  }
};

i64 Map::UUID(i32 javaMapId, u8 scale) {
  return Impl::UUID(javaMapId, scale);
}

bool Map::Convert(i32 javaMapId, CompoundTag const &item, std::filesystem::path const &input, Options const &opt, DbInterface &db) {
  return Impl::Convert(javaMapId, item, input, opt, db);
}

} // namespace je2be::tobe
