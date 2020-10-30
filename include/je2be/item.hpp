#pragma once

namespace j2b {

class Item {
private:
    using ItemData = std::shared_ptr<mcfile::nbt::CompoundTag>;
    using Converter = std::function<ItemData(std::string const&, mcfile::nbt::CompoundTag const&)>;
    using CompoundTag = mcfile::nbt::CompoundTag;

public:
    static std::shared_ptr<CompoundTag> From(CompoundTag const& item) {
        using namespace props;
        using namespace std;
        using namespace mcfile::nbt;

        static unique_ptr<unordered_map<string, Converter> const> const blockItemMapping(CreateBlockItemConverterTable());
        static unique_ptr<unordered_map<string, Converter> const> const itemMapping(CreateItemConverterTable());

        auto id = GetString(item, "id");
        if (!id) return nullptr;
        string const& name = *id;

        if (IsItem(name)) {
            auto found = itemMapping->find(name);
            if (found == itemMapping->end()) {
                return DefaultItem(name, item);
            } else {
                return found->second(name, item);
            }
        } else {
            auto found = blockItemMapping->find(name);
            if (found == blockItemMapping->end()) {
                return DefaultBlockItem(name, item);
            } else {
                return found->second(name, item);
            }
        }
    }

private:
    static std::unordered_map<std::string, Converter>* CreateBlockItemConverterTable() {
        using namespace std;
        using namespace mcfile::nbt;
        auto table = new unordered_map<string, Converter>();
#define E(__name, __func) table->insert(make_pair("minecraft:" __name, __func))
        E("brown_mushroom_block", MushroomBlock);
        E("red_mushroom_block", MushroomBlock);
        E("white_bed", Bed);
        E("orange_bed", Bed);
        E("magenta_bed", Bed);
        E("light_blue_bed", Bed);
        E("yellow_bed", Bed);
        E("lime_bed", Bed);
        E("pink_bed", Bed);
        E("gray_bed", Bed);
        E("light_gray_bed", Bed);
        E("cyan_bed", Bed);
        E("purple_bed", Bed);
        E("blue_bed", Bed);
        E("brown_bed", Bed);
        E("green_bed", Bed);
        E("red_bed", Bed);
        E("black_bed", Bed);
        E("white_banner", Banner);
        E("orange_banner", Banner);
        E("magenta_banner", Banner);
        E("light_blue_banner", Banner);
        E("yellow_banner", Banner);
        E("lime_banner", Banner);
        E("pink_banner", Banner);
        E("gray_banner", Banner);
        E("light_gray_banner", Banner);
        E("cyan_banner", Banner);
        E("purple_banner", Banner);
        E("blue_banner", Banner);
        E("brown_banner", Banner);
        E("green_banner", Banner);
        E("red_banner", Banner);
        E("black_banner", Banner);

        E("skeleton_skull", Skull);
        E("wither_skeleton_skull", Skull);
        E("player_head", Skull);
        E("zombie_head", Skull);
        E("greeper_head", Skull);
        E("dragon_head", Skull);

        E("item_frame", Rename("frame"));
        E("repeater", DefaultItem);
        E("comparator", DefaultItem);

        E("iron_door", DefaultItem);
        E("oak_door", Rename("wooden_door"));
        E("spruce_door", DefaultItem);
        E("birch_door", DefaultItem);
        E("jungle_door", DefaultItem);
        E("acacia_door", DefaultItem);
        E("dark_oak_door", DefaultItem);
        E("crimson_door", DefaultItem);
        E("warped_door", DefaultItem);

        E("campfire", DefaultItem);
        E("soul_campfire", DefaultItem);
        E("chain", DefaultItem);

        E("torch", AnyTorch);
        E("soul_torch", AnyTorch);
        E("redstone_torch", AnyTorch);

        E("oak_sign", Rename("sign"));
        E("birch_sign", DefaultItem);
        E("spruce_sign", DefaultItem);
        E("jungle_sign", DefaultItem);
        E("acacia_sign", DefaultItem);
        E("dark_oak_sign", Rename("darkoak_sign"));
        E("crimson_sign", DefaultItem);
        E("warped_sign", DefaultItem);
#undef E
        return table;
    }

