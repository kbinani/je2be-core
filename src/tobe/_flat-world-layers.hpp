#pragma once

#include <nlohmann/json.hpp>

#include <je2be/nbt.hpp>

#include "_props.hpp"
#include "tobe/_block-data.hpp"

namespace je2be::tobe {
class FlatWorldLayers {
  FlatWorldLayers() = delete;

public:
  static std::optional<std::u8string> FromLevelData(CompoundTag const &data) {
    using namespace std;
    auto compoundGeneratorOptions = data.compoundTag(u8"generatorOptions");
    auto worldGenSettings = data.compoundTag(u8"WorldGenSettings");
    auto stringGeneratorOptions = data.string(u8"generatorOptions");
    auto generatorName = data.string(u8"generatorName");
    auto version = data.int32(u8"DataVersion");
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
    if (generatorName == u8"flat") {
      return u8R"({"biome_id":1,"block_layers":[{"block_name":"minecraft:bedrock","count":1},{"block_name":"minecraft:dirt","count":2},{"block_name":"minecraft:grass","count":1}],"encoding_version":5,"structure_options":null})" + u8string(u8"\x0a");
    }
    return nullopt;
  }

  static std::optional<std::u8string> UsingStringGeneratorOptions(std::u8string const &generatorOptions, int dataVersion) {
    using namespace std;
    using namespace mcfile;

    auto idx = generatorOptions.find(u8';');
    if (idx == u8string::npos) {
      return nullopt;
    }
    u8string version;
    version.assign(generatorOptions.begin(), generatorOptions.begin() + idx);
    u8string trailing = generatorOptions.substr(idx + 1);
    if (version == u8"2") {
      return UsingStringGeneratorOptionsV2(trailing, dataVersion);
    } else if (version == u8"3") {
      return UsingStringGeneratorOptionsV3(trailing, dataVersion);
    }
    return nullopt;
  }

