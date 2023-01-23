#include "box360/_block-data.hpp"

#include <je2be/integers.hpp>

namespace je2be::box360 {

static void StrippedLog(u8 data, std::map<std::string, std::string> &props) {
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

static std::string CoralWallFan(std::string name, u8 data, std::map<std::string, std::string> &props) {
  switch ((data & 0xc) >> 2) {
  case 1:
    props["facing"] = "east";
    break;
  case 2:
    props["facing"] = "north";
    break;
  case 3:
    props["facing"] = "south";
    break;
  case 0:
  default:
    props["facing"] = "west";
    break;
  }
  if ((data & 0x1) == 0x1) {
    name = "dead_" + name;
  }
  props["waterlogged"] = "false";
  return name;
}

std::shared_ptr<mcfile::je::Block const> BlockData::unsafeToBlock() const {
  using namespace std;
  u8 rawId = rawBlockId();
  u16 id = extendedBlockId();
  u8 data = this->data();
  if (rawId == id) {
    return mcfile::je::Flatten::DoFlatten(id, data);
  }
  string name;
  map<string, string> props;
  switch (id) {
  case 256:
    name = "conduit";
    props["waterlogged"] = "false";
    break;
  case 257:
    name = "pumpkin";
    break;
  case 258:
    // kelp_plant: stem
    // kelp: tip
    name = "kelp_plant";
    break;
  case 259:
    switch (data & 0x7) {
    case 1:
      name = "brain_coral_block";
      break;
    case 2:
      name = "bubble_coral_block";
      break;
    case 3:
      name = "fire_coral_block";
      break;
    case 4:
      name = "horn_coral_block";
      break;
    case 0:
    default:
      name = "tube_coral_block";
      break;
    }
    if ((data & 0x8) == 0x8) {
      name = "dead_" + name;
    }
    break;
  case 263:
    switch (data) {
    case 1:
      name = "brain_coral";
      break;
    case 2:
      name = "bubble_coral";
      break;
    case 3:
      name = "fire_coral";
      break;
    case 4:
      name = "horn_coral";
      break;
    case 0:
    default:
      name = "tube_coral";
      break;
    }
    break;
  case 264:
    switch (data & 0x7) {
    case 1:
      name = "brain_coral_fan";
      break;
    case 2:
      name = "bubble_coral_fan";
      break;
    case 3:
      name = "fire_coral_fan";
      break;
    case 4:
      name = "horn_coral_fan";
      break;
    case 0:
    default:
      name = "tube_coral_fan";
      break;
    }
    props["waterlogged"] = "false";
    break;
  case 265:
    switch (data & 0x7) {
    case 1:
      name = "dead_brain_coral_fan";
      break;
    case 2:
      name = "dead_bubble_coral_fan";
      break;
    case 3:
      name = "dead_fire_coral_fan";
      break;
    case 4:
      name = "dead_horn_coral_fan";
      break;
    case 0:
    default:
      name = "dead_tube_coral_fan";
      break;
    }
    props["waterlogged"] = "false";
    break;
  case 266:
    if ((data & 0x2) == 0x2) {
      name = "brain_coral_wall_fan";
    } else {
      name = "tube_coral_wall_fan";
    }
    name = CoralWallFan(name, data, props);
    break;
  case 267:
    if ((data & 0x2) == 0x2) {
      name = "fire_coral_wall_fan";
    } else {
      name = "bubble_coral_wall_fan";
    }
    name = CoralWallFan(name, data, props);
    break;
  case 268:
    name = CoralWallFan("horn_coral_wall_fan", data, props);
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
  case 271:
    name = "sea_pickle";
    props["waterlogged"] = (data & 0x8) == 0x8 ? "false" : "true";
    props["pickles"] = to_string((data & 0x7) + 1);
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
  case 279:
    name = "turtle_egg";
    props["eggs"] = std::to_string((data & 0x3) + 1);
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
  case 301:
    name = "acacia_pressure_plate";
    break;
  case 302:
    name = "birch_pressure_plate";
    break;
  case 303:
    name = "dark_oak_pressure_plate";
    break;
  case 304:
    name = "jungle_pressure_plate";
    break;
  case 305:
    name = "spruce_pressure_plate";
    break;
  case 306:
    name = "acacia_button";
    mcfile::je::Flatten::Button(data, props);
    break;
  case 307:
    name = "birch_button";
    mcfile::je::Flatten::Button(data, props);
    break;
  case 308:
    name = "dark_oak_button";
    mcfile::je::Flatten::Button(data, props);
    break;
  case 309:
    name = "jungle_button";
    mcfile::je::Flatten::Button(data, props);
    break;
  case 310:
    name = "spruce_button";
    mcfile::je::Flatten::Button(data, props);
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

} // namespace je2be::box360
