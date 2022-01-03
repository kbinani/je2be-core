#pragma once

namespace je2be::tobe {
class FlatWorldLayers {
  FlatWorldLayers() = delete;

public:
  static std::optional<std::string> FromLevelData(mcfile::nbt::CompoundTag const &data) {
    using namespace std;
    auto compoundGeneratorOptions = data.compoundTag("generatorOptions");
    auto worldGenSettings = data.compoundTag("WorldGenSettings");
    auto stringGeneratorOptions = data.string("generatorOptions");
    auto generatorName = data.string("generatorName");
    auto version = data.int32("DataVersion");
    if (worldGenSettings && version) {
      auto ret = UsingWorldGenSettings(*worldGenSettings, *version);
      if (ret) {
        return ret;
      }
    }
    if (compoundGeneratorOptions && version) {
      auto ret = UsingCompoundGeneratorOptions(*compoundGeneratorOptions, *version);
      if (ret) {
        return ret;
      }
    }
    if (stringGeneratorOptions) {
      auto ret = UsingStringGeneratorOptions(*stringGeneratorOptions, version ? *version : 100);
      if (ret) {
        return ret;
      }
    }
    if (generatorName == "flat") {
      return R"({"biome_id":1,"block_layers":[{"block_name":"minecraft:bedrock","count":1},{"block_name":"minecraft:dirt","count":2},{"block_name":"minecraft:grass","count":1}],"encoding_version":5,"structure_options":null})" + string("\x0a");
    }
    return nullopt;
  }

  static std::optional<std::string> UsingStringGeneratorOptions(std::string const &generatorOptions, int dataVersion) {
    using namespace std;
    using namespace mcfile;

    auto idx = generatorOptions.find(';');
    if (idx == string::npos) {
      return nullopt;
    }
    string version;
    version.assign(generatorOptions.begin(), generatorOptions.begin() + idx);
    string trailing = generatorOptions.substr(idx + 1);
    if (version == "2") {
      return UsingStringGeneratorOptionsV2(trailing, dataVersion);
    } else if (version == "3") {
      return UsingStringGeneratorOptionsV3(trailing, dataVersion);
    }
  }

  static std::optional<std::string> UsingStringGeneratorOptionsV3(std::string const &options, int dataVersion) {
    // 3;minecraft:bedrock,230*minecraft:stone,5*minecraft:dirt,minecraft:grass;3;biome_1,decoration,stronghold,mineshaft,dungeon

    using namespace std;
    using namespace mcfile;

    auto idxBiomeStart = options.find(';');
    if (idxBiomeStart == string::npos) {
      return nullopt;
    }
    string biomeString;
    auto idxBiomeEnd = options.find(';', idxBiomeStart + 1);
    if (idxBiomeEnd == string::npos) {
      biomeString = options.substr(idxBiomeStart + 1);
    } else {
      biomeString.assign(options.begin() + idxBiomeStart + 1, options.begin() + idxBiomeEnd);
    }
    auto biomeInt = strings::Toi(biomeString);
    if (!biomeInt) {
      return nullopt;
    }
    auto biome = mcfile::biomes::FromInt(*biomeInt);
    if (biome == biomes::unknown) {
      return nullopt;
    }

    nlohmann::json obj;
    obj["biome_id"] = mcfile::be::Biome::ToUint32(biome);
    obj["block_layers"] = nlohmann::json::value_t::array;

    string layersString = options.substr(0, idxBiomeStart);

    for (string const &option : String::Split(layersString, ',')) {
      auto x = option.find('*');
      int count = 1;
      string blockString = option;
      if (x != string::npos) {
        string countString = option.substr(0, x);
        auto c = strings::Toi(countString);
        if (!c) {
          return nullopt;
        }
        count = *c;
        blockString = option.substr(x + 1);
      }
      auto name = BedrockBlockName(blockString, dataVersion);
      if (!name) {
        return nullopt;
      }
      nlohmann::json item;
      item["block_name"] = *name;
      item["count"] = count;
      obj["block_layers"].push_back(item);
    }

    obj["encoding_version"] = 5;
    obj["structure_options"] = nlohmann::json::value_t::null;

    return nlohmann::to_string(obj) + "\x0a";
  }

