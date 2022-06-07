#pragma once

namespace je2be {

class JavaLevelDat {
  JavaLevelDat() = delete;

  enum {
    kLevelVersion = toje::kLevelVersion,
  };

public:
  struct Options {
    std::optional<int64_t> fRandomSeed;
    std::string fVersionString;
    int32_t fDataVersion;
    CompoundTagPtr fFlatWorldSettings;
    std::optional<bool> fBonusChestEnabled;
  };

  static CompoundTagPtr TemplateData(Options o) {
    using namespace std;

    auto data = Compound();
    CompoundTag &j = *data;

    j["DataVersion"] = Int(o.fDataVersion);
    j["version"] = Int(kLevelVersion);

    {
      auto dataPacks = Compound();
      dataPacks->set("Disabled", List<Tag::Type::String>());
      auto enabled = List<Tag::Type::String>();
      enabled->push_back(String("vanilla"));
      dataPacks->set("Enabled", enabled);
      j["DataPacks"] = dataPacks;
    }
    {
      auto brands = List<Tag::Type::String>();
      brands->push_back(String("vanilla"));
      j["ServerBrands"] = brands;
    }
    {
      auto version = Compound();
      version->set("Id", Int(toje::kDataVersion));
      version->set("Name", String(o.fVersionString));
      version->set("Series", String("main"));
      version->set("Snapshot", Bool(false));
      j["Version"] = version;
    }
    {
      auto worldGenSettings = Compound();
      if (o.fBonusChestEnabled) {
        worldGenSettings->set("bonus_chest", Bool(*o.fBonusChestEnabled));
      }
      worldGenSettings->set("generate_features", Bool(true));
      if (o.fRandomSeed) {
        worldGenSettings->set("seed", Long(*o.fRandomSeed));
        auto dimensions = Compound();
        {
          auto overworld = Compound();
          auto generator = Compound();
          if (o.fFlatWorldSettings) {
            generator->set("type", String("minecraft:flat"));
            generator->set("settings", o.fFlatWorldSettings);
          } else {
            auto biomeSource = Compound();
            biomeSource->set("preset", String("minecraft:overworld"));
            biomeSource->set("type", String("minecraft:multi_noise"));
            generator->set("biome_source", biomeSource);
            generator->set("settings", String("minecraft:overworld"));
            generator->set("type", String("minecraft:noise"));
          }
          overworld->set("generator", generator);
          overworld->set("type", String("minecraft:overworld"));
          dimensions->set("minecraft:overworld", overworld);
        }
        {
          auto end = Compound();
          auto generator = Compound();
          auto biomeSource = Compound();
          biomeSource->set("type", String("minecraft:the_end"));
          generator->set("biome_source", biomeSource);
          generator->set("settings", String("minecraft:end"));
          generator->set("type", String("minecraft:noise"));
          end->set("generator", generator);
          end->set("type", String("minecraft:the_end"));
          dimensions->set("minecraft:the_end", end);
        }
        {
          auto nether = Compound();
          auto generator = Compound();
          auto biomeSource = Compound();
          biomeSource->set("preset", String("minecraft:nether"));
          biomeSource->set("type", String("minecraft:multi_noise"));
          generator->set("biome_source", biomeSource);
          generator->set("settings", String("minecraft:nether"));
          generator->set("type", String("minecraft:noise"));
          nether->set("generator", generator);
          nether->set("type", String("minecraft:the_nether"));
          dimensions->set("minecraft:the_nether", nether);
        }
        worldGenSettings->set("dimensions", dimensions);
      }
      j["WorldGenSettings"] = worldGenSettings;
    }

    return data;
  }
};

} // namespace je2be
