#pragma once

#include "entity/_entity-attributes.hpp"

namespace je2be {

class TropicalFish {
public:
  static TropicalFish FromJavaVariant(i32 variant) {
    u32 u = *(u32 *)&variant;
    bool small = (0xf & u) == 0;
    i32 pattern = 0xf & (u >> 8);
    i32 bodyColor = 0xf & (u >> 16);
    i32 patternColor = 0xf & (u >> 24);
    TropicalFish tf(small, pattern, (i8)bodyColor, (i8)patternColor);
    return tf;
  }

  static TropicalFish FromBedrockBucketTag(CompoundTag const &tag) {
    bool small = tag.int32("Variant", 0) == 0;
    i32 pattern = tag.int32("MarkVariant", 0);
    i8 bodyColor = tag.byte("Color", 0);
    i8 patternColor = tag.byte("Color2", 0);
    return TropicalFish(small, pattern, bodyColor, patternColor);
  }

  i32 toJavaVariant() const {
    u32 u = 0;
    u |= fSmall ? 0 : 1;
    u |= (0xf & (u32)fPattern) << 8;
    u |= (0xf & (u32)fBodyColor) << 16;
    u |= (0xf & (u32)fPatternColor) << 24;
    return *(i32 *)&u;
  }

  CompoundTagPtr toBedrockBucketTag() const {
    using namespace std;
    auto ret = Compound();
    auto attributes = EntityAttributes::Mob("minecraft:tropical_fish", nullopt);
    if (attributes) {
      ret->set("Attributes", attributes->toBedrockListTag());
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
  TropicalFish(bool small, i32 pattern, i8 bodyColor, i8 patternColor) : fSmall(small), fPattern(pattern), fBodyColor(bodyColor), fPatternColor(patternColor) {}

public:
  bool fSmall;
  i32 fPattern;
  i8 fBodyColor;
  i8 fPatternColor;
};

} // namespace je2be
