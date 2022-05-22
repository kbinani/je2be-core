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

    CompoundTagPtr toCompoundTag(std::string const &name) const {
      auto a = Compound();
      a->set("Base", Float(base));
      a->set("Current", Float(current));
      a->set("Max", Float(max));
      a->set("Name", String("minecraft:" + name));
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

    ListTagPtr toListTag() const {
      auto list = List<Tag::Type::Compound>();
      list->push_back(luck.toCompoundTag("luck"));
      list->push_back(health.toCompoundTag("health"));
      list->push_back(absorption.toCompoundTag("absorption"));
      list->push_back(knockback_resistance.toCompoundTag("knockback_resistance"));
      list->push_back(movement.toCompoundTag("movement"));
      list->push_back(underwater_movement.toCompoundTag("underwater_movement"));
      list->push_back(lava_movement.toCompoundTag("lava_movement"));
      list->push_back(follow_range.toCompoundTag("follow_range"));
      if (attack_damage) {
        list->push_back(attack_damage->toCompoundTag("attack_damage"));
      }
      return list;
    }
  };

  static std::optional<Attributes> Mob(std::string const &name, std::optional<float> health) {
    static std::unique_ptr<std::unordered_map<std::string, Attributes> const> const table(CreateTable());
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

  static ListTagPtr AnyHorse(CompoundTag const &tag, std::optional<float> currentHealth) {
    using namespace std;

    auto attributes = tag.listTag("Attributes");
    Attribute health(15, 15, 15);
    Attribute movement(0.1125, 0.1125);
    Attribute jumpStrength(0.4, 0.4);
    if (attributes) {
      for (auto const &it : *attributes) {
        auto attrs = it->asCompound();
        if (!attrs) {
          continue;
        }
        auto name = attrs->string("Name");
        auto value = attrs->float64("Base");
        if (!name || !value) {
          continue;
        }
        if (*name == "minecraft:generic.max_health") {
          health = Attribute(*value, *value, *value);
        } else if (*name == "minecraft:generic.movement_speed") {
          movement = Attribute(*value, *value);
        } else if (*name == "minecraft:horse.jump_strength") {
          jumpStrength = Attribute(*value, *value);
        }
      }
    }

    Attribute luck(0, 0, 1024);
    Attribute followRange(16, 16, 2048);
    Attribute absorption(0, 0, 16);

    if (currentHealth) {
      health.updateCurrent(*currentHealth);
    }

    auto ret = List<Tag::Type::Compound>();
    ret->push_back(luck.toCompoundTag("luck"));
    ret->push_back(health.toCompoundTag("health"));
    ret->push_back(movement.toCompoundTag("movement"));
    ret->push_back(followRange.toCompoundTag("follow_range"));
    ret->push_back(absorption.toCompoundTag("absorption"));
    ret->push_back(jumpStrength.toCompoundTag("jump_strength"));
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
    Attributes tamedAttrs(Attribute(20, 20, 20),   // health
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

private:
  static std::unordered_map<std::string, Attributes> *CreateTable() {
    using namespace std;
    auto table = new unordered_map<string, Attributes>();
    table->insert(make_pair("minecraft:armor_stand", Attributes(
                                                         Attribute(6, 6, 6),      // health
                                                         Attribute(1, 1, 1),      // knockback_resistance
                                                         Attribute(0.7, 0.7),     // movement
                                                         Attribute(0.2, 0.2),     // underwater_movement
                                                         Attribute(0.02, 0.02),   // lava_movement
                                                         Attribute(16, 16, 2048), // follow_range
                                                         nullopt)));              // attack_damage
    table->insert(make_pair("minecraft:bat", Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1), Attribute(0.1, 0.1), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:bee", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(1024, 1024, 2048), nullopt)));
    table->insert(make_pair("minecraft:blaze", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.23, 0.23), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(48, 48, 48), Attribute(6, 6, 6))));
    table->insert(make_pair("minecraft:cat", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(4, 4))));
    table->insert(make_pair("minecraft:cave_spider", Attributes(Attribute(12, 12, 12), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:chicken", Attributes(Attribute(4, 4, 4), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:cod", Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1), Attribute(0.1, 0.1), Attribute(0.1, 0.1), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:cow", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:creeper", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.2, 0.2), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair("minecraft:dolphin", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.1, 0.1), Attribute(0.15, 0.15), Attribute(0.02, 0.02), Attribute(48, 48, 48), Attribute(3, 3, 3))));
    table->insert(make_pair("minecraft:drowned", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.23, 0.23), Attribute(0.06, 0.06), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:elder_guardian", Attributes(Attribute(80, 80, 80), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(16, 16, 16), Attribute(5, 5, 5))));
    table->insert(make_pair("minecraft:ender_dragon", Attributes(Attribute(200, 200, 200), Attribute(100, 1, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair("minecraft:enderman", Attributes(Attribute(40, 40, 40), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(32, 32, 32), Attribute(7, 7, 7))));
    table->insert(make_pair("minecraft:endermite", Attributes(Attribute(8, 8, 8), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(2, 2, 2))));
    table->insert(make_pair("minecraft:evoker", Attributes(Attribute(24, 24, 24), Attribute(0, 0, 1), Attribute(0.5, 0.5), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), nullopt)));
    table->insert(make_pair("minecraft:fox", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(2, 2, 2))));
    table->insert(make_pair("minecraft:ghast", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.03, 0.03), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 64), nullopt)));
    table->insert(make_pair("minecraft:guardian", Attributes(Attribute(30, 30, 30), Attribute(0, 0, 1), Attribute(0.12, 0.12), Attribute(0.12, 0.12), Attribute(0.02, 0.02), Attribute(16, 16, 16), Attribute(5, 5, 5))));
    table->insert(make_pair("minecraft:hoglin", Attributes(Attribute(40, 40, 40), Attribute(0.5, 0.5, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair("minecraft:husk", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.23, 0.23), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair("minecraft:iron_golem", Attributes(Attribute(100, 100, 100), Attribute(1, 1, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), Attribute(7, 7, 7))));
    Attributes llama(Attribute(27, 27, 27), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(40, 40, 40), nullopt);
    table->insert(make_pair("minecraft:llama", llama));
    table->insert(make_pair("minecraft:trader_llama", llama));
    table->insert(make_pair("minecraft:magma_cube", Attributes(Attribute(16, 16, 16), Attribute(0, 0, 1), Attribute(0.75, 0.75), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(6, 6, 6))));
    table->insert(make_pair("minecraft:mooshroom", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:ocelot", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(4, 4))));
    table->insert(make_pair("minecraft:panda", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.15, 0.15), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(2, 2, 2))));
    table->insert(make_pair("minecraft:parrot", Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1), Attribute(0.4, 0.4), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:phantom", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(1.8, 1.8), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 64), Attribute(6, 6, 6))));
    table->insert(make_pair("minecraft:pig", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:piglin", Attributes(Attribute(16, 16, 16), Attribute(0, 0, 1), Attribute(0.35, 0.35), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), Attribute(5, 5, 5))));
    table->insert(make_pair("minecraft:piglin_brute", Attributes(Attribute(50, 50, 50), Attribute(0, 0, 1), Attribute(0.35, 0.35), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), Attribute(7, 7, 7))));
    table->insert(make_pair("minecraft:pillager", Attributes(Attribute(24, 24, 24), Attribute(0, 0, 1), Attribute(0.35, 0.35), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), nullopt)));
    table->insert(make_pair("minecraft:polar_bear", Attributes(Attribute(30, 30, 30), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(48, 48, 2048), nullopt)));
    table->insert(make_pair("minecraft:pufferfish", Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1), Attribute(0.13, 0.13), Attribute(0.13, 0.13), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:rabbit", Attributes(Attribute(3, 3, 3), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:ravager", Attributes(Attribute(100, 100, 100), Attribute(0.5, 0.5, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), Attribute(12, 12, 12))));
    table->insert(make_pair("minecraft:salmon", Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1), Attribute(0.12, 0.12), Attribute(0.12, 0.12), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:sheep", Attributes(Attribute(8, 8, 8), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:shulker", Attributes(Attribute(30, 30, 30), Attribute(0, 0, 1), Attribute(0, 0, 0), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:silverfish", Attributes(Attribute(8, 8, 8), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(1, 1, 1))));
    table->insert(make_pair("minecraft:skeleton", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:snow_golem", Attributes(Attribute(4, 4, 4), Attribute(0, 0, 1), Attribute(0.2, 0.2), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(2, 2, 2))));
    table->insert(make_pair("minecraft:spider", Attributes(Attribute(16, 16, 16), Attribute(0, 0, 1), Attribute(0.3, 0.3), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    Attributes squid(Attribute(10, 10, 10), Attribute(0, 0, 1), Attribute(0.2, 0.2), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt);
    table->insert(make_pair("minecraft:squid", squid));
    table->insert(make_pair("minecraft:stray", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:strider", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.16, 0.16), Attribute(0.02, 0.02), Attribute(0.32, 0.32), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:tropical_fish", Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1), Attribute(0.12, 0.12), Attribute(0.12, 0.12), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:turtle", Attributes(Attribute(30, 30, 30), Attribute(0, 0, 1), Attribute(0.1, 0.1), Attribute(0.12, 0.12), Attribute(0.02, 0.02), Attribute(1024, 1024, 2048), nullopt)));
    table->insert(make_pair("minecraft:vex", Attributes(Attribute(14, 14, 14), Attribute(0, 0, 1), Attribute(1, 1), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair("minecraft:villager", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.5, 0.5), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(128, 128, 2048), nullopt)));
    table->insert(make_pair("minecraft:vindicator", Attributes(Attribute(24, 24, 24), Attribute(0, 0, 1), Attribute(0.35, 0.35), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), Attribute(8, 8, 8))));
    table->insert(make_pair("minecraft:wandering_trader", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.5, 0.5), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:witch", Attributes(Attribute(26, 26, 26), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(64, 64, 2048), nullopt)));
    table->insert(make_pair("minecraft:wither", Attributes(Attribute(600, 600, 600), Attribute(0, 0, 1), Attribute(0.6, 0.6), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(70, 70, 2048), nullopt)));
    table->insert(make_pair("minecraft:wither_skeleton", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(4, 4, 4))));
    table->insert(make_pair("minecraft:zoglin", Attributes(Attribute(40, 40, 40), Attribute(0.5, 0.5, 1), Attribute(0.25, 0.25), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair("minecraft:zombie", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.23, 0.23), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair("minecraft:zombified_piglin", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.23, 0.23), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(5, 5, 5))));
    table->insert(make_pair("minecraft:zombie_villager", Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1), Attribute(0.23, 0.23), Attribute(0.02, 0.02), Attribute(0.02, 0.02), Attribute(16, 16, 2048), Attribute(3, 3, 3))));

    // 1.17
    table->insert(make_pair("minecraft:glow_squid", squid));
    table->insert(make_pair("minecraft:axolotl", Attributes(
                                                     Attribute(14, 14, 14),   // health
                                                     Attribute(0, 0, 1),      // knockback_resistance
                                                     Attribute(0.1, 0.1),     // movement
                                                     Attribute(0.2, 0.2),     // underwater_movement
                                                     Attribute(0.02, 0.02),   // lava_movement
                                                     Attribute(16, 16, 2048), // follow_range
                                                     Attribute(2, 2, 2))));   // attack_damage
    table->insert(make_pair("minecraft:goat", Attributes(
                                                  Attribute(10, 10, 10),   // health
                                                  Attribute(0, 0, 1),      // knockback_resistance
                                                  Attribute(0.4, 0.4),     // movement
                                                  Attribute(0.02, 0.02),   // underwater_movement
                                                  Attribute(0.02, 0.02),   // lava_movement
                                                  Attribute(16, 16, 2048), // follow_range
                                                  Attribute(2, 2, 2))));   // attack_damage

    // 1.19
    table->insert(make_pair("minecraft:frog", Attributes(
                                                  Attribute(10, 10, 10),   // health
                                                  Attribute(0, 0, 1),      // knockback_resistance
                                                  Attribute(0.1, 0.1),     // movement
                                                  Attribute(0.15, 0.15),   // underwater_movement
                                                  Attribute(0.02, 0.02),   // lava_movement
                                                  Attribute(16, 16, 2048), // follow_range
                                                  nullopt)));              // attack_damage

    return table;
  }
};

} // namespace je2be
