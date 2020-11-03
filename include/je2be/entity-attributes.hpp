#pragma once

namespace j2b {

class EntityAttributes {
	using AttributesData = std::shared_ptr<mcfile::nbt::ListTag>;
	using Provider = std::function<AttributesData(void)>;
	using CompoundTag = mcfile::nbt::CompoundTag;


public:
	static std::shared_ptr<mcfile::nbt::ListTag> Mob(std::string const& name) {
		static std::unique_ptr<std::unordered_map<std::string, Provider> const> const table(CreateTable());
		auto found = table->find(name);
		if (found == table->end()) {
#ifndef NDEBUG
			std::cout << "Entity Attributes unknown for mob: " << name << std::endl;
#endif
			return nullptr;
		}
		return found->second();
	}

private:
	struct Parameters {
		float health;
		float movement;
		float followRange = 16;
		std::optional<float> attackDamage = std::nullopt;
		float underwaterMovement = 0.02f;
		float lavaMovement = 0.02f;
	};

	static std::unordered_map<std::string, Provider>* CreateTable() {
		using namespace std;
		auto table = new unordered_map<string, Provider>();

#define E(__name, __provider) table->insert(make_pair("minecraft:" __name, __provider))
#define D(__name, __health, __movement) table->insert(make_pair("minecraft:" __name, Attrs({.health = __health, .movement = __movement})))
		E("bee", Attrs({.health = 10, .movement = 0.3f, .followRange = 1024}));
		E("cat", Attrs({.health = 10, .movement = 0.3f, .attackDamage = 4}));
		D("chicken", 4, 0.25f);
		D("cow", 10, 0.25f);
		D("pig", 10, 0.25f);
		D("wolf", 8, 0.3f);
		E("polar_bear", Attrs({.health = 30, .movement = 0.25f, .followRange = 48}));
		E("ocelot", Attrs({.health = 10, .movement = 0.3f, .attackDamage = 4}));
		D("mooshroom", 10, 0.25f);
		D("bat", 6, 0.1f);
		D("parrot", 6, 0.4f);
		E("salmon", Attrs({.health = 6, .movement = 0.12, .underwaterMovement = 0.12}));
		D("rabbit", 3, 0.3f);
		// horse and its variants: random movement and jump_strength
		E("llama", Attrs({.health = 21, .movement = 0.25f, .followRange = 40}));
		E("spider", Attrs({.health = 16, .movement = 0.3f, .attackDamage = 3}));
		E("tropical_fish", Attrs({.health = 6, .movement = 0.12f, .underwaterMovement = 0.12f}));
		E("cod", Attrs({.health = 6, .movement = 0.1f, .underwaterMovement = 0.1f}));
		E("pufferfish", Attrs({.health = 6, .movement = 0.13f, .underwaterMovement = 0.13f}));
		E("dolphin", Attrs({.health = 10, .movement = 0.1f, .followRange = 48, .attackDamage = 3, .underwaterMovement = 0.15f}));

		E("villager", Attrs({.health = 20, .movement = 0.5f, .followRange = 128}));
#undef D
#undef E
		return table;
	}

	static std::shared_ptr<CompoundTag> Attribute(float base, float current, float max, std::string const& name) {
		auto a = std::make_shared<CompoundTag>();
		a->fValue["Base"] = props::Float(base);
		a->fValue["Current"] = props::Float(current);
		a->fValue["Max"] = props::Float(max);
		a->fValue["Name"] = props::String(name);
		return a;
	}

	static Provider Attrs(Parameters p) {
		using namespace mcfile::nbt;
		return [=]() {
			auto list = std::make_shared<ListTag>();
			list->fType = Tag::TAG_Compound;
			static float constexpr fmax = std::numeric_limits<float>::max();
			list->fValue = {
				Attribute(0, 0, 1024, "minecraft:luck"),
				Attribute(p.health, p.health, p.health, "minecraft:helth"),
				Attribute(0, 0, 16, "minecraft:absorption"),
				Attribute(0, 0, 1, "minecraft:knockback_resistance"),
				Attribute(p.movement, p.movement, fmax, "minecraft:movement"),
				Attribute(p.underwaterMovement, p.underwaterMovement, fmax, "minecraft:underwater_movement"),
				Attribute(p.lavaMovement, p.lavaMovement, fmax, "minecraft:lava_movement"),
				Attribute(p.followRange, p.followRange, 2048, "minecraft:follow_range"),
			};
			if (p.attackDamage) {
				list->fValue.push_back(Attribute(*p.attackDamage, *p.attackDamage, fmax, "minecraft:attack_damage"));
			}
			return list;
		};
	}
};

}