    static std::unordered_map<std::string, Converter>* CreateItemConverterTable() {
        using namespace std;
        using namespace mcfile::nbt;
        auto table = new unordered_map<string, Converter>();
#define E(__name, __func) table->insert(make_pair("minecraft:" __name, __func))
        E("oak_door", Rename("wooden_door"));
        E("furnace_minecart", Rename("minecart")); // furnace minecart does not exist in bedrock
        E("oak_boat", Subtype("boat", 0));
        E("spruce_boat", Subtype("boat", 1));
        E("birch_boat", Subtype("boat", 2));
        E("jungle_boat", Subtype("boat", 3));
        E("acacia_boat", Subtype("boat", 4));
        E("dark_oak_boat", Subtype("boat", 5));
        E("carrot_on_a_stick", Rename("carrotonastick"));
        E("scute", Rename("turtle_shell_piece"));
        E("charcoal", Subtype("coal", 1));
        E("milk_bucket", Subtype("bucket", 1));
        E("water_bucket", Subtype("bucket", 8));
        E("lava_bucket", Subtype("bucket", 10));
        E("cod_bucket", Subtype("bucket", 2));
        E("salmon_bucket", Subtype("bucket", 3));
        E("tropical_fish_bucket", Subtype("bucket", 4)); //TODO: tropical fish entity is in CompoundTag, key named "tag"
        E("pufferfish_bucket", Subtype("bucket", 5));

        E("ink_sac", Subtype("dye", 0));
        E("red_dye", Subtype("dye", 1));
        E("green_dye", Subtype("dye", 2));
        E("cocoa_beans", Subtype("dye", 3));
        E("lapis_lazuli", Subtype("dye", 4));
        E("purple_dye", Subtype("dye", 5));
        E("cyan_dye", Subtype("dye", 6));
        E("light_gray_dye", Subtype("dye", 7));
        E("gray_dye", Subtype("dye", 8));
        E("pink_dye", Subtype("dye", 9));
        E("lime_dye", Subtype("dye", 10));
        E("yellow_dye", Subtype("dye", 11));
        E("light_blue_dye", Subtype("dye", 12));
        E("magenta_dye", Subtype("dye", 13));
        E("orange_dye", Subtype("dye", 14));
        E("bone_meal", Subtype("dye", 15));
        E("black_dye", Subtype("dye", 16));
        E("brown_dye", Subtype("dye", 17));
        E("blue_dye", Subtype("dye", 18));
        E("white_dye", Subtype("dye", 19));

#define G(__mob, __id) table->insert(make_pair("minecraft:" __mob "_spawn_egg", SpawnEgg(__mob, __id)))
        G("bee", 122);
        G("chicken", 10);
        G("mooshroom", 16);
        G("polar_bear", 28);
        G("cow", 11);
        G("pig", 12);
        G("sheep", 13);
        G("wolf", 14);
        G("ocelot", 22);
        G("cat", 75);
        G("parrot", 30);
        G("bat", 19);
        G("turtle", 74);
        G("mule", 25);
        G("rabbit", 18);
        G("llama", 29);
        G("horse", 23);
        G("donkey", 24);
        G("skeleton_horse", 26);
        G("zombie_horse", 27);
        G("fox", 121);
        G("panda", 113);
        G("dolphin", 31);
        G("pufferfish", 108);
        G("salmon", 109);
        G("cod", 112);
        E("tropical_fish_spawn_egg", SpawnEgg("tropicalfish", 111));
        G("enderman", 38);
        G("creeper", 33);
        G("silverfish", 39);
        G("skeleton", 34);
        G("witch", 45);
        G("slime", 37);
        G("wither_skeleton", 48);
        G("stray", 46);
        G("spider", 35);
        G("zombie", 32);
        G("elder_guardian", 50);
        G("guardian", 49);
        G("cave_spider", 40);
        G("squid", 17);
        G("drowned", 110);
        G("husk", 47);
        E("zombified_piglin_spawn_egg", SpawnEgg("zombie_pigman", 36));
        G("magma_cube", 42);
        G("endermite", 55);
        G("wandering_trader", 118);
        G("piglin_brute", 127);
        G("strider", 125);
        G("hoglin", 124);
        G("piglin", 123);
        G("zoglin", 126);
        G("ghast", 41);
        G("blaze", 43);
        G("phantom", 58);
        E("zombie_villager_spawn_egg", SpawnEgg("zombie_villager_v2", 116));
        E("villager_spawn_egg", SpawnEgg("villager_v2", 115));
        E("evoker_spawn_egg", SpawnEgg("evocation_illager", 104));
        G("vex", 105);
        G("vindicator", 57);
        G("shulker", 54);
        G("ravager", 59);
        G("pillager", 114);
#undef G

        E("fire_charge", Rename("fireball"));
        E("map", Subtype("emptymap", 2));
        E("firework_rocket", FireworkRocket);
        E("nether_star", Rename("netherstar"));
        E("firework_star", FireworkStar);

        E("leather_horse_armor", Rename("horsearmorleather"));
        E("iron_horse_armor", Rename("horsearmoriron"));
        E("golden_horse_armor", Rename("horsearmorgold"));
        E("diamond_horse_armor", Rename("horsearmordiamond"));
        E("popped_chorus_fruit", Rename("chorus_fruit_popped"));
        E("nether_brick", Rename("netherbrick"));
        for (string title : {"13", "cat", "blocks", "chirp", "far", "mall", "mellohi", "stal", "strad", "ward", "11", "wait", "pigstep"}) {
            table->insert(make_pair("minecraft:music_disc_" + title, Rename("record_" + title)));
        }
        E("flower_banner_pattern", Subtype("banner_pattern", 2));
        E("creeper_banner_pattern", Subtype("banner_pattern", 1));
        E("skull_banner_pattern", Subtype("banner_pattern", 0));
        E("piglin_banner_pattern", Subtype("banner_pattern", 6));
        E("mojang_banner_pattern", Subtype("banner_pattern", 3));

        E("cod", Rename("fish"));
        E("enchanted_golden_apple", Rename("appleenchanted"));
        E("cooked_cod", Rename("cooked_fish"));
        E("cooked_mutton", Rename("muttoncooked"));
        E("tropical_fish", Rename("clownfish"));
        E("melon_slice", Rename("melon"));
        E("mutton", Rename("muttonraw"));
        E("tipped_arrow", TippedArrow);
        E("totem_of_undying", Rename("totem"));
        E("potion", AnyPotion);
        E("splash_potion", AnyPotion);
        E("lingering_potion", AnyPotion);
#undef E
        return table;
    }

