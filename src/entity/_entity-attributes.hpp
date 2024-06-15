#pragma once

namespace je2be {

class EntityAttributes {
  using AttributesData = ListTagPtr;
  using Provider = std::function<AttributesData(void)>;

public:
  struct Attribute {
    float base;
    float current;
    float max;

    Attribute(float base, float current, float max = std::numeric_limits<float>::max()) : base(base), current(current), max(max) {}

    void updateCurrent(float v) {
      current = std::min(v, max);
    }

    CompoundTagPtr toBedrockCompoundTag(std::u8string const &name) const {
      auto a = Compound();
      a->set(u8"Base", Float(base));
      a->set(u8"Current", Float(current));
      a->set(u8"Max", Float(max));
      a->set(u8"Name", u8"minecraft:" + name);
      a->set(u8"DefaultMax", Float(max));
      a->set(u8"DefaultMin", Float(0));
      a->set(u8"Min", Float(0));
      return a;
    }
  };

  struct Attributes {
    Attribute luck;
    Attribute health;
    Attribute absorption;
    Attribute knockback_resistance;
    Attribute movement;
    Attribute underwater_movement;
    Attribute lava_movement;
    Attribute follow_range;
    std::optional<Attribute> attack_damage;

    Attributes(Attribute health, Attribute knockback_resistance, Attribute movement, Attribute underwater_movement, Attribute lava_movement, Attribute follow_range, std::optional<Attribute> attack_damage) : luck(0, 0, 1024), health(health), absorption(0, 0, 16), knockback_resistance(knockback_resistance), movement(movement), underwater_movement(underwater_movement), lava_movement(lava_movement), follow_range(follow_range), attack_damage(attack_damage) {}

    ListTagPtr toBedrockListTag() const {
      auto list = List<Tag::Type::Compound>();
      list->push_back(luck.toBedrockCompoundTag(u8"luck"));
      list->push_back(health.toBedrockCompoundTag(u8"health"));
      list->push_back(absorption.toBedrockCompoundTag(u8"absorption"));
      list->push_back(knockback_resistance.toBedrockCompoundTag(u8"knockback_resistance"));
      list->push_back(movement.toBedrockCompoundTag(u8"movement"));
      list->push_back(underwater_movement.toBedrockCompoundTag(u8"underwater_movement"));
      list->push_back(lava_movement.toBedrockCompoundTag(u8"lava_movement"));
      list->push_back(follow_range.toBedrockCompoundTag(u8"follow_range"));
      if (attack_damage) {
        list->push_back(attack_damage->toBedrockCompoundTag(u8"attack_damage"));
      }
      return list;
    }
  };

  static std::optional<Attributes> Mob(std::u8string const &name, std::optional<float> health) {
    static std::unique_ptr<std::unordered_map<std::u8string, Attributes> const> const table(CreateTable());
    auto found = table->find(name);
    if (found == table->end()) {
      return std::nullopt;
    }
    Attributes attrs = found->second;
    if (health) {
      attrs.health.updateCurrent(*health);
    }
    return attrs;
  }

  static ListTagPtr AnyHorseFromJava(CompoundTag const &tag, std::optional<float> currentHealth) {
    using namespace std;

    auto attributes = FallbackPtr<ListTag>(tag, {u8"attributes", u8"Attributes"});
    Attribute health(15, 15, 15);
    Attribute movement(0.1125, 0.1125);
    Attribute jumpStrength(0.4, 0.4);
    if (attributes) {
      for (auto const &it : *attributes) {
        auto attrs = it->asCompound();
        if (!attrs) {
          continue;
        }
        auto idPtr = FallbackPtr<StringTag>(*attrs, {u8"id", u8"Name"});
        auto basePtr = FallbackPtr<DoubleTag>(*attrs, {u8"base", u8"Base"});
        if (!idPtr || !basePtr) {
          continue;
        }
        u8string id = idPtr->fValue;
        double value = basePtr->fValue;
        if (id == u8"minecraft:generic.max_health") {
          health = Attribute(value, value, value);
        } else if (id == u8"minecraft:generic.movement_speed") {
          movement = Attribute(value, value);
        } else if (id == u8"minecraft:horse.jump_strength") {
          jumpStrength = Attribute(value, value);
        }
      }
    }

    Attribute luck(0, -1024, 1024);
    Attribute followRange(16, 16, 2048);
    Attribute absorption(0, 0, 16);

    if (currentHealth) {
      health.updateCurrent(*currentHealth);
    }

    auto ret = List<Tag::Type::Compound>();
    ret->push_back(luck.toBedrockCompoundTag(u8"luck"));
    ret->push_back(health.toBedrockCompoundTag(u8"health"));
    ret->push_back(movement.toBedrockCompoundTag(u8"movement"));
    ret->push_back(followRange.toBedrockCompoundTag(u8"follow_range"));
    ret->push_back(absorption.toBedrockCompoundTag(u8"absorption"));
    ret->push_back(jumpStrength.toBedrockCompoundTag(u8"horse.jump_strength"));
    return ret;
  }

