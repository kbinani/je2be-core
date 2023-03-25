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
    bool small = tag.int32(u8"Variant", 0) == 0;
    i32 pattern = tag.int32(u8"MarkVariant", 0);
    i8 bodyColor = tag.byte(u8"Color", 0);
    i8 patternColor = tag.byte(u8"Color2", 0);
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
    auto attributes = EntityAttributes::Mob(u8"minecraft:tropical_fish", nullopt);
    if (attributes) {
      ret->set(u8"Attributes", attributes->toBedrockListTag());
    }
    ret->set(u8"Variant", Int(fSmall ? 0 : 1));
    ret->set(u8"MarkVariant", Int(fPattern));
    ret->set(u8"Color", Byte(fBodyColor));
    ret->set(u8"Color2", Byte(fPatternColor));
    ret->set(u8"EntityType", Int(9071));
    u8string bodyIdPrefix;
    if (fSmall) {
      static vector<u8string> const m = {u8"item.tropicalBodyFlopper", u8"item.tropicalBodyStripey", u8"item.tropicalBodyGlitter", u8"item.tropicalBodyBlockfish", u8"item.tropicalBodyBetty", u8"item.tropicalBodyClayfish"};
      bodyIdPrefix = m[std::clamp(fPattern, 0, 5)];
    } else {
      static vector<u8string> const m = {u8"item.tropicalBodyKob", u8"item.tropicalBodySunstreak", u8"item.tropicalBodySnooper", u8"item.tropicalBodyDasher", u8"item.tropicalBodyBrinely", u8"item.tropicalBodySpotty"};
      bodyIdPrefix = m[std::clamp(fPattern, 0, 5)];
    }

    static vector<u8string> const colorIdMap = {u8"item.tropicalColorWhite.name", u8"item.tropicalColorOrange.name", u8"item.tropicalColorMagenta.name", u8"item.tropicalColorSky.name", u8"item.tropicalColorYellow.name", u8"item.tropicalColorLime.name", u8"item.tropicalColorRose.name", u8"item.tropicalColorGray.name", u8"item.tropicalColorSilver.name", u8"item.tropicalColorTeal.name", u8"item.tropicalColorPlum.name", u8"item.tropicalColorBlue.name", u8"item.tropicalColorBrown.name", u8"item.tropicalColorGreen.name", u8"item.tropicalColorRed.name"};
    u8string colorId = colorIdMap[std::clamp((int)fBodyColor, 0, 14)];

    u8string color2Id = colorIdMap[std::clamp((int)fPatternColor, 0, 14)];
    u8string bodyId;
    if (colorId == color2Id) {
      bodyId = bodyIdPrefix + u8"Single.name";
      color2Id = u8"";
    } else {
      bodyId = bodyIdPrefix + u8"Multi.name";
    }
    ret->set(u8"BodyID", String(bodyId));
    ret->set(u8"ColorID", String(colorId));
    if (!color2Id.empty()) {
      ret->set(u8"Color2ID", String(color2Id));
    }
    ret->set(u8"AppendCustomName", Bool(true));
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