    static ItemData AnyPotion(std::string const& name, CompoundTag const& item) {
        auto tag = New(name, true);
        auto count = props::GetByteOrDefault(item, "Count", 1);
        auto t = item.query("tag")->asCompound();
        int16_t type = 0;
        if (t) {
            auto potion = props::GetStringOrDefault(*t, "Potion", "");
            static std::unordered_map<std::string, int16_t> const mapping = {
                {"minecraft:water", 0},
                {"minecraft:mundane", 1},
                {"minecraft:night_vision", 5},
                {"minecraft:long_night_vision", 6},
                {"minecraft:thick", 2},
                {"minecraft:awkward", 3},
                {"minecraft:invisibility", 7},
                {"minecraft:long_invisibility", 8},
                {"minecraft:leaping", 9},
                {"minecraft:long_leaping", 10},
                {"minecraft:strong_leaping", 11},
                {"minecraft:fire_resistance", 12},
                {"minecraft:long_fire_resistance", 13},
                {"minecraft:swiftness", 14},
                {"minecraft:long_swiftness", 15},
                {"minecraft:strong_swiftness", 16},
                {"minecraft:slowness", 17},
                {"minecraft:long_slowness", 18},
                {"minecraft:strong_slowness", 42},
                {"minecraft:water_breathing", 19},
                {"minecraft:long_water_breathing", 20},
                {"minecraft:healing", 21},
                {"minecraft:strong_healing", 22},
                {"minecraft:harming", 23},
                {"minecraft:strong_harming", 24},
                {"minecraft:poison", 25},
                {"minecraft:long_poison", 26},
                {"minecraft:strong_poison", 27},
                {"minecraft:regeneration", 28},
                {"minecraft:long_regeneration", 29},
                {"minecraft:strong_regeneration", 30},
                {"minecraft:strength", 31},
                {"minecraft:long_strength", 32},
                {"minecraft:strong_strength", 33},
                {"minecraft:weakness", 34},
                {"minecraft:long_weakness", 35},
                {"minecraft:turtle_master", 37},
                {"minecraft:long_turtle_master", 38},
                {"minecraft:strong_turtle_master", 39},
                {"minecraft:slow_falling", 40},
                {"minecraft:long_slow_falling", 41},
            };
            auto found = mapping.find(potion);
            if (found != mapping.end()) {
                type = found->second;
            }
        }
        tag->fValue["Damage"] = props::Short(type);
        tag->fValue["Count"] = props::Byte(count);
        return Post(tag, item);
    }

