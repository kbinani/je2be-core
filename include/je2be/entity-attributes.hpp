#pragma once

namespace j2b {

class EntityAttributes {
  using AttributesData = std::shared_ptr<mcfile::nbt::ListTag>;
  using Provider = std::function<AttributesData(void)>;
  using CompoundTag = mcfile::nbt::CompoundTag;

  struct Attribute {
    float base;
    float current;
    float max;

    Attribute(double base, double current,
              double max = std::numeric_limits<float>::max())
        : base(base), current(current), max(max) {}

    std::shared_ptr<CompoundTag> toCompoundTag(std::string const &name) const {
      auto a = std::make_shared<CompoundTag>();
      a->fValue["Base"] = props::Float(base);
      a->fValue["Current"] = props::Float(current);
      a->fValue["Max"] = props::Float(max);
      a->fValue["Name"] = props::String("minecraft:" + name);
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

    Attributes(Attribute health, Attribute knockback_resistance,
               Attribute movement, Attribute underwater_movement,
               Attribute lava_movement, Attribute follow_range,
               std::optional<Attribute> attack_damage)
        : luck(0, 0, 1024), health(health), absorption(0, 0, 16),
          knockback_resistance(knockback_resistance), movement(movement),
          underwater_movement(underwater_movement),
          lava_movement(lava_movement), follow_range(follow_range),
          attack_damage(attack_damage) {}

    std::shared_ptr<mcfile::nbt::ListTag> toListTag() const {
      using namespace mcfile::nbt;

      auto list = std::make_shared<ListTag>();
      list->fType = Tag::TAG_Compound;
      list->fValue = {
          luck.toCompoundTag("luck"),
          health.toCompoundTag("health"),
          absorption.toCompoundTag("absorption"),
          knockback_resistance.toCompoundTag("knockback_resistance"),
          movement.toCompoundTag("movement"),
          underwater_movement.toCompoundTag("underwater_movement"),
          lava_movement.toCompoundTag("lava_movement"),
          follow_range.toCompoundTag("follow_range"),
      };
      if (attack_damage) {
        list->fValue.push_back(attack_damage->toCompoundTag("attack_damage"));
      }
      return list;
    }
  };

public:
  static std::shared_ptr<mcfile::nbt::ListTag> Mob(std::string const &name) {
    static std::unique_ptr<
        std::unordered_map<std::string, Attributes> const> const
        table(CreateTable());
    auto found = table->find(name);
    if (found == table->end()) {
#ifndef NDEBUG
      std::cout << "Entity Attributes unknown for mob: " << name << std::endl;
#endif
      return nullptr;
    }
    return found->second.toListTag();
  }