  static Attributes Slime(int sizeB, std::optional<float> currentHealth) {
    float sizeScale = std::max(1.0f, (float)sizeB);
    float strengthScale = sizeScale * sizeScale;
    float logScale = log2f(sizeScale);

    float health = strengthScale;
    float movement = 0.3f + 0.1f * logScale;
    float attackDamage = sizeScale;
    /*
    sizeB  health movement attack_damage
    1      1      0.3      0
    2      4      0.4      2
    4      16     0.6      4
    */
    Attributes attrs(Attribute(health, health, health),                    // health
                     Attribute(0, 0, 1),                                   // knockback_resistance
                     Attribute(movement, movement),                        // movement
                     Attribute(0.02, 0.02),                                // underwater_movement
                     Attribute(0.02, 0.02),                                // lava_movement
                     Attribute(16, 16, 2048),                              // follow_range
                     Attribute(attackDamage, attackDamage, attackDamage)); // attack_damage
    if (currentHealth) {
      attrs.health.updateCurrent(*currentHealth);
    }
    return attrs;
  }

  static Attributes Wolf(bool tamed, std::optional<float> currentHealth) {
    Attributes wildAttrs(Attribute(8, 8, 8),       // health
                         Attribute(0, 0, 1),       // knockback_resistance
                         Attribute(0.3, 0.3),      // movement
                         Attribute(0.02, 0.02),    // underwater_movement
                         Attribute(0.02, 0.02),    // lava_movement
                         Attribute(16, 16, 2048),  // follow_range
                         Attribute(3, 3, 3));      // attack_damage
    Attributes tamedAttrs(Attribute(40, 40, 40),   // health
                          Attribute(0, 0, 1),      // knockback_resistance
                          Attribute(0.3, 0.3),     // movement
                          Attribute(0.02, 0.02),   // underwater_movement
                          Attribute(0.02, 0.02),   // lava_movement
                          Attribute(16, 16, 2048), // follow_range
                          Attribute(4, 4, 4));     // attack_damage
    Attributes attrs = tamed ? tamedAttrs : wildAttrs;
    if (currentHealth) {
      attrs.health.updateCurrent(*currentHealth);
    }
    return attrs;
  }

  static Attributes Player(std::optional<float> currentHealth) {
    Attributes attrs(Attribute(20, 20, 20),   // health
                     Attribute(0, 0, 16),     // knockback_resistance
                     Attribute(0.1, 0.1),     // movement
                     Attribute(0.02, 0.02),   // underwater_movement
                     Attribute(0.02, 0.02),   // lava_movement
                     Attribute(16, 16, 2048), // follow_range
                     Attribute(1, 1, 1));     // attack_damage
    if (currentHealth) {
      attrs.health.updateCurrent(*currentHealth);
    }
    return attrs;
  }