    static ItemData TippedArrow(std::string const& name, CompoundTag const& item) {
        auto tag = New("arrow");
        auto count = props::GetByteOrDefault(item, "Count", 1);
        auto t = item.query("tag")->asCompound();
        int16_t type = 0;
        if (t) {
            auto potion = props::GetStringOrDefault(*t, "Potion", "");
            if (potion == "minecraft:night_vision") {
                type = 6;
            } else if (potion == "minecraft:long_night_vision") {
                type = 7;
            } else if (potion == "minecraft:fire_resistance") {
                type = 13;
            } else if (potion == "minecraft:long_fire_resistance") {
                type = 14;
            } else if (potion == "minecraft:invisibility") {
                type = 8;
            } else if (potion == "minecraft:long_invisibility") {
                type = 9;
            } else if (potion == "minecraft:leaping") {
                type = 10;
            } else if (potion == "minecraft:long_leaping") {
                type = 11;
            } else if (potion == "minecraft:strong_leaping") {
                type = 12;
            } else if (potion == "minecraft:swiftness") {
                type = 15;
            } else if (potion == "minecraft:long_swiftness") {
                type = 16;
            } else if (potion == "minecraft:strong_swiftness") {
                type = 17;
            } else if (potion == "minecraft:slowness") {
                type = 18;
            } else if (potion == "minecraft:long_slowness") {
                type = 19;
            } else if (potion == "minecraft:strong_slowness") {
                type = 43;
            } else if (potion == "minecraft:water_breathing") {
                type = 20;
            } else if (potion == "minecraft:long_water_breathing") {
                type = 21;
            } else if (potion == "minecraft:healing") {
                type = 22;
            } else if (potion == "minecraft:strong_healing") {
                type = 23;
            } else if (potion == "minecraft:harming") {
                type = 24;
            } else if (potion == "minecraft:strong_harming") {
                type = 25;
            } else if (potion == "minecraft:poison") {
                type = 26;
            } else if (potion == "minecraft:long_poison") {
                type = 27;
            } else if (potion == "minecraft:strong_poison") {
                type = 28;
            } else if (potion == "minecraft:regeneration") {
                type = 29;
            } else if (potion == "minecraft:long_regeneration") {
                type = 30;
            } else if (potion == "minecraft:strong_regeneration") {
                type = 31;
            } else if (potion == "minecraft:strength") {
                type = 32;
            } else if (potion == "minecraft:long_strength") {
                type = 33;
            } else if (potion == "minecraft:strong_strength") {
                type = 34;
            } else if (potion == "minecraft:weakness") {
                type = 35;
            } else if (potion == "minecraft:long_weakness") {
                type = 36;
            } else if (potion == "minecraft:turtle_master") {
                type = 38;
            } else if (potion == "minecraft:long_turtle_master") {
                type = 39;
            } else if (potion == "minecraft:strong_turtle_master") {
                type = 40;
            } else if (potion == "minecraft:slow_falling") {
                type = 41;
            } else if (potion == "minecraft:long_slow_falling") {
                type = 42;
            }
        }
        tag->fValue["Damage"] = props::Short(type);
        tag->fValue["Count"] = props::Byte(count);
        return Post(tag, item);
    }

