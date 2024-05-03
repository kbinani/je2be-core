#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <je2be/nbt.hpp>
#include <je2be/pos2.hpp>

#include "item/_map-decoration.hpp"

namespace je2be::bedrock {

class MapInfo {
public:
  struct Decoration {
    std::u8string fId;
    float fRotation;
    std::u8string fType;
    double fX;
    double fZ;

    CompoundTagPtr toJavaCompoundTag(int dataVersion) const {
      auto ret = Compound();
      if (dataVersion >= kDataVersionComponentIntroduced) {
        ret->set(u8"rotation", Float(fRotation));
        if (!fType.empty()) {
          ret->set(u8"type", String(u8"minecraft:" + fType));
        }
        ret->set(u8"x", Double(fX));
        ret->set(u8"z", Double(fZ));
      } else {
        ret->set(u8"id", fId);
        ret->set(u8"rot", Double(fRotation));
        if (auto type = MapDecoration::LegacyJavaTypeFromJava(fType); type) {
          ret->set(u8"type", Byte(*type));
        }
        ret->set(u8"x", Double(fX));
        ret->set(u8"z", Double(fZ));
      }
      return ret;
    }
  };

  struct Map {
    int fNumber;
    std::vector<Decoration> fDecorations;
  };

  static std::optional<Map> Parse(std::string const &value, i64 &outMapId, mcfile::Endian endian) {
    using namespace std;
    using namespace leveldb;
    auto tag = CompoundTag::Read(value, endian);
    if (!tag) {
      return nullopt;
    }
    auto mapId = tag->int64(u8"mapId");
    if (!mapId) {
      return nullopt;
    }
    outMapId = *mapId;

    Map map;
    map.fNumber = 0; // This will be set later process

    auto xCenter = tag->int32(u8"xCenter");
    auto zCenter = tag->int32(u8"zCenter");
    if (!xCenter || !zCenter) {
      return nullopt;
    }
    auto scale = tag->byte(u8"scale");
    if (!scale) {
      return nullopt;
    }
    auto decorationsB = tag->listTag(u8"decorations");
    if (decorationsB) {
      for (auto const &it : *decorationsB) {
        auto decorationB = it->asCompound();
        if (!decorationB) {
          continue;
        }
        Decoration decorationJ;
        decorationJ.fId = u8"+";
        auto data = decorationB->compoundTag(u8"data");
        if (!data) {
          continue;
        }
        auto typeB = data->int32(u8"type");
        if (!typeB) {
          continue;
        }
        auto rotB = data->int32(u8"rot");
        if (!rotB) {
          continue;
        }
        auto markerX = data->int32(u8"x");
        auto markerY = data->int32(u8"y");
        if (!markerX || !markerY) {
          continue;
        }
        decorationJ.fRotation = 0;
        if (*rotB == 0) {
          decorationJ.fRotation = 0;
        } else if (*rotB == 8) {
          decorationJ.fRotation = 180;
        } else if (*rotB == 12) {
          decorationJ.fRotation = 270;
        }
        if (auto typeJ = MapDecoration::JavaTypeFromBedrock(*typeB); typeJ) {
          decorationJ.fType = *typeJ;
        }
        auto pos = BlockPosFromMarkerPosition(*xCenter, *zCenter, *scale, *markerX, *markerY);
        decorationJ.fX = pos.fX;
        decorationJ.fZ = pos.fZ;
        map.fDecorations.push_back(decorationJ);
      }
    }
    return map;
  }

  void add(Map map, i64 mapId) {
    map.fNumber = fMaps.size();
    fMaps.insert(std::make_pair(mapId, map));
  }

  std::optional<Map> mapFromUuid(i64 uuid) const {
    auto found = fMaps.find(uuid);
    if (found == fMaps.end()) {
      return std::nullopt;
    } else {
      return found->second;
    }
  }

  static Pos2i BlockPosFromMarkerPosition(i32 xCenter, i32 zCenter, i32 scale, i32 markerX, i32 markerZ) {
    i32 size = 128 * (i32)ceil(pow(2, scale));
    i32 blockX = markerX * size / 256 + xCenter;
    i32 blockZ = markerZ * size / 256 + zCenter;
    return Pos2i(blockX, blockZ);
  }

private:
  std::unordered_map<i64, Map> fMaps;
};

} // namespace je2be::bedrock