  static Attributes Wither(int difficultyBedrock, std::optional<float> currentHealth) {
    float healthB = 300;
    switch (difficultyBedrock) {
    case 3: // hard
      healthB = 600;
      break;
    case 2: // normal
      healthB = 450;
      break;
    }
    Attributes attrs(
        Attribute(1, healthB, healthB), // health(base, current, max)
        Attribute(0, 0, 1),             // knockback_resistance
        Attribute(0.6, 0.6),            // movement
        Attribute(0.02, 0.02),          // underwater_movement
        Attribute(0.02, 0.02),          // lava_movement
        Attribute(70, 70, 2048),        // follow_range
        std::nullopt);                  // attack_damage
    if (currentHealth) {
      float ratio = *currentHealth / 300.0f;
      attrs.health.updateCurrent(healthB * ratio);
    }
    return attrs;
  }

private:
  static std::unordered_map<std::u8string, Attributes> *CreateTable() {
    using namespace std;
    auto table = new unordered_map<u8string, Attributes>();
    table->insert(make_pair(u8"minecraft:armor_stand", Attributes(
                                                           Attribute(6, 6, 6),      // health
                                                           Attribute(1, 1, 1),      // knockback_resistance
                                                           Attribute(0.7, 0.7),     // movement
                                                           Attribute(0.2, 0.2),     // underwater_movement
                                                           Attribute(0.02, 0.02),   // lava_movement
                                                           Attribute(16, 16, 2048), // follow_range
                                                           nullopt)));              // attack_damage
    table->insert(make_pair(u8"minecraft:bat", Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1), Attribute(0.1, 0.1), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:bee", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(1024, 1024, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:blaze", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.23, 0.23), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(48, 48, 48), Attribute(6, 6, 6))));
    table->insert(make_pair(u8"minecraft:cat", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(4, 4))));
    table->insert(make_pair(u8"minecraft:cave_spider", Attributes(Attribute(12, 12, 12), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:chicken", Attributes(Attribute(4, 4, 4), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:cod", Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1), Attribute(0.1, 0.1), Attribute(0.1, 0.1), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:cow", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:creeper", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.2, 0.2), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair(u8"minecraft:dolphin", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.1, 0.1), Attribute(0.15, 0.15), Attribute(0.02, 0.02), Attribute(48, 48, 48), Attribute(3, 3, 3))));
    table->insert(make_pair(u8"minecraft:drowned", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.23, 0.23), Attribute(0.06, 0.06), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:elder_guardian", Attributes(Attribute(80, 80, 80), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(16, 16, 16), Attribute(5, 5, 5))));
    table->insert(make_pair(u8"minecraft:ender_dragon", Attributes(Attribute(200, 200, 200), Attribute(100, 1, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair(u8"minecraft:enderman", Attributes(Attribute(40, 40, 40), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(32, 32, 32), Attribute(7, 7, 7))));
    table->insert(make_pair(u8"minecraft:endermite", Attributes(Attribute(8, 8, 8), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(2, 2, 2))));
    table->insert(make_pair(u8"minecraft:evoker", Attributes(Attribute(24, 24, 24), Attribute(0, 0, 1), Attribute(0.5, 0.5), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:fox", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(2, 2, 2))));
    table->insert(make_pair(u8"minecraft:ghast", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.03, 0.03), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 64), nullopt)));
    table->insert(make_pair(u8"minecraft:guardian", Attributes(Attribute(30, 30, 30), Attribute(0, 0, 1), Attribute(0.12, 0.12), Attribute(0.12, 0.12), Attribute(0.02, 0.02), Attribute(16, 16, 16), Attribute(5, 5, 5))));
    table->insert(make_pair(u8"minecraft:hoglin", Attributes(Attribute(40, 40, 40), Attribute(0.5, 0.5, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair(u8"minecraft:husk", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.23, 0.23), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair(u8"minecraft:iron_golem", Attributes(Attribute(100, 100, 100), Attribute(1, 1, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), Attribute(7, 7, 7))));
    Attributes llama(Attribute(27, 27, 27), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(40, 40, 40), nullopt);
    table->insert(make_pair(u8"minecraft:llama", llama));
    table->insert(make_pair(u8"minecraft:trader_llama", llama));
    table->insert(make_pair(u8"minecraft:magma_cube", Attributes(Attribute(16, 16, 16), Attribute(0, 0, 1), Attribute(0.75, 0.75), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(6, 6, 6))));
    table->insert(make_pair(u8"minecraft:mooshroom", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:ocelot", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(4, 4))));
    table->insert(make_pair(u8"minecraft:panda", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.15, 0.15), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(2, 2, 2))));
    table->insert(make_pair(u8"minecraft:parrot", Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1), Attribute(0.4, 0.4), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:phantom", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(1.8, 1.8), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 64), Attribute(6, 6, 6))));
    table->insert(make_pair(u8"minecraft:pig", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:piglin", Attributes(Attribute(16, 16, 16), Attribute(0, 0, 1), Attribute(0.35, 0.35), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), Attribute(5, 5, 5))));
    table->insert(make_pair(u8"minecraft:piglin_brute", Attributes(Attribute(50, 50, 50), Attribute(0, 0, 1), Attribute(0.35, 0.35), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), Attribute(7, 7, 7))));
    table->insert(make_pair(u8"minecraft:pillager", Attributes(Attribute(24, 24, 24), Attribute(0, 0, 1), Attribute(0.35, 0.35), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:polar_bear", Attributes(Attribute(30, 30, 30), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(48, 48, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:pufferfish", Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1), Attribute(0.13, 0.13), Attribute(0.13, 0.13), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:rabbit", Attributes(Attribute(3, 3, 3), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:ravager", Attributes(Attribute(100, 100, 100), Attribute(0.5, 0.5, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), Attribute(12, 12, 12))));
    table->insert(make_pair(u8"minecraft:salmon", Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1), Attribute(0.12, 0.12), Attribute(0.12, 0.12), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:sheep", Attributes(Attribute(8, 8, 8), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:shulker", Attributes(Attribute(30, 30, 30), Attribute(0, 0, 1), Attribute(0, 0, 0), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:silverfish", Attributes(Attribute(8, 8, 8), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(1, 1, 1))));
    table->insert(make_pair(u8"minecraft:skeleton", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:snow_golem", Attributes(Attribute(4, 4, 4), Attribute(0, 0, 1), Attribute(0.2, 0.2), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(2, 2, 2))));
    table->insert(make_pair(u8"minecraft:spider", Attributes(Attribute(16, 16, 16), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    Attributes squid(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.2, 0.2), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt);
    table->insert(make_pair(u8"minecraft:squid", squid));
    table->insert(make_pair(u8"minecraft:stray", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:strider", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.16, 0.16), Attribute(0.02, 0.02), Attribute(0.32, 0.32), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:tropical_fish", Attributes(Attribute(3, 3, 3),      // health
                                                                    Attribute(0, 0, 1),      //knockback_resistance
                                                                    Attribute(0.12, 0.12),   // movement
                                                                    Attribute(0.12, 0.12),   // underwater_movement
                                                                    Attribute(0.02, 0.02),   //lava_movement
                                                                    Attribute(16, 16, 2048), //follow_range
                                                                    nullopt)));              //attack_damage
    table->insert(make_pair(u8"minecraft:turtle", Attributes(Attribute(30, 30, 30), Attribute(0, 0, 1), Attribute(0.1, 0.1), Attribute(0.12, 0.12), Attribute(0.02, 0.02), Attribute(1024, 1024, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:vex", Attributes(Attribute(14, 14, 14), Attribute(0, 0, 1), Attribute(1, 1), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair(u8"minecraft:villager", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.5, 0.5), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(128, 128, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:vindicator", Attributes(Attribute(24, 24, 24), Attribute(0, 0, 1), Attribute(0.35, 0.35), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), Attribute(8, 8, 8))));
    table->insert(make_pair(u8"minecraft:wandering_trader", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.5, 0.5), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:witch", Attributes(Attribute(26, 26, 26), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), nullopt)));
    table->insert(make_pair(u8"minecraft:wither_skeleton", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(4, 4, 4))));
    table->insert(make_pair(u8"minecraft:zoglin", Attributes(Attribute(40, 40, 40), Attribute(0.5, 0.5, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair(u8"minecraft:zombie", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.23, 0.23), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair(u8"minecraft:zombified_piglin", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.23, 0.23), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(5, 5, 5))));
    table->insert(make_pair(u8"minecraft:zombie_villager", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.23, 0.23), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));

    // 1.17
    table->insert(make_pair(u8"minecraft:glow_squid", squid));
    table->insert(make_pair(u8"minecraft:axolotl", Attributes(
                                                       Attribute(14, 14, 14),   // health
                                                       Attribute(0, 0, 1),      // knockback_resistance
                                                       Attribute(0.1, 0.1),     // movement
                                                       Attribute(0.2, 0.2),     // underwater_movement
                                                       Attribute(0.02, 0.02),   // lava_movement
                                                       Attribute(16, 16, 2048), // follow_range
                                                       Attribute(2, 2, 2))));   // attack_damage
    table->insert(make_pair(u8"minecraft:goat", Attributes(
                                                    Attribute(10, 10, 10),   // health
                                                    Attribute(0, 0, 1),      // knockback_resistance
                                                    Attribute(0.4, 0.4),     // movement
                                                    Attribute(0.02, 0.02),   // underwater_movement
                                                    Attribute(0.02, 0.02),   // lava_movement
                                                    Attribute(16, 16, 2048), // follow_range
                                                    Attribute(2, 2, 2))));   // attack_damage

    // 1.19
    table->insert(make_pair(u8"minecraft:frog", Attributes(
                                                    Attribute(10, 10, 10),   // health
                                                    Attribute(0, 0, 1),      // knockback_resistance
                                                    Attribute(0.1, 0.1),     // movement
                                                    Attribute(0.15, 0.15),   // underwater_movement
                                                    Attribute(0.02, 0.02),   // lava_movement
                                                    Attribute(16, 16, 2048), // follow_range
                                                    nullopt)));              // attack_damage
    table->insert(make_pair(u8"minecraft:warden", Attributes(
                                                      Attribute(500, 500, 500),    // health
                                                      Attribute(1, 1, 1),          // knockback_resistance
                                                      Attribute(0.3, 0.3),         // movement
                                                      Attribute(0.02, 0.02),       // underwater_movement
                                                      Attribute(0.02, 0.02),       // lava_movement
                                                      Attribute(2048, 2048, 2048), // follow_range
                                                      Attribute(30, 30, 30))));    // attack_damage
    table->insert(make_pair(u8"minecraft:allay", Attributes(
                                                     Attribute(20, 20, 20),       // health(base, current, max)
                                                     Attribute(0, 0, 1),          // knockback_resistance
                                                     Attribute(0.1, 0.1),         // movement
                                                     Attribute(0.02, 0.02),       // underwater_movement
                                                     Attribute(0.02, 0.02),       // lava_movement
                                                     Attribute(1024, 1024, 2048), // follow_range
                                                     nullopt)));                  // attack_damage
    table->insert(make_pair(u8"minecraft:tadpole", Attributes(
                                                       Attribute(6, 6, 6),      // health(base, current, max)
                                                       Attribute(0, 0, 1),      // knockback_resistance
                                                       Attribute(0.1, 0.1),     // movement
                                                       Attribute(0.02, 0.02),   // underwater_movement
                                                       Attribute(0.02, 0.02),   // lava_movement
                                                       Attribute(16, 16, 2048), // follow_range
                                                       nullopt)));              // attack_damage
    table->insert(make_pair(u8"minecraft:camel", Attributes(
                                                     Attribute(32, 32, 32),   // health(base, current, max)
                                                     Attribute(0, 0, 1),      // knockback_resistance
                                                     Attribute(0.01, 0.09),   // movement
                                                     Attribute(0.02, 0.02),   // underwater_movement
                                                     Attribute(0.02, 0.02),   // lava_movement
                                                     Attribute(16, 16, 2048), // follow_range
                                                     nullopt)));              // attack_damage

    // 1.20
    table->insert(make_pair(u8"minecraft:sniffer", Attributes(
                                                       Attribute(14, 14, 14),   // health(base, current, max)
                                                       Attribute(0, 0, 1),      // knockback_resistance
                                                       Attribute(0.09, 0.09),   // movement
                                                       Attribute(0.02, 0.02),   // underwater_movement
                                                       Attribute(0.02, 0.02),   // lava_movement
                                                       Attribute(64, 64, 2048), // follow_range
                                                       nullopt)));              // attack_damage

    // 1.21
    table->insert(make_pair(u8"minecraft:armadillo", Attributes(
                                                         Attribute(12, 12, 12),   // health(base, current, max)
                                                         Attribute(0, 0, 1),      // knockback_resistance
                                                         Attribute(0.14, 0.14),   // movement
                                                         Attribute(0.02, 0.02),   // underwater_movement
                                                         Attribute(0.02, 0.02),   // lava_movement
                                                         Attribute(16, 16, 2048), // follow_range
                                                         nullopt)));              // attack_damage
    table->insert(make_pair(u8"minecraft:bogged", Attributes(
                                                      Attribute(16, 16, 16),   // health(base, current, max)
                                                      Attribute(0, 0, 1),      // knockback_resistance
                                                      Attribute(0.25, 0.25),   // movement
                                                      Attribute(0.02, 0.02),   // underwater_movement
                                                      Attribute(0.02, 0.02),   // lava_movement
                                                      Attribute(16, 16, 2048), // follow_range
                                                      nullopt)));              // attack_damage
    table->insert(make_pair(u8"minecraft:breeze", Attributes(
                                                      Attribute(30, 30, 30),   // health(base, current, max)
                                                      Attribute(0, 0, 1),      // knockback_resistance
                                                      Attribute(0.4, 0.4),     // movement
                                                      Attribute(0.02, 0.02),   // underwater_movement
                                                      Attribute(0.02, 0.02),   // lava_movement
                                                      Attribute(32, 32, 2048), // follow_range
                                                      nullopt)));              // attack_damage

    return table;
  }
};

} // namespace je2be