    static ItemData FireworkStar(std::string const& name, CompoundTag const& item) {
        //TODO: colors and effects.
        return Rename("fireworkscharge")(name, item);
    }

    static ItemData FireworkRocket(std::string const& name, CompoundTag const& item) {
        //TODO: colors and effects.
        return Rename("fireworks")(name, item);
    }

    static Converter SpawnEgg(std::string const& mob, int16_t damage) {
        using namespace std;
        return [=](string const& name, CompoundTag const& item) {
            auto tag = New("spawn_egg");
            tag->fValue["Damage"] = props::Short(damage);
            tag->fValue["Identifier"] = props::String("minecraft:"s + mob);
            return Post(tag, item);
        };
    }

    static Converter Subtype(std::string const& newName, int16_t damage) {
        return [=](std::string const& name, CompoundTag const& item) {
            auto tag = New(newName);
            tag->fValue["Damage"] = props::Short(damage);
            return Post(tag, item);
        };
    }

    static std::unordered_set<std::string>* CreateItemNameList() {
        using namespace std;
        auto table = new unordered_set<string>();
#define E(__name) table->insert("minecraft:" __name)
        E("minecart");
        E("saddle");
        E("oak_boat");
        E("chest_minecart");
        E("furnace_minecart");
        E("carrot_on_a_stick");
        E("warped_fungus_on_a_stick");
        E("tnt_minecart");
        E("hopper_minecart");
        E("elytra");
        E("spruce_boat");
        E("birch_boat");
        E("jungle_boat");
        E("acacia_boat");
        E("dark_oak_boat");

        E("beacon");
        E("turtle_egg");
        E("conduit");
        E("scute");
        E("coal");
        E("charcoal");
        E("diamond");
        E("iron_ingot");
        E("gold_ingot");
        E("netherite_ingot");
        E("netherite_scrap");
        E("stick");
        E("bowl");
        E("string");
        E("feather");
        E("gunpowder");
        E("wheat_seeds");
        E("wheat");
        E("flint");
        E("bucket");
        E("water_bucket");
        E("lava_bucket");
        E("snowball");
        E("leather");
        E("milk_bucket");
        E("pufferfish_bucket");
        E("salmon_bucket");
        E("cod_bucket");
        E("tropical_fish_bucket");
        E("brick");
        E("clay_ball");
        E("paper");
        E("book");
        E("slime_ball");
        E("egg");
        E("glowstone_dust");
        E("ink_sac");
        E("cocoa_beans");
        E("lapis_lazuli");
        E("white_dye");
        E("orange_dye");
        E("magenta_dye");
        E("light_blue_dye");
        E("yellow_dye");
        E("lime_dye");
        E("pink_dye");
        E("gray_dye");
        E("light_gray_dye");
        E("cyan_dye");
        E("purple_dye");
        E("blue_dye");
        E("brown_dye");
        E("green_dye");
        E("red_dye");
        E("black_dye");
        E("bone_meal");
        E("bone");
        E("sugar");
        E("pumpkin_seeds");
        E("melon_seeds");
        E("ender_pearl");
        E("blaze_rod");
        E("gold_nugget");
        E("nether_wart");
        E("ender_eye");
        E("bat_spawn_egg");
        E("bee_spawn_egg");
        E("blaze_spawn_egg");
        E("cat_spawn_egg");
        E("cave_spider_spawn_egg");
        E("chicken_spawn_egg");
        E("cod_spawn_egg");
        E("cow_spawn_egg");
        E("creeper_spawn_egg");
        E("dolphin_spawn_egg");
        E("donkey_spawn_egg");
        E("drowned_spawn_egg");
        E("elder_guardian_spawn_egg");
        E("enderman_spawn_egg");
        E("endermite_spawn_egg");
        E("evoker_spawn_egg");
        E("fox_spawn_egg");
        E("ghast_spawn_egg");
        E("guardian_spawn_egg");
        E("hoglin_spawn_egg");
        E("horse_spawn_egg");
        E("husk_spawn_egg");
        E("llama_spawn_egg");
        E("magma_cube_spawn_egg");
        E("mooshroom_spawn_egg");
        E("mule_spawn_egg");
        E("ocelot_spawn_egg");
        E("panda_spawn_egg");
        E("parrot_spawn_egg");
        E("phantom_spawn_egg");
        E("pig_spawn_egg");
        E("piglin_spawn_egg");
        E("piglin_brute_spawn_egg");
        E("pillager_spawn_egg");
        E("polar_bear_spawn_egg");
        E("pufferfish_spawn_egg");
        E("rabbit_spawn_egg");
        E("ravager_spawn_egg");
        E("salmon_spawn_egg");
        E("sheep_spawn_egg");
        E("shulker_spawn_egg");
        E("silverfish_spawn_egg");
        E("skeleton_spawn_egg");
        E("skeleton_horse_spawn_egg");
        E("slime_spawn_egg");
        E("spider_spawn_egg");
        E("squid_spawn_egg");
        E("stray_spawn_egg");
        E("strider_spawn_egg");
        E("trader_llama_spawn_egg"); // there is no spawn egg for trader llama for bedrock
        E("tropical_fish_spawn_egg");
        E("turtle_spawn_egg");
        E("vex_spawn_egg");
        E("villager_spawn_egg");
        E("vindicator_spawn_egg");
        E("wandering_trader_spawn_egg");
        E("witch_spawn_egg");
        E("wither_skeleton_spawn_egg");
        E("wolf_spawn_egg");
        E("zoglin_spawn_egg");
        E("zombie_spawn_egg");
        E("zombie_horse_spawn_egg");
        E("zombie_villager_spawn_egg");
        E("zombified_piglin_spawn_egg");
        E("experience_bottole");
        E("fire_charge");
        E("writable_book");
        E("emerald");
        E("map");
        E("nether_star");
        E("firework_rocket");
        E("firework_star");
        E("nether_brick");
        E("quartz");
        E("prismarine_shard");
        E("prismarine_crystals");
        E("rabbit_hide");
        E("iron_horse_armor");
        E("golden_horse_armor");
        E("diamond_horse_armor");
        E("leather_horse_armor");
        E("chorus_fruit");
        E("popped_chorus_fruit");
        E("beetroot_seeds");
        E("shulker_shell");
        E("iron_nugget");
        E("music_disc_13");
        E("music_disc_cat");
        E("music_disc_blocks");
        E("music_disc_chirp");
        E("music_disc_far");
        E("music_disc_mall");
        E("music_disc_mellohi");
        E("music_disc_stal");
        E("music_disc_strad");
        E("music_disc_ward");
        E("music_disc_11");
        E("music_disc_wait");
        E("music_disc_pigstep");
        E("nautilus_shell");
        E("heart_of_the_sea");
        E("flower_banner_pattern");
        E("creeper_banner_pattern");
        E("skull_banner_pattern");
        E("mojang_banner_pattern");
        E("globe_banner_pattern"); // there is no globe banner pattern for bedrock
        E("piglin_banner_pattern");
        E("honeycomb");

        E("apple");
        E("mushroom_stew");
        E("bread");
        E("porkchop");
        E("cooked_porkchop");
        E("golden_apple");
        E("enchanted_golden_apple");
        E("cod");
        E("salmon");
        E("tropical_fish");
        E("pufferfish");
        E("cooked_cod");
        E("cooked_salmon");
        E("cake");
        E("cookie");
        E("melon_slice");
        E("dried_kelp");
        E("beef");
        E("cooked_beef");
        E("chicken");
        E("cooked_chicken");
        E("rotten_flesh");
        E("spider_eye");
        E("carrot");
        E("potato");
        E("baked_potato");
        E("poisonous_potato");
        E("pumpkin_pie");
        E("rabbit");
        E("cooked_rabbit");
        E("rabbit_stew");
        E("mutton");
        E("cooked_mutton");
        E("beetroot");
        E("beetroot_soup");
        E("sweet_berries");
        E("honey_bottle");

        E("flint_and_steel");
        for (string type : {"wooden", "stone", "golden", "iron", "diamond", "netherite"}) {
            for (string tool : {"shovel", "pickaxe", "axe", "hoe"}) {
                table->insert(type + "_" + tool);
            }
        }
        E("compass");
        E("fishing_rod");
        E("clock");
        E("shears");
        E("enchanted_book");
        E("lead");
        E("name_tag");
        
        E("turtle_helmet");
        E("bow");
        E("arrow");
        E("wooden_sword");
        E("stone_sword");
        E("golden_sword");
        E("iron_sword");
        E("diamond_sword");
        E("netherite_sword");
        for (string type : {"leather", "chainmail", "iron", "diamond", "golden", "netherite"}) {
            for (string item : {"helmet", "chestplate", "leggings", "boots"}) {
                table->insert(type + "_" + item);
            }
        }
        E("spectral_arrow");
        E("tipped_arrow");
        E("shield");
        E("totem_of_undying");
        E("trident");
        E("crossbow");

        E("ghast_tear");
        E("potion");
        E("glass_bottle");
        E("fermented_spider_eye");
        E("blaze_powder");
        E("magma_cream");
        E("brewing_stand");
        E("cauldron");
        E("glistering_melon_slice");
        E("golden_carrot");
        E("rabbit_foot");
        E("dragon_breath");
        E("splash_potion");
        E("lingering_potion");
        E("phantom_membrane");

        // listed with blocks, but not block item

        E("repeater");
        E("comparator");
        E("iron_door");
        E("oak_door");
        E("spruce_door");
        E("birch_door");
        E("jungle_door");
        E("acacia_door");
        E("dark_oak_door");
        E("crimson_door");
        E("warped_door");
        E("campfire");
        E("soul_campfire");
        E("hopper");
#undef E
        return table;
    }
    