  static std::shared_ptr<mcfile::nbt::ListTag>
  AnyHorse(CompoundTag const &tag) {
    using namespace std;
    using namespace props;
    using namespace mcfile::nbt;

    auto attributes = tag.listTag("Attributes");
    Attribute health(15, 15, 15);
    Attribute movement(0.1125, 0.1125);
    Attribute jumpStrength(0.4, 0.4);
    if (attributes) {
      for (auto const &it : attributes->fValue) {
        auto attrs = it->asCompound();
        if (!attrs)
          continue;
        auto name = GetString(*attrs, "Name");
        auto value = GetDouble(*attrs, "Base");
        if (!name || !value)
          continue;
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

    auto ret = make_shared<ListTag>();
    ret->fType = Tag::TAG_Compound;
    ret->fValue = {
        luck.toCompoundTag("luck"),
        health.toCompoundTag("health"),
        movement.toCompoundTag("movement"),
        followRange.toCompoundTag("follow_range"),
        absorption.toCompoundTag("absorption"),
        jumpStrength.toCompoundTag("jump_strength"),
    };
    return ret;
  }

private:
  static std::unordered_map<std::string, Attributes> *CreateTable() {
    using namespace std;
    auto table = new unordered_map<string, Attributes>();
    table->insert(make_pair(
        "minecraft:bat",
        Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1), Attribute(0.1, 0.1),
                   Attribute(0.02, 0.02), Attribute(0.02, 0.02),
                   Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(
        "minecraft:bee", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1),
                                    Attribute(0.3, 0.3), Attribute(0.02, 0.02),
                                    Attribute(0.02, 0.02),
                                    Attribute(1024, 1024, 2048), nullopt)));
    table->insert(
        make_pair("minecraft:blaze",
                  Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                             Attribute(0.23, 0.23), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(48, 48, 48),
                             Attribute(6, 6, 6))));
    table->insert(make_pair(
        "minecraft:cat", Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1),
                                    Attribute(0.3, 0.3), Attribute(0.02, 0.02),
                                    Attribute(0.02, 0.02),
                                    Attribute(16, 16, 2048), Attribute(4, 4))));
    table->insert(make_pair(
        "minecraft:cave_spider",
        Attributes(Attribute(12, 12, 12), Attribute(0, 0, 1),
                   Attribute(0.3, 0.3), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(
        "minecraft:chicken",
        Attributes(Attribute(4, 4, 4), Attribute(0, 0, 1),
                   Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair("minecraft:cod",
                            Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1),
                                       Attribute(0.1, 0.1), Attribute(0.1, 0.1),
                                       Attribute(0.02, 0.02),
                                       Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(
        "minecraft:cow",
        Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1),
                   Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(
        make_pair("minecraft:creeper",
                  Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                             Attribute(0.2, 0.2), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(3, 3, 3))));
    table->insert(
        make_pair("minecraft:dolphin",
                  Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1),
                             Attribute(0.1, 0.1), Attribute(0.15, 0.15),
                             Attribute(0.02, 0.02), Attribute(48, 48, 48),
                             Attribute(3, 3, 3))));
    table->insert(make_pair(
        "minecraft:drowned",
        Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                   Attribute(0.23, 0.23), Attribute(0.06, 0.06),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(
        make_pair("minecraft:elder_guardian",
                  Attributes(Attribute(80, 80, 80), Attribute(0, 0, 1),
                             Attribute(0.3, 0.3), Attribute(0.3, 0.3),
                             Attribute(0.02, 0.02), Attribute(16, 16, 16),
                             Attribute(5, 5, 5))));
    table->insert(
        make_pair("minecraft:ender_dragon",
                  Attributes(Attribute(200, 200, 200), Attribute(100, 1, 1),
                             Attribute(0.3, 0.3), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(3, 3, 3))));
    table->insert(
        make_pair("minecraft:enderman",
                  Attributes(Attribute(40, 40, 40), Attribute(0, 0, 1),
                             Attribute(0.3, 0.3), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(32, 32, 32),
                             Attribute(7, 7, 7))));
    table->insert(
        make_pair("minecraft:endermite",
                  Attributes(Attribute(8, 8, 8), Attribute(0, 0, 1),
                             Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(2, 2, 2))));
    table->insert(make_pair(
        "minecraft:evoker",
        Attributes(Attribute(24, 24, 24), Attribute(0, 0, 1),
                   Attribute(0.5, 0.5), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(64, 64, 2048), nullopt)));
    table->insert(
        make_pair("minecraft:fox",
                  Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                             Attribute(0.3, 0.3), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(2, 2, 2))));
    table->insert(make_pair(
        "minecraft:ghast",
        Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1),
                   Attribute(0.03, 0.03), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(64, 64, 64), nullopt)));
    table->insert(
        make_pair("minecraft:guardian",
                  Attributes(Attribute(30, 30, 30), Attribute(0, 0, 1),
                             Attribute(0.12, 0.12), Attribute(0.12, 0.12),
                             Attribute(0.02, 0.02), Attribute(16, 16, 16),
                             Attribute(5, 5, 5))));
    table->insert(
        make_pair("minecraft:hoglin",
                  Attributes(Attribute(40, 40, 40), Attribute(0.5, 0.5, 1),
                             Attribute(0.3, 0.3), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(3, 3, 3))));
    table->insert(
        make_pair("minecraft:husk",
                  Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                             Attribute(0.23, 0.23), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(3, 3, 3))));
    table->insert(
        make_pair("minecraft:iron_golem",
                  Attributes(Attribute(100, 100, 100), Attribute(1, 1, 1),
                             Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(64, 64, 2048),
                             Attribute(7, 7, 7))));
    Attributes llama(Attribute(27, 27, 27), Attribute(0, 0, 1),
                     Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                     Attribute(0.02, 0.02), Attribute(40, 40, 40), nullopt);
    table->insert(make_pair("minecraft:llama", llama));
    table->insert(make_pair("minecraft:trader_llama", llama));
    table->insert(
        make_pair("minecraft:magma_cube",
                  Attributes(Attribute(16, 16, 16), Attribute(0, 0, 1),
                             Attribute(0.75, 0.75), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(6, 6, 6))));
    table->insert(make_pair(
        "minecraft:mooshroom",
        Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1),
                   Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(
        make_pair("minecraft:ocelot",
                  Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1),
                             Attribute(0.3, 0.3), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(4, 4))));
    table->insert(
        make_pair("minecraft:panda",
                  Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                             Attribute(0.15, 0.15), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(2, 2, 2))));
    table->insert(make_pair(
        "minecraft:parrot",
        Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1), Attribute(0.4, 0.4),
                   Attribute(0.02, 0.02), Attribute(0.02, 0.02),
                   Attribute(16, 16, 2048), nullopt)));
    table->insert(
        make_pair("minecraft:phantom",
                  Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                             Attribute(1.8, 1.8), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(64, 64, 64),
                             Attribute(6, 6, 6))));
    table->insert(make_pair(
        "minecraft:pig",
        Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1),
                   Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(
        make_pair("minecraft:piglin",
                  Attributes(Attribute(16, 16, 16), Attribute(0, 0, 1),
                             Attribute(0.35, 0.35), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(64, 64, 2048),
                             Attribute(5, 5, 5))));
    table->insert(
        make_pair("minecraft:piglin_brute",
                  Attributes(Attribute(50, 50, 50), Attribute(0, 0, 1),
                             Attribute(0.35, 0.35), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(64, 64, 2048),
                             Attribute(7, 7, 7))));
    table->insert(make_pair(
        "minecraft:pillager",
        Attributes(Attribute(24, 24, 24), Attribute(0, 0, 1),
                   Attribute(0.35, 0.35), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(64, 64, 2048), nullopt)));
    table->insert(make_pair(
        "minecraft:polar_bear",
        Attributes(Attribute(30, 30, 30), Attribute(0, 0, 1),
                   Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(48, 48, 2048), nullopt)));
    table->insert(make_pair(
        "minecraft:pufferfish",
        Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1),
                   Attribute(0.13, 0.13), Attribute(0.13, 0.13),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(
        "minecraft:rabbit",
        Attributes(Attribute(3, 3, 3), Attribute(0, 0, 1), Attribute(0.3, 0.3),
                   Attribute(0.02, 0.02), Attribute(0.02, 0.02),
                   Attribute(16, 16, 2048), nullopt)));
    table->insert(
        make_pair("minecraft:ravager",
                  Attributes(Attribute(100, 100, 100), Attribute(0.5, 0.5, 1),
                             Attribute(0.3, 0.3), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(64, 64, 2048),
                             Attribute(12, 12, 12))));
    table->insert(make_pair(
        "minecraft:salmon",
        Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1),
                   Attribute(0.12, 0.12), Attribute(0.12, 0.12),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(
        "minecraft:sheep",
        Attributes(Attribute(8, 8, 8), Attribute(0, 0, 1),
                   Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(
        "minecraft:shulker",
        Attributes(Attribute(30, 30, 30), Attribute(0, 0, 1),
                   Attribute(0, 0, 0), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(
        make_pair("minecraft:silverfish",
                  Attributes(Attribute(8, 8, 8), Attribute(0, 0, 1),
                             Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(1, 1, 1))));
    table->insert(make_pair(
        "minecraft:skeleton",
        Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                   Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(
        make_pair("minecraft:slime",
                  Attributes(Attribute(16, 16, 16), Attribute(0, 0, 1),
                             Attribute(0.6, 0.6), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(4, 4, 4))));
    table->insert(make_pair(
        "minecraft:snow_golem",
        Attributes(Attribute(4, 4, 4), Attribute(0, 0, 1), Attribute(0.2, 0.2),
                   Attribute(0.02, 0.02), Attribute(0.02, 0.02),
                   Attribute(16, 16, 2048), Attribute(2, 2, 2))));
    table->insert(
        make_pair("minecraft:spider",
                  Attributes(Attribute(16, 16, 16), Attribute(0, 0, 1),
                             Attribute(0.3, 0.3), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(3, 3, 3))));
    table->insert(make_pair(
        "minecraft:squid",
        Attributes(Attribute(10, 10, 10), Attribute(0, 0, 1),
                   Attribute(0.2, 0.2), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(
        "minecraft:stray",
        Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                   Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(
        "minecraft:strider",
        Attributes(Attribute(15, 10, 15), Attribute(0, 0, 1),
                   Attribute(0.16, 0.16), Attribute(0.02, 0.02),
                   Attribute(0.32, 0.32), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(
        "minecraft:tropical_fish",
        Attributes(Attribute(6, 6, 6), Attribute(0, 0, 1),
                   Attribute(0.12, 0.12), Attribute(0.12, 0.12),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(
        make_pair("minecraft:turtle",
                  Attributes(Attribute(30, 30, 30), Attribute(0, 0, 1),
                             Attribute(0.1, 0.1), Attribute(0.12, 0.12),
                             Attribute(0.02, 0.02), Attribute(1024, 1024, 2048),
                             nullopt)));
    table->insert(make_pair(
        "minecraft:vex",
        Attributes(Attribute(14, 14, 14), Attribute(0, 0, 1), Attribute(1, 1),
                   Attribute(0.02, 0.02), Attribute(0.02, 0.02),
                   Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(make_pair(
        "minecraft:villager",
        Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                   Attribute(0.5, 0.5), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(128, 128, 2048), nullopt)));
    table->insert(
        make_pair("minecraft:vindicator",
                  Attributes(Attribute(24, 24, 24), Attribute(0, 0, 1),
                             Attribute(0.35, 0.35), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(64, 64, 2048),
                             Attribute(8, 8, 8))));
    table->insert(make_pair(
        "minecraft:wandering_trader",
        Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                   Attribute(0.5, 0.5), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(16, 16, 2048), nullopt)));
    table->insert(make_pair(
        "minecraft:witch",
        Attributes(Attribute(26, 26, 26), Attribute(0, 0, 1),
                   Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(64, 64, 2048), nullopt)));
    table->insert(make_pair(
        "minecraft:wither",
        Attributes(Attribute(600, 600, 600), Attribute(0, 0, 1),
                   Attribute(0.6, 0.6), Attribute(0.02, 0.02),
                   Attribute(0.02, 0.02), Attribute(70, 70, 2048), nullopt)));
    table->insert(
        make_pair("minecraft:wither_skeleton",
                  Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                             Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(4, 4, 4))));
    table->insert(make_pair(
        "minecraft:wolf",
        Attributes(Attribute(8, 8, 8), Attribute(0, 0, 1), Attribute(0.3, 0.3),
                   Attribute(0.02, 0.02), Attribute(0.02, 0.02),
                   Attribute(16, 16, 2048), Attribute(3, 3, 3))));
    table->insert(
        make_pair("minecraft:zoglin",
                  Attributes(Attribute(40, 40, 40), Attribute(0.5, 0.5, 1),
                             Attribute(0.25, 0.25), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(3, 3, 3))));
    table->insert(
        make_pair("minecraft:zombie",
                  Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                             Attribute(0.23, 0.23), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(3, 3, 3))));
    table->insert(
        make_pair("minecraft:zombified_piglin",
                  Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                             Attribute(0.23, 0.23), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(5, 5, 5))));
    table->insert(
        make_pair("minecraft:zombie_villager",
                  Attributes(Attribute(20, 20, 20), Attribute(0, 0, 1),
                             Attribute(0.23, 0.23), Attribute(0.02, 0.02),
                             Attribute(0.02, 0.02), Attribute(16, 16, 2048),
                             Attribute(3, 3, 3))));
    return table;
  }
};

} // namespace j2b
