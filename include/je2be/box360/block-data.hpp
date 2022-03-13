#pragma once

namespace je2be::box360 {

struct BlockData : std::tuple<uint8_t, uint8_t> {
  BlockData(uint8_t rawBlockId, uint8_t data) : std::tuple<uint8_t, uint8_t>(rawBlockId, data) {}
  BlockData() : std::tuple<uint8_t, uint8_t>(0, 0) {}

  uint16_t rawBlockId() const {
    return std::get<0>(*this);
  }

  uint8_t rawData() const {
    return std::get<1>(*this);
  };

  uint16_t extendedBlockId() const {
    return (((uint16_t)rawData() & 0x70) << 4) | rawBlockId();
  }

  uint8_t data() const {
    return rawData() & 0xf;
  }

  bool isWaterlogged() const {
    return (rawData() & 0x80) == 0x80;
  }

  std::shared_ptr<mcfile::je::Block const> toBlock() const {
    using namespace std;
    auto p = unsafeToBlock();
    if (p) {
      if (isWaterlogged()) {
        map<string, string> props(p->fProperties);
        props["waterlogged"] = "true";
        return make_shared<mcfile::je::Block const>(p->fName, props);
      } else {
        return p;
      }
    } else {
      return make_shared<mcfile::je::Block const>("minecraft:air");
    }
  }

  static void StrippedLog(uint8_t data, std::map<std::string, std::string> &props) {
    switch (data) {
    case 0x4:
      props["axis"] = "x";
      break;
    case 0x8:
      props["axis"] = "z";
      break;
    case 0x0:
    default:
      props["axis"] = "y";
      break;
    }
  }

  std::shared_ptr<mcfile::je::Block const> unsafeToBlock() const {
    using namespace std;
    uint8_t rawId = rawBlockId();
    uint8_t rawData = this->rawData();
    uint16_t id = extendedBlockId();
    uint8_t data = this->data();
    if (rawId == id) {
      return mcfile::je::Flatten::DoFlatten(id, data);
    }
    string name;
    map<string, string> props;
    switch (id) {
    case 258:
      name = "kelp_plant";
      break;
    case 269:
      switch (data) {
      case 0x6:
        name = "dried_kelp_block";
        break;
      }
      break;
    case 270:
      switch (data) {
      case 0:
        name = "seagrass";
        break;
      case 1:
        name = "tall_seagrass";
        props["half"] = "upper";
        break;
      case 2:
        name = "tall_seagrass";
        props["half"] = "lower";
        break;
      }
      break;
    case 272:
      switch (data) {
      case 1:
        name = "bubble_column";
        break;
      }
      break;
    case 273:
      name = "blue_ice";
      break;
    case 274:
      name = "spruce_trapdoor";
      mcfile::je::Flatten::Trapdoor(data, props);
      break;
    case 275:
      name = "birch_trapdoor";
      mcfile::je::Flatten::Trapdoor(data, props);
      break;
    case 276:
      name = "jungle_trapdoor";
      mcfile::je::Flatten::Trapdoor(data, props);
      break;
    case 277:
      name = "acacia_trapdoor";
      mcfile::je::Flatten::Trapdoor(data, props);
      break;
    case 278:
      name = "dark_oak_trapdoor";
      mcfile::je::Flatten::Trapdoor(data, props);
      break;
    case 291:
      name = "prismarine_stairs";
      mcfile::je::Flatten::Stairs(data, props);
      break;
    case 292:
      name = "prismarine_brick_stairs";
      mcfile::je::Flatten::Stairs(data, props);
      break;
    case 293:
      name = "dark_prismarine_stairs";
      mcfile::je::Flatten::Stairs(data, props);
      break;
    case 295:
      name = "stripped_spruce_log";
      StrippedLog(data, props);
      break;
    case 296:
      name = "stripped_birch_log";
      StrippedLog(data, props);
      break;
    case 297:
      name = "stripped_jungle_log";
      StrippedLog(data, props);
      break;
    case 298:
      name = "stripped_acacia_log";
      StrippedLog(data, props);
      break;
    case 299:
      name = "stripped_dark_oak_log";
      StrippedLog(data, props);
      break;
    case 300:
      name = "stripped_oak_log";
      StrippedLog(data, props);
      break;
    case 311:
      switch (data & 0x7) {
      case 1:
        name = "prismarine_brick_slab";
        break;
      case 2:
        name = "dark_prismarine_slab";
        break;
      case 0:
      default:
        name = "prismarine_slab";
        break;
      }
      props["type"] = "double";
      break;
    case 312:
      switch (data & 0x7) {
      case 1:
        name = "prismarine_brick_slab";
        break;
      case 2:
        name = "dark_prismarine_slab";
        break;
      case 0:
      default:
        name = "prismarine_slab";
        break;
      }
      props["type"] = (data & 0x8) == 0x8 ? "bottom" : "top";
      break;
    }
    if (name.empty()) {
      auto b = mcfile::je::Flatten::DoFlatten(rawId, data);
      return b;
    } else {
      return make_shared<mcfile::je::Block const>("minecraft:" + name, props);
    }
  }
};

} // namespace je2be::box360