    static bool IsItem(std::string const& name) {
        using namespace std;
        static unique_ptr<unordered_set<string> const> const list(CreateItemNameList());
        return list->find(name) != list->end();
    }

    static ItemData AnyTorch(std::string const& name, CompoundTag const& item) {
        using namespace std;
        using namespace props;

        auto count = GetByteOrDefault(item, "Count", 1);

        map<string, string> empty;
        auto block = make_shared<mcfile::Block>(name, empty);
        auto blockData = BlockData::From(block);

        auto states = make_shared<CompoundTag>();
        states->fValue = {
            {"torch_facing_direction", String("unknown")},
        };
        blockData->fValue["states"] = states;

        auto tag = New(name, true);
        tag->fValue["Count"] = Byte(count);
        tag->fValue["Damage"] = Short(0);
        tag->fValue["Block"] = blockData;
        return Post(tag, item);
    }

    static Converter Rename(std::string const& newName) {
        return [=](std::string const& name, CompoundTag const& item) {
            auto tag = New(newName);
            return Post(tag, item);
        };
    }

    static ItemData Skull(std::string const& name, CompoundTag const& item) {
        int8_t type = TileEntity::GetSkullTypeFromBlockName(name);
        auto tag = New("skull");
        tag->fValue["Damage"] = props::Short(type);
        return Post(tag, item);
    }