  static std::optional<std::string> UsingStringGeneratorOptionsV2(std::string const &options, int dataVersion) {
    // 2;7,2x3:2,2;1;village
    using namespace std;
    using namespace mcfile;
    auto idxBiomeStart = options.find(';');
    if (idxBiomeStart == string::npos) {
      return nullopt;
    }
    string biomeString;
    auto idxBiomeEnd = options.find(';', idxBiomeStart + 1);
    if (idxBiomeEnd == string::npos) {
      biomeString = options.find(idxBiomeStart + 1);
    } else {
      biomeString.assign(options.begin() + idxBiomeStart + 1, options.begin() + idxBiomeEnd);
    }
    auto biomeInt = strings::Toi(biomeString);
    if (!biomeInt) {
      return nullopt;
    }
    auto biome = mcfile::biomes::FromInt(*biomeInt);
    if (biome == biomes::unknown) {
      return nullopt;
    }

    nlohmann::json obj;
    obj["biome_id"] = mcfile::be::Biome::ToUint32(biome);
    obj["block_layers"] = nlohmann::json::value_t::array;

    string layersString = options.substr(0, idxBiomeStart);
    for (string const &option : String::Split(layersString, ',')) {
      int count = 1;
      auto x = option.find('x');
      string blockString = option;
      if (x != string::npos) {
        string countString = option.substr(0, x);
        auto countInt = strings::Toi(countString);
        if (!countInt) {
          return nullopt;
        }
        count = *countInt;
        blockString = option.substr(x + 1);
      }
      auto colon = blockString.find(':');
      int data = 0;
      if (colon != string::npos) {
        auto maybeData = strings::Toi(blockString.substr(colon + 1));
        if (!maybeData) {
          return nullopt;
        }
        blockString = blockString.substr(0, colon);
        data = *maybeData;
      }
      auto blockId = strings::Toi(blockString);
      if (!blockId) {
        return nullopt;
      }
      auto block = mcfile::je::Flatten::DoFlatten(*blockId, data);
      if (!block) {
        return nullopt;
      }
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
      item["count"] = count;
      obj["block_layers"].push_back(item);
    }

    obj["encoding_version"] = 5;
    obj["structure_options"] = nlohmann::json::value_t::null;

    return nlohmann::to_string(obj) + "\x0a";
  }

  static std::optional<std::string> UsingCompoundGeneratorOptions(mcfile::nbt::CompoundTag const &generatorOptions, int dataVersion) {
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
    obj["block_layers"] = nlohmann::json::value_t::array;
    for (auto const &layer : *layers) {
      auto c = layer->asCompound();
      if (!c) {
        return nullopt;
      }
      auto blockString = c->string("block");
      if (!blockString) {
        return nullopt;
      }
      int height = 1;
      auto heightByte = c->byte("height");
      if (heightByte) {
        height = *heightByte;
      } else {
        auto heightInt16 = c->int16("height");
        if (!heightInt16) {
          return nullopt;
        }
        height = *heightInt16;
      }
      auto name = BedrockBlockName(*blockString, dataVersion);
      if (!name) {
        return nullopt;
      }
      nlohmann::json item;
      item["block_name"] = *name;
      item["count"] = height;
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
    obj["block_layers"] = nlohmann::json::value_t::array;
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
      auto name = BedrockBlockName(*blockString, dataVersion);
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

private:
  static std::optional<std::string> BedrockBlockName(std::string const &blockData, int dataVersion) {
    using namespace std;
    auto block = mcfile::je::Block::FromBlockData(blockData, dataVersion);
    if (!block) {
      return nullopt;
    }
    auto converted = BlockData::From(block);
    if (!converted) {
      return nullopt;
    }
    auto name = converted->string("name");
    if (!name) {
      return nullopt;
    }
    return *name;
  }
};
} // namespace je2be::tobe
