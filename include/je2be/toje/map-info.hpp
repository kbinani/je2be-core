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
      auto ret = Compound();
      ret->set("id", String(fId));
      ret->set("rot", Double(fRot));
      ret->set("type", Byte(fType));
      ret->set("x", Double(fX));
      ret->set("z", Double(fZ));
      return ret;
    }
  };

  struct Map {
    int fNumber;
    std::vector<Decoration> fDecorations;
  };

  static std::optional<Map> Parse(std::string const &value, int64_t &outMapId) {
    using namespace std;
    using namespace leveldb;
    auto tag = CompoundTag::Read(value, {.fLittleEndian = true});
    if (!tag) {
      return nullopt;
    }
    auto mapId = tag->int64("mapId");
    if (!mapId) {
      return nullopt;
    }
    outMapId = *mapId;

    Map map;
    auto xCenter = tag->int32("xCenter");
    auto zCenter = tag->int32("zCenter");
    if (!xCenter || !zCenter) {
      return nullopt;
    }
    auto scale = tag->byte("scale");
    if (!scale) {
      return nullopt;
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
    return map;
  }

  void add(Map map, int64_t mapId) {
    map.fNumber = fMaps.size();
    fMaps.insert(std::make_pair(mapId, map));
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
