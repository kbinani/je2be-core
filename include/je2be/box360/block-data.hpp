#pragma once

namespace je2be::box360 {

struct BlockData : std::tuple<uint8_t, uint8_t> {
  BlockData(uint8_t blockId, uint8_t data) : std::tuple<uint8_t, uint8_t>(blockId, data) {}
  BlockData() : std::tuple<uint8_t, uint8_t>(0, 0) {}

  uint16_t blockId() const {
    return std::get<0>(*this);
  }

  uint8_t data() const {
    return std::get<1>(*this);
  };

  std::shared_ptr<mcfile::je::Block const> toBlock() const {
    using namespace std;
    auto p = unsafeToBlock();
    if (p) {
      if ((data() & 0x80) == 0x80) {
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

  std::shared_ptr<mcfile::je::Block const> unsafeToBlock() const {
    using namespace std;
    uint8_t id = blockId();
    uint8_t rawData = this->data();
    if ((rawData & 0x70) == 0) {
      return mcfile::je::Flatten::DoFlatten(id, 0xf & rawData);
    }
    string name;
    map<string, string> props;
    uint8_t data = rawData & 0x7f;
    switch (id) {
    case 2:
      switch (data) {
      case 0x10:
        name = "kelp_plant";
        break;
      }
      break;
    case 13:
      switch (data) {
      case 16:
        name = "dried_kelp_block";
        break;
      }
      break;
    case 14:
      switch (data) {
      case 0x10:
        name = "seagrass";
        break;
      case 0x11:
        name = "tall_seagrass";
        props["half"] = "upper";
        break;
      case 0x12:
        name = "tall_seagrass";
        props["half"] = "lower";
        break;
      }
      break;
    case 16:
      switch (data) {
      case 0x11:
        name = "bubble_column";
        break;
      }
      break;
    case 17:
      switch (data) {
      case 0x10:
        name = "blue_ice";
        break;
      }
      break;
    case 18:
      switch (data) {
      case 0x11:
        name = "spruce_trap_door";
        break;
      }
      break;
    case 39:
      switch (data) {
      case 0x10:
        name = "stripped_spruce_log";
        props["axis"] = "y";
        break;
      case 0x14:
        name = "stripped_spruce_log";
        props["axis"] = "x";
        break;
      case 0x18:
        name = "stripped_spruce_log";
        props["axis"] = "z";
        break;
      }
      break;
    case 40:
      switch (data) {
      case 0x10:
        name = "stripped_birch_log";
        props["axis"] = "y";
        break;
      case 0x14:
        name = "stripped_birch_log";
        props["axis"] = "x";
        break;
      case 0x18:
        name = "stripped_birch_log";
        props["axis"] = "z";
        break;
      }
      break;
    case 41:
      switch (data) {
      case 0x10:
        name = "stripped_jungle_log";
        props["axis"] = "y";
        break;
      case 0x14:
        name = "stripped_jungle_log";
        props["axis"] = "x";
        break;
      case 0x18:
        name = "stripped_jungle_log";
        props["axis"] = "z";
        break;
      }
      break;
    case 42:
      switch (data) {
      case 0x10:
        name = "stripped_acacia_log";
        props["axis"] = "y";
        break;
      case 0x14:
        name = "stripped_acacia_log";
        props["axis"] = "x";
        break;
      case 0x18:
        name = "stripped_acacia_log";
        props["axis"] = "z";
        break;
      }
      break;
    case 43:
      switch (data) {
      case 0x10:
        name = "stripped_dark_oak_log";
        props["axis"] = "y";
        break;
      case 0x14:
        name = "stripped_dark_oak_log";
        props["axis"] = "x";
        break;
      case 0x18:
        name = "stripped_dark_oak_log";
        props["axis"] = "z";
        break;
      }
      break;
    case 44:
      switch (data) {
      case 0x00:
        name = "smooth_stone_slab";
        props["type"] = "double";
        break;
      case 0x02:
        name = "smooth_stone_slab";
        props["type"] = "bottom";
        break;
      case 0x08:
        name = "smooth_stone_slab";
        props["type"] = "top";
        break;
      case 0x10:
        name = "stripped_oak_log";
        props["axis"] = "y";
        break;
      case 0x14:
        name = "stripped_oak_log";
        props["axis"] = "x";
        break;
      case 0x18:
        name = "stripped_oak_log";
        props["axis"] = "z";
        break;
      }
      break;
    }
    if (name.empty()) {
      return mcfile::je::Flatten::DoFlatten(id, 0xf & data);
    } else {
      return make_shared<mcfile::je::Block const>("minecraft:" + name, props);
    }
  }
};

} // namespace je2be::box360
