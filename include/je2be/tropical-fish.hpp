#pragma once

namespace j2b {

class TropicalFish {
public:
  static TropicalFish FromVariant(int32_t variant) {
    bool small = (0xf & variant) == 0;
    int32_t pattern = 0xf & (variant >> 8);
    int32_t bodyColor = 0xf & (variant >> 16);
    int32_t patternColor = 0xf & (variant >> 24);
    TropicalFish tf(small, pattern, (int8_t)bodyColor, (int8_t)patternColor);
    return tf;
  }

  std::shared_ptr<mcfile::nbt::CompoundTag> toBucketTag() const {
    using namespace props;
    using namespace std;
    auto ret = make_shared<mcfile::nbt::CompoundTag>();
    auto attributes = EntityAttributes::Mob("minecraft:tropical_fish");
    if (attributes) {
      ret->set("Attributes", attributes->toListTag());
    }
    ret->set("Variant", Int(fSmall ? 0 : 1));
    ret->set("MarkVariant", Int(fPattern));
    ret->set("Color", Byte(fBodyColor));
    ret->set("Color2", Byte(fPatternColor));
    ret->set("EntityType", Int(9071));
    string bodyIdPrefix;
    if (fSmall) {
      static vector<string> const m = {"item.tropicalBodyFlopper", "item.tropicalBodyStripey", "item.tropicalBodyGlitter", "item.tropicalBodyBlockfish", "item.tropicalBodyBetty", "item.tropicalBodyClayfish"};
      bodyIdPrefix = m[std::clamp(fPattern, 0, 5)];
    } else {
      static vector<string> const m = {"item.tropicalBodyKob", "item.tropicalBodySunstreak", "item.tropicalBodySnooper", "item.tropicalBodyDasher", "item.tropicalBodyBrinely", "item.tropicalBodySpotty"};
      bodyIdPrefix = m[std::clamp(fPattern, 0, 5)];
    }

    static vector<string> const colorIdMap = {"item.tropicalColorWhite.name", "item.tropicalColorOrange.name", "item.tropicalColorMagenta.name", "item.tropicalColorSky.name", "item.tropicalColorYellow.name", "item.tropicalColorLime.name", "item.tropicalColorRose.name", "item.tropicalColorGray.name", "item.tropicalColorSilver.name", "item.tropicalColorTeal.name", "item.tropicalColorPlum.name", "item.tropicalColorBlue.name", "item.tropicalColorBrown.name", "item.tropicalColorGreen.name", "item.tropicalColorRed.name"};
    string colorId = colorIdMap[std::clamp((int)fBodyColor, 0, 14)];

    string color2Id = colorIdMap[std::clamp((int)fPatternColor, 0, 14)];
    string bodyId;
    if (colorId == color2Id) {
      bodyId = bodyIdPrefix + "Single.name";
      color2Id = "";
    } else {
      bodyId = bodyIdPrefix + "Multi.name";
    }
    ret->set("BodyID", String(bodyId));
    ret->set("ColorID", String(colorId));
    if (!color2Id.empty()) {
      ret->set("Color2ID", String(color2Id));
    }
    ret->set("AppendCustomName", Bool(true));
    return ret;
  }

private:
  TropicalFish(bool small, int32_t pattern, int8_t bodyColor, int8_t patternColor) : fSmall(small), fPattern(pattern), fBodyColor(bodyColor), fPatternColor(patternColor) {}

public:
  bool fSmall;
  int32_t fPattern;
  int8_t fBodyColor;
  int8_t fPatternColor;
};

} // namespace j2b
