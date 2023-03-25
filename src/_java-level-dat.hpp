#pragma once

#include "toje/_constants.hpp"

namespace je2be {

class JavaLevelDat {
  JavaLevelDat() = delete;

  enum {
    kLevelVersion = toje::kLevelVersion,
  };

public:
  struct Options {
    std::optional<i64> fRandomSeed;
    std::u8string fVersionString;
    i32 fDataVersion;
    CompoundTagPtr fFlatWorldSettings;
    std::optional<bool> fBonusChestEnabled;
    std::vector<std::u8string> fEnabledDataPacks;
  };

  static CompoundTagPtr TemplateData(Options o) {
    using namespace std;

    auto data = Compound();
    CompoundTag &j = *data;

    j[u8"DataVersion"] = Int(o.fDataVersion);
    j[u8"version"] = Int(kLevelVersion);

    {
      auto dataPacks = Compound();
      dataPacks->set(u8"Disabled", List<Tag::Type::String>());
      auto enabled = List<Tag::Type::String>();
      enabled->push_back(String(u8"vanilla"));
      for (auto const &pack : o.fEnabledDataPacks) {
        enabled->push_back(String(pack));
      }
      dataPacks->set(u8"Enabled", enabled);
      j[u8"DataPacks"] = dataPacks;
    }
    {
      auto brands = List<Tag::Type::String>();
      brands->push_back(String(u8"vanilla"));
      j[u8"ServerBrands"] = brands;
    }
    {
      auto version = Compound();
      version->set(u8"Id", Int(toje::kDataVersion));
      version->set(u8"Name", String(o.fVersionString));
      version->set(u8"Series", String(u8"main"));
      version->set(u8"Snapshot", Bool(false));
      j[u8"Version"] = version;
    }
    {
      auto worldGenSettings = Compound();
      if (o.fBonusChestEnabled) {
        worldGenSettings->set(u8"bonus_chest", Bool(*o.fBonusChestEnabled));
      }
      worldGenSettings->set(u8"generate_features", Bool(true));
      if (o.fRandomSeed) {
        worldGenSettings->set(u8"seed", Long(*o.fRandomSeed));
        auto dimensions = Compound();
        {
          auto overworld = Compound();
          auto generator = Compound();
          if (o.fFlatWorldSettings) {
            generator->set(u8"type", String(u8"minecraft:flat"));
            generator->set(u8"settings", o.fFlatWorldSettings);
          } else {
            auto biomeSource = Compound();
            biomeSource->set(u8"preset", String(u8"minecraft:overworld"));
            biomeSource->set(u8"type", String(u8"minecraft:multi_noise"));
            generator->set(u8"biome_source", biomeSource);
            generator->set(u8"settings", String(u8"minecraft:overworld"));
            generator->set(u8"type", String(u8"minecraft:noise"));
          }
          overworld->set(u8"generator", generator);
          overworld->set(u8"type", String(u8"minecraft:overworld"));
          dimensions->set(u8"minecraft:overworld", overworld);
        }
        {
          auto end = Compound();
          auto generator = Compound();
          auto biomeSource = Compound();
          biomeSource->set(u8"type", String(u8"minecraft:the_end"));
          generator->set(u8"biome_source", biomeSource);
          generator->set(u8"settings", String(u8"minecraft:end"));
          generator->set(u8"type", String(u8"minecraft:noise"));
          end->set(u8"generator", generator);
          end->set(u8"type", String(u8"minecraft:the_end"));
          dimensions->set(u8"minecraft:the_end", end);
        }
        {
          auto nether = Compound();
          auto generator = Compound();
          auto biomeSource = Compound();
          biomeSource->set(u8"preset", String(u8"minecraft:nether"));
          biomeSource->set(u8"type", String(u8"minecraft:multi_noise"));
          generator->set(u8"biome_source", biomeSource);
          generator->set(u8"settings", String(u8"minecraft:nether"));
          generator->set(u8"type", String(u8"minecraft:noise"));
          nether->set(u8"generator", generator);
          nether->set(u8"type", String(u8"minecraft:the_nether"));
          dimensions->set(u8"minecraft:the_nether", nether);
        }
        worldGenSettings->set(u8"dimensions", dimensions);
      }
      j[u8"WorldGenSettings"] = worldGenSettings;
    }

    return data;
  }
};

} // namespace je2be