    static ItemData Banner(std::string const& name, CompoundTag const& item) {
        auto colorName = strings::RTrim(strings::LTrim(name, "minecraft:"), "_banner");
        BannerColorCodeBedrock color = BannerColorCodeFromName(colorName);
        int16_t damage = (int16_t)color;
        auto tag = New("banner");
        tag->fValue["Damage"] = props::Short(damage);
        return Post(tag, item);
    }

    static ItemData Bed(std::string const& name, CompoundTag const& item) {
        using namespace std;
        string colorName = strings::RTrim(strings::LTrim(name, "minecraft:"), "_bed");
        ColorCodeJava color = ColorCodeJavaFromName(colorName);
        int16_t damage = (int16_t)color;
        auto tag = New("bed");
        tag->fValue["Damage"] = props::Short(damage);
        return Post(tag, item);
    }

    static ItemData MushroomBlock(std::string const& name, CompoundTag const& item) {
        using namespace std;
        using namespace props;

        auto count = GetByteOrDefault(item, "Count", 1);

        map<string, string> empty;
        auto block = make_shared<mcfile::Block>(name, empty);
        auto blockData = BlockData::From(block);

        auto states = make_shared<CompoundTag>();
        states->fValue = {
            {"huge_mushroom_bits", Int(14)},
        };
        blockData->fValue["states"] = states;

        auto tag = New(name, true);
        tag->fValue["Count"] = Byte(count);
        tag->fValue["Damage"] = Short(0);
        tag->fValue["Block"] = blockData;
        return Post(tag, item);
    }

