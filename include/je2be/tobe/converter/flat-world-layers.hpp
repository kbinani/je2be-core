#pragma once

namespace je2be::tobe {
class FlatWorldLayers {
  FlatWorldLayers() = delete;

public:
  static std::optional<std::string> FromLevelData(mcfile::nbt::CompoundTag const &data) {
    using namespace std;
    auto generatorOptions = data.compoundTag("generatorOptions");
    auto worldGenSettings = data.compoundTag("WorldGenSettings");
    auto generatorName = data.string("generatorName");
    auto version = data.int32("DataVersion");
    if (worldGenSettings && version) {
      auto ret = UsingWorldGenSettings(*worldGenSettings, *version);
      if (ret) {
        return ret;
      }
    }
    if (generatorOptions) {
      auto ret = UsingGeneratorOptions(*generatorOptions);
      if (ret) {
        return ret;
      }
    }
    if (generatorName == "flat") {
      return R"({"biome_id":1,"block_layers":[{"block_name":"minecraft:bedrock","count":1},{"block_name":"minecraft:dirt","count":2},{"block_name":"minecraft:grass","count":1}],"encoding_version":5,"structure_options":null})" + string("\x0a");
    }
    return nullopt;
  }

  static std::optional<std::string> UsingGeneratorOptions(mcfile::nbt::CompoundTag const &generatorOptions) {
    using namespace std;
    auto layers = generatorOptions.listTag("layers");
    auto biomeString = generatorOptions.string("biome");
    if (!layers || !biomeString) {
      return nullopt;
    }
    auto biome = mcfile::biomes::FromName(*biomeString);
    if (!biome) {
      return nullopt;
    }
    nlohmann::json obj;
    obj["biome_id"] = mcfile::be::Biome::ToUint32(*biome);
    (void)obj["block_layers"].array();
    for (auto const &layer : *layers) {
      auto c = layer->asCompound();
      if (!c) {
        return nullopt;
      }
      auto blockString = c->string("block");
      auto height = c->byte("height");
      if (!blockString || !height) {
        return nullopt;
      }
      auto block = make_shared<mcfile::je::Block const>(*blockString);
      auto converted = BlockData::From(block);
      if (!converted) {
        return nullopt;
      }
      auto name = converted->string("name");
      if (!name) {
        return nullopt;
      }
      nlohmann::json item;
      item["block_name"] = *name;
      item["count"] = *height;
      obj["block_layers"].push_back(item);
    }
    obj["encoding_version"] = 5;
    obj["structure_options"] = nlohmann::json::value_t::null;

    return nlohmann::to_string(obj) + "\x0a";
  }

  static std::optional<std::string> UsingWorldGenSettings(mcfile::nbt::CompoundTag const &worldGenSettings, int dataVersion) {
    using namespace std;
    auto dimensions = worldGenSettings.compoundTag("dimensions");
    if (!dimensions) {
      return nullopt;
    }
    auto overworld = dimensions->compoundTag("minecraft:overworld");
    if (!overworld) {
      return nullopt;
    }
    auto type = overworld->string("type");
    if (type != "minecraft:overworld") {
      return nullopt;
    }
    auto generator = overworld->compoundTag("generator");
    if (!generator) {
      return nullopt;
    }
    auto generatorType = generator->string("type");
    if (generatorType != "minecraft:flat") {
      return nullopt;
    }
    auto settings = generator->compoundTag("settings");
    if (!settings) {
      return nullopt;
    }
    auto layers = settings->listTag("layers");
    auto biomeString = settings->string("biome");
    if (!layers || !biomeString) {
      return nullopt;
    }
    auto biome = mcfile::biomes::FromName(*biomeString);
    if (!biome) {
      return nullopt;
    }
    nlohmann::json obj;
    obj["biome_id"] = mcfile::be::Biome::ToUint32(*biome);
    (void)obj["block_layers"].array();
    for (auto const &layer : *layers) {
      auto c = layer->asCompound();
      if (!c) {
        return nullopt;
      }
      auto blockString = c->string("block");
      auto height = c->int32("height");
      if (!blockString || !height) {
        return nullopt;
      }
      auto block = make_shared<mcfile::je::Block const>(*blockString);
      auto converted = BlockData::From(block);
      if (!converted) {
        return nullopt;
      }
      auto name = converted->string("name");
      if (!name) {
        return nullopt;
      }
      nlohmann::json item;
      item["block_name"] = *name;
      item["count"] = *height;
      obj["block_layers"].push_back(item);
    }
    obj["structure_options"] = nlohmann::json::value_t::null;

    if (dataVersion < 2834) {
      obj["encoding_version"] = 5;
    } else {
      obj["encoding_version"] = 6;
      obj["world_version"] = "version.post_1_18";
    }

    return nlohmann::to_string(obj) + "\x0a";
  }
};
} // namespace je2be::tobe
