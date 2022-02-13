#pragma once

namespace je2be::toje {

class MapInfo {
public:
  struct Decoration {
    std::string fId;
    double fRot;
    int64_t fType;
    double fX;
    double fZ;

    std::shared_ptr<CompoundTag> toCompoundTag() const {
      auto ret = std::make_shared<CompoundTag>();
      ret->set("id", props::String(fId));
      ret->set("rot", props::Double(fRot));
      ret->set("type", props::Byte(fType));
      ret->set("x", props::Double(fX));
      ret->set("z", props::Double(fZ));
      return ret;
    }
  };

  struct Map {
    int fNumber;
    std::vector<Decoration> fDecorations;
  };

  explicit MapInfo(leveldb::DB &db) {
    using namespace std;
    using namespace leveldb;
    ReadOptions ro;
    ro.fill_cache = false;
    unique_ptr<leveldb::Iterator> itr(db.NewIterator(ro));
    int mapNumber = 0;
    for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
      string dbKey = itr->key().ToString();
      if (!dbKey.starts_with("map_")) {
        continue;
      }
      Slice value = itr->value();
      auto root = make_shared<CompoundTag>();
      vector<uint8_t> buffer;
      buffer.reserve(value.size());
      copy_n(value.data(), value.size(), back_inserter(buffer));
      auto s = make_shared<mcfile::stream::ByteStream>(buffer);
      mcfile::stream::InputStreamReader isr(s, {.fLittleEndian = true});
      if (!root->read(isr)) {
        continue;
      }
      auto tag = root->compoundTag("");
      if (!tag) {
        continue;
      }
      auto mapId = tag->int64("mapId");
      if (!mapId) {
        continue;
      }
      Map map;
      map.fNumber = mapNumber;
      auto xCenter = tag->int32("xCenter");
      auto zCenter = tag->int32("zCenter");
      if (!xCenter || !zCenter) {
        continue;
      }
      auto scale = tag->byte("scale");
      if (!scale) {
        continue;
      }
      auto decorationsB = tag->listTag("decorations");
      if (decorationsB) {
        for (auto const &it : *decorationsB) {
          auto decorationB = it->asCompound();
          if (!decorationB) {
            continue;
          }
          Decoration decorationJ;
          decorationJ.fId = "+";
          auto data = decorationB->compoundTag("data");
          if (!data) {
            continue;
          }
          auto typeB = data->int32("type");
          if (!typeB) {
            continue;
          }
          auto rotB = data->int32("rot");
          if (!rotB) {
            continue;
          }
          auto markerX = data->int32("x");
          auto markerY = data->int32("y");
          if (!markerX || !markerY) {
            continue;
          }
          decorationJ.fRot = 0;
          if (rotB == 0) {
            decorationJ.fRot = 0;
          } else if (rotB == 8) {
            decorationJ.fRot = 180;
          } else if (rotB == 12) {
            decorationJ.fRot = 270;
          }
          decorationJ.fType = 0;
          if (typeB == 15) {
            decorationJ.fType = 9;
          } else if (typeB == 14) {
            decorationJ.fType = 8;
          } else if (typeB == 4) {
            decorationJ.fType = 26;
          }
          auto pos = BlockPosFromMarkerPosition(*xCenter, *zCenter, *scale, *markerX, *markerY);
          decorationJ.fX = pos.fX;
          decorationJ.fZ = pos.fZ;
          map.fDecorations.push_back(decorationJ);
        }
      }
      fMaps.insert(make_pair(*mapId, map));
      mapNumber++;
    }
  }

  std::optional<Map> mapFromUuid(int64_t uuid) const {
    auto found = fMaps.find(uuid);
    if (found == fMaps.end()) {
      return std::nullopt;
    } else {
      return found->second;
    }
  }

  static Pos2i BlockPosFromMarkerPosition(int32_t xCenter, int32_t zCenter, int32_t scale, int32_t markerX, int32_t markerZ) {
    int32_t size = 128 * (int32_t)ceil(pow(2, scale));
    int32_t blockX = markerX * size / 256 + xCenter;
    int32_t blockZ = markerZ * size / 256 + zCenter;
    return Pos2i(blockX, blockZ);
  }

private:
  std::unordered_map<int64_t, Map> fMaps;
};

} // namespace je2be::toje
