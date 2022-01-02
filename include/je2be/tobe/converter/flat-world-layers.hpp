#pragma once

namespace je2be::tobe {
class FlatWorldLayers {
  FlatWorldLayers() = delete;

public:
  static std::optional<std::string> FromLevelData(mcfile::nbt::CompoundTag const &data) {
    using namespace std;
    auto version = data.int32("version");
    if (!version) {
      return nullopt;
    }
    auto worldGenSettings = data.compoundTag("WorldGenSettings");
    if (!worldGenSettings) {
      return nullopt;
    }
    auto dimensions = worldGenSettings->compoundTag("dimensions");
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
    obj["block_layers"].array();
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
      nlohmann::json layer;
      layer["block_name"] = *name;
      layer["count"] = *height;
      obj["block_layers"].push_back(layer);
    }
    obj["encoding_version"] = 6;
    obj["structure_options"] = nlohmann::json::value_t::null;
    obj["world_version"] = "version.post_1_18";

    return nlohmann::to_string(obj) + "\x0a";
  }
};
} // namespace je2be::tobe
