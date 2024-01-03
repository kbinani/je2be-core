#pragma once

#include <je2be/nbt.hpp>

#include <iostream>
#include <mutex>

namespace je2be::box360 {

class Attribute {
  Attribute() = delete;

public:
  static CompoundTagPtr Convert(CompoundTag const &in, std::u8string const &entityId) {
    using namespace std;
    auto out = in.copy();
    auto id = in.int32(u8"ID");
    if (!id) {
      return out;
    }
    static vector<optional<u8string>> const sMap = {
        u8"minecraft:generic.max_health",           // 0
        u8"minecraft:generic.follow_range",         // 1
        u8"minecraft:generic.knockback_resistance", // 2
        u8"minecraft:generic.movement_speed",       // 3
        u8"minecraft:generic.attack_damage",        // 4
        u8"minecraft:horse.jump_strength",          // 5
        u8"minecraft:zombie.spawn_reinforcements",  // 6
        u8"minecraft:generic.attack_speed",         // 7
        u8"minecraft:generic.armor",                // 8
        u8"minecraft:generic.armor_toughness",      // 9
        u8"minecraft:generic.luck",                 // 10
        u8"minecraft:generic.flying_speed",         // 11, wither:0.4, parrot:0.4
    };
    optional<u8string> name;
    if (0 <= *id && *id < sMap.size()) {
      name = sMap[*id];
    }
    if (!name) {
#if 0
      static std::mutex sMut;
      lock_guard<std::mutex> lock(sMut);
      using namespace mcfile::u8stream;
      static set<pair<u8string, int>> sReported;
      if (auto found = sReported.find(make_pair(entityId, *id)); found == sReported.end()) {
        std::cout << "unknown attribute id: " << *id << "; entity=" << entityId << "; ";
        mcfile::nbt::PrintAsJson(cout, in);
        sReported.insert(make_pair(entityId, *id));
      }
#endif
      return out;
    }
    out->erase(u8"ID");
    out->set(u8"Name", String(*name));
    if (auto base = in.float64(u8"Base"); base) {
      out->set(u8"Base", Double(*base));
    }
    return out;
  }
};

} // namespace je2be::box360