  static std::optional<std::u8string> UsingStringGeneratorOptionsV3(std::u8string const &options, int dataVersion) {
    // 3;minecraft:bedrock,230*minecraft:stone,5*minecraft:dirt,minecraft:grass;3;biome_1,decoration,stronghold,mineshaft,dungeon

    using namespace std;
    using namespace mcfile;

    auto idxBiomeStart = options.find(u8';');
    if (idxBiomeStart == u8string::npos) {
      return nullopt;
    }
    u8string biomeString;
    auto idxBiomeEnd = options.find(u8';', idxBiomeStart + 1);
    if (idxBiomeEnd == u8string::npos) {
      biomeString = options.substr(idxBiomeStart + 1);
    } else {
      biomeString.assign(options.begin() + idxBiomeStart + 1, options.begin() + idxBiomeEnd);
    }
    auto biomeInt = strings::ToI32(biomeString);
    if (!biomeInt) {
      return nullopt;
    }
    auto biome = mcfile::biomes::Biome::FromInt(*biomeInt);
    if (!biome) {
      return nullopt;
    }

    props::Json obj;
    obj["biome_id"] = mcfile::be::Biome::ToUint32(*biome);
    obj["block_layers"] = nlohmann::json::value_t::array;

    u8string layersString = options.substr(0, idxBiomeStart);

    for (u8string const &option : String::Split(layersString, u8',')) {
      auto x = option.find(u8'*');
      int count = 1;
      u8string blockString = option;
      if (x != u8string::npos) {
        u8string countString = option.substr(0, x);
        auto c = strings::ToI32(countString);
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
      props::Json item;
      props::SetJsonString(item, u8"block_name", *name);
      item["count"] = count;
      obj["block_layers"].push_back(item);
    }

    obj["encoding_version"] = 5;
    obj["structure_options"] = nlohmann::json::value_t::null;

    return props::StringFromJson(obj) + u8"\x0a";
  }

  static std::optional<std::u8string> UsingStringGeneratorOptionsV2(std::u8string const &options, int dataVersion) {
    // 2;7,2x3:2,2;1;village
    using namespace std;
    using namespace mcfile;
    auto idxBiomeStart = options.find(u8';');
    if (idxBiomeStart == u8string::npos) {
      return nullopt;
    }
    u8string biomeString;
    auto idxBiomeEnd = options.find(u8';', idxBiomeStart + 1);
    if (idxBiomeEnd == u8string::npos) {
      biomeString = options.substr(idxBiomeStart + 1);
    } else {
      biomeString.assign(options.begin() + idxBiomeStart + 1, options.begin() + idxBiomeEnd);
    }
    auto biomeInt = strings::ToI32(biomeString);
    if (!biomeInt) {
      return nullopt;
    }
    auto biome = mcfile::biomes::Biome::FromInt(*biomeInt);
    if (!biome) {
      return nullopt;
    }

    props::Json obj;
    obj["biome_id"] = mcfile::be::Biome::ToUint32(*biome);
    obj["block_layers"] = nlohmann::json::value_t::array;

    u8string layersString = options.substr(0, idxBiomeStart);
    for (u8string const &option : String::Split(layersString, u8',')) {
      int count = 1;
      auto x = option.find(u8'x');
      u8string blockString = option;
      if (x != u8string::npos) {
        u8string countString = option.substr(0, x);
        auto countInt = strings::ToI32(countString);
        if (!countInt) {
          return nullopt;
        }
        count = *countInt;
        blockString = option.substr(x + 1);
      }
      auto colon = blockString.find(':');
      int data = 0;
      if (colon != u8string::npos) {
        auto maybeData = strings::ToI32(blockString.substr(colon + 1));
        if (!maybeData) {
          return nullopt;
        }
        blockString = blockString.substr(0, colon);
        data = *maybeData;
      }
      auto blockId = strings::ToI32(blockString);
      if (!blockId) {
        return nullopt;
      }
      auto block = mcfile::je::Flatten::Block(*blockId, data);
      if (!block) {
        return nullopt;
      }
      auto converted = BlockData::From(block, nullptr);
      if (!converted) {
        return nullopt;
      }
      auto name = converted->string(u8"name");
      if (!name) {
        return nullopt;
      }
      props::Json item;
      props::SetJsonString(item, u8"block_name", *name);
      item["count"] = count;
      obj["block_layers"].push_back(item);
    }

    obj["encoding_version"] = 5;
    obj["structure_options"] = nlohmann::json::value_t::null;

    return props::StringFromJson(obj) + u8"\x0a";
  }

  static std::optional<std::u8string> UsingCompoundGeneratorOptions(CompoundTag const &generatorOptions, int dataVersion) {
    using namespace std;
    auto layers = generatorOptions.listTag(u8"layers");
    auto biomeString = generatorOptions.string(u8"biome");
    if (!layers || !biomeString) {
      return nullopt;
    }
    auto biome = mcfile::biomes::Biome::FromName(*biomeString);
    if (!biome) {
      return nullopt;
    }
    props::Json obj;
    obj["biome_id"] = mcfile::be::Biome::ToUint32(*biome);
    obj["block_layers"] = nlohmann::json::value_t::array;
    for (auto const &layer : *layers) {
      auto c = layer->asCompound();
      if (!c) {
        return nullopt;
      }
      auto blockString = c->string(u8"block");
      if (!blockString) {
        return nullopt;
      }
      int height = 1;
      auto heightByte = c->byte(u8"height");
      if (heightByte) {
        height = *heightByte;
      } else {
        auto heightInt16 = c->int16(u8"height");
        if (!heightInt16) {
          return nullopt;
        }
        height = *heightInt16;
      }
      auto name = BedrockBlockName(*blockString, dataVersion);
      if (!name) {
        return nullopt;
      }
      props::Json item;
      props::SetJsonString(item, u8"block_name", *name);
      item["count"] = height;
      obj["block_layers"].push_back(item);
    }
    obj["encoding_version"] = 5;
    obj["structure_options"] = nlohmann::json::value_t::null;

    return props::StringFromJson(obj) + u8"\x0a";
  }

  static std::optional<std::u8string> UsingWorldGenSettings(CompoundTag const &worldGenSettings, int dataVersion) {
    using namespace std;
    auto dimensions = worldGenSettings.compoundTag(u8"dimensions");
    if (!dimensions) {
      return nullopt;
    }
    auto overworld = dimensions->compoundTag(u8"minecraft:overworld");
    if (!overworld) {
      return nullopt;
    }
    auto type = overworld->string(u8"type");
    if (type != u8"minecraft:overworld") {
      return nullopt;
    }
    auto generator = overworld->compoundTag(u8"generator");
    if (!generator) {
      return nullopt;
    }
    auto generatorType = generator->string(u8"type");
    if (generatorType != u8"minecraft:flat") {
      return nullopt;
    }
    auto settings = generator->compoundTag(u8"settings");
    if (!settings) {
      return nullopt;
    }
    auto layers = settings->listTag(u8"layers");
    auto biomeString = settings->string(u8"biome");
    if (!layers || !biomeString) {
      return nullopt;
    }
    auto biome = mcfile::biomes::Biome::FromName(*biomeString);
    if (!biome) {
      return nullopt;
    }
    props::Json obj;
    obj["biome_id"] = mcfile::be::Biome::ToUint32(*biome);
    obj["block_layers"] = nlohmann::json::value_t::array;
    for (auto const &layer : *layers) {
      auto c = layer->asCompound();
      if (!c) {
        return nullopt;
      }
      auto blockString = c->string(u8"block");
      auto height = c->int32(u8"height");
      if (!blockString || !height) {
        return nullopt;
      }
      auto name = BedrockBlockName(*blockString, dataVersion);
      if (!name) {
        return nullopt;
      }
      props::Json item;
      props::SetJsonString(item, u8"block_name", *name);
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

    return props::StringFromJson(obj) + u8"\x0a";
  }

private:
  static std::optional<std::u8string> BedrockBlockName(std::u8string const &blockData, int dataVersion) {
    using namespace std;
    auto block = mcfile::je::Block::FromBlockData(blockData, dataVersion);
    if (!block) {
      return nullopt;
    }
    auto converted = BlockData::From(block, nullptr);
    if (!converted) {
      return nullopt;
    }
    auto name = converted->string(u8"name");
    if (!name) {
      return nullopt;
    }
    return *name;
  }
};
} // namespace je2be::tobe
