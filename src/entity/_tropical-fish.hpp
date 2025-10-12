#pragma once

#include "_optional.hpp"
#include "_reversible-map.hpp"
#include "entity/_entity-attributes.hpp"
#include "enums/_color-code-java.hpp"
#include "java/_entity.hpp"

namespace je2be {

class TropicalFish {
  static ReversibleMap<std::u8string, u16> const *CreateTable() {
    /*
    1.21.4                                                             | 1.21.5
    -------------------------------------------------------------------|-------------------------------------
    variant    variant(hex)  small  pattern  body_color  pattern_color | base_color  pattern    pattern_color
    -------------------------------------------------------------------|-------------------------------------
    459008     0x00070100    true   0x01     0x07        0x00            gray        sunstreak  white
    218234880  0x0d020000    true   0x00     0x02        0x0d            magenta     kob        green
    65536      0x00010000    true   0x00     0x00        0x01            orange      kob        white
    917504     0x000e0000    true   0x00     0x00        0x0e            red         kob        white
    117899265  0x07070001    false  0x00     0x07        0x07            gray        flopper    gray
    118161664  0x070b0100    true   0x01     0x0b        0x07            blue        sunstreak  gray
    235340288  0x0e070200    true   0x02     0x07        0x0e            gray        snooper    red
    101253888  0x06090300    true   0x03     0x09        0x06            cyan        dasher     pink
    100794624  0x06020100    true   0x01     0x02        0x06            magenta     sunstreak  pink
    67764993   0x040A0301    false  0x03     0x0a        0x04            purple      blockfish  yellow
    918529     0x000E0401    false  0x04     0x0e        0x00            red         betty      white
    50660352   0x03050400    true   0x04     0x05        0x03            lime        brinely    light_blue
    117441793  0x07000501    false  0x05     0x00        0x07            white       clayfish   gray
    67110144   0x04000500    true   0x05     0x00        0x04            white       spotty     yellow
    117506305  0x07010101    false  0x01     0x01        0x07            orange      stripey    gray
    17498625   0x010B0201    false  0x02     0x0b        0x01            blue        glitter    orange
    */
    return new ReversibleMap<std::u8string, u16>({
        {u8"kob", 0x0000},
        {u8"sunstreak", 0x0100},
        {u8"snooper", 0x0200},
        {u8"dasher", 0x0300},
        {u8"brinely", 0x0400},
        {u8"spotty", 0x0500},
        {u8"flopper", 0x0001},
        {u8"stripey", 0x0101},
        {u8"glitter", 0x0201},
        {u8"blockfish", 0x0301},
        {u8"betty", 0x0401},
        {u8"clayfish", 0x0501},
    });
  }

  static ReversibleMap<std::u8string, u16> const &GetTable() {
    using namespace std;
    static unique_ptr<ReversibleMap<u8string, u16> const> sTable(CreateTable());
    return *sTable;
  }

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

  struct JavaComponents {
    std::u8string fBaseColor;
    std::u8string fPattern;
    std::u8string fPatternColor;
  };

  static TropicalFish FromJavaComponents(JavaComponents const &components) {
    using namespace std;
    auto bodyColor = ColorCodeJavaFromJavaName(components.fBaseColor);
    auto patternColor = ColorCodeJavaFromJavaName(components.fPatternColor);
    auto const &table = GetTable();
    u16 pattern = Wrap<u16>(table.forward(components.fPattern), 0);
    bool small = (pattern & 0xf) == 0x0;
    TropicalFish tf(small, pattern >> 8, (i8)bodyColor, (i8)patternColor);
    return tf;
  }

  static TropicalFish FromBedrockBucketTag(CompoundTag const &tag) {
    bool small = tag.int32(u8"Variant", 0) == 0;
    i32 pattern = tag.int32(u8"MarkVariant", 0);
    i8 bodyColor = tag.byte(u8"Color", 0);
    i8 patternColor = tag.byte(u8"Color2", 0);
    return TropicalFish(small, pattern, bodyColor, patternColor);
  }

  // 25w03a or later
  JavaComponents toJavaComponents() const {
    JavaComponents jc;
    u16 v = u16((0xf & fPattern) << 8) | u16(fSmall ? 0x0 : 0x1);
    auto const &table = GetTable();
    auto pattern = Wrap<std::u8string>(table.backward(v), u8"kob");
    jc.fPattern = pattern;
    jc.fBaseColor = JavaNameFromColorCodeJava((ColorCodeJava)fBodyColor);
    jc.fPatternColor = JavaNameFromColorCodeJava((ColorCodeJava)fPatternColor);
    return jc;
  }

  // 25w02a or older
  i32 toJavaVariant() const {
    u32 u = 0;
    u |= fSmall ? 0 : 1;
    u |= (0xf & (u32)fPattern) << 8;
    u |= (0xf & (u32)fBodyColor) << 16;
    u |= (0xf & (u32)fPatternColor) << 24;
    return *(i32 *)&u;
  }

  CompoundTagPtr toBedrockBucketTag(i64 entityId, std::optional<float> health) const {
    using namespace std;
    java::Entity::Rep rep(entityId);
    auto ret = rep.toCompoundTag();
    auto attributes = EntityAttributes::Mob(u8"minecraft:tropical_fish", health);
    if (attributes) {
      ret->set(u8"Attributes", attributes->toBedrockListTag());
    }
    auto props = java::Entity::TropicalFishProperties(toJavaVariant());
    for (auto const &it : *props) {
      ret->set(it.first, it.second);
    }
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