    static ItemData New(std::string const& name, bool fullname = false) {
        using namespace props;
        std::string n;
        if (fullname) {
            n = name;
        } else {
            n = "minecraft:" + name;
        }
        auto tag = std::make_shared<CompoundTag>();
        tag->fValue = {
            {"Name", String(name)},
            {"WasPickedUp", Bool(false)},
            {"Count", Byte(1)},
            {"Damage", Short(0)},
        };
        return tag;
    }

    Item() = delete;

    static ItemData DefaultBlockItem(std::string const& id, CompoundTag const& item) {
        using namespace std;
        using namespace props;
        using namespace mcfile::nbt;

        auto count = GetByteOrDefault(item, "Count", 1);

        map<string, string> p;
        auto block = make_shared<mcfile::Block>(id, p);
        auto blockData = BlockData::From(block);
        assert(blockData);

        auto name = GetString(*blockData, "name");
        if (!name) return nullptr;

        auto tag = make_shared<CompoundTag>();
        tag->fValue = {
            {"Name", String(ItemNameFromBlockName(*name))},
            {"Count", Byte(count)},
            {"WasPickedUp", Bool(false)},
            {"Damage", Short(0)},
        };
        tag->fValue["Block"] = blockData;
        return Post(tag, item);
    }

    static ItemData DefaultItem(std::string const& name, CompoundTag const& item) {
        using namespace props;

        auto count = GetByteOrDefault(item, "Count", 1);
        auto tag = std::make_shared<CompoundTag>();
        tag->fValue = {
            {"Name", String(name)},
            {"Count", Byte(count)},
            {"WasPickedUp", Bool(false)},
            {"Damage", Short(0)},
        };
        return Post(tag, item);
    }

    static ItemData Post(ItemData const& tag, CompoundTag const& item) {
        //TODO: custom name, enchant, etc
        return tag;
    }

    // converts block name (bedrock) to item name (bedrock)
    static std::string ItemNameFromBlockName(std::string const& name) {
        if (name == "minecraft:concretePowder") {
            return "minecraft:concrete_powder";
        }
        return name;
    }
};

}
