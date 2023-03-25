#include "box360/_block-data.hpp"

#include <je2be/integers.hpp>

namespace je2be::box360 {

static void StrippedLog(u8 data, std::map<std::u8string, std::u8string> &props) {
  switch (data) {
  case 0x4:
    props[u8"axis"] = u8"x";
    break;
  case 0x8:
    props[u8"axis"] = u8"z";
    break;
  case 0x0:
  default:
    props[u8"axis"] = u8"y";
    break;
  }
}

static std::u8string CoralWallFan(std::u8string name, u8 data, std::map<std::u8string, std::u8string> &props) {
  switch ((data & 0xc) >> 2) {
  case 1:
    props[u8"facing"] = u8"east";
    break;
  case 2:
    props[u8"facing"] = u8"north";
    break;
  case 3:
    props[u8"facing"] = u8"south";
    break;
  case 0:
  default:
    props[u8"facing"] = u8"west";
    break;
  }
  if ((data & 0x1) == 0x1) {
    name = u8"dead_" + name;
  }
  props[u8"waterlogged"] = u8"false";
  return name;
}

std::shared_ptr<mcfile::je::Block const> BlockData::unsafeToBlock() const {
  using namespace std;
  u8 rawId = fRawId;
  u16 id = this->id();
  u8 data = this->data();
  if (rawId == id) {
    return mcfile::je::Flatten::Block(id, data);
  }
  u8string name;
  map<u8string, u8string> props;
  switch (id) {
  case 256:
    name = u8"conduit";
    props[u8"waterlogged"] = u8"false";
    break;
  case 257:
    name = u8"pumpkin";
    break;
  case 258:
    // kelp_plant: stem
    // kelp: tip
    name = u8"kelp_plant";
    break;
  case 259:
    switch (data & 0x7) {
    case 1:
      name = u8"brain_coral_block";
      break;
    case 2:
      name = u8"bubble_coral_block";
      break;
    case 3:
      name = u8"fire_coral_block";
      break;
    case 4:
      name = u8"horn_coral_block";
      break;
    case 0:
    default:
      name = u8"tube_coral_block";
      break;
    }
    if ((data & 0x8) == 0x8) {
      name = u8"dead_" + name;
    }
    break;
  case 263:
    switch (data) {
    case 1:
      name = u8"brain_coral";
      break;
    case 2:
      name = u8"bubble_coral";
      break;
    case 3:
      name = u8"fire_coral";
      break;
    case 4:
      name = u8"horn_coral";
      break;
    case 0:
    default:
      name = u8"tube_coral";
      break;
    }
    break;
  case 264:
    switch (data & 0x7) {
    case 1:
      name = u8"brain_coral_fan";
      break;
    case 2:
      name = u8"bubble_coral_fan";
      break;
    case 3:
      name = u8"fire_coral_fan";
      break;
    case 4:
      name = u8"horn_coral_fan";
      break;
    case 0:
    default:
      name = u8"tube_coral_fan";
      break;
    }
    props[u8"waterlogged"] = u8"false";
    break;
  case 265:
    switch (data & 0x7) {
    case 1:
      name = u8"dead_brain_coral_fan";
      break;
    case 2:
      name = u8"dead_bubble_coral_fan";
      break;
    case 3:
      name = u8"dead_fire_coral_fan";
      break;
    case 4:
      name = u8"dead_horn_coral_fan";
      break;
    case 0:
    default:
      name = u8"dead_tube_coral_fan";
      break;
    }
    props[u8"waterlogged"] = u8"false";
    break;
  case 266:
    if ((data & 0x2) == 0x2) {
      name = u8"brain_coral_wall_fan";
    } else {
      name = u8"tube_coral_wall_fan";
    }
    name = CoralWallFan(name, data, props);
    break;
  case 267:
    if ((data & 0x2) == 0x2) {
      name = u8"fire_coral_wall_fan";
    } else {
      name = u8"bubble_coral_wall_fan";
    }
    name = CoralWallFan(name, data, props);
    break;
  case 268:
    name = CoralWallFan(u8"horn_coral_wall_fan", data, props);
    break;
  case 269:
    switch (data) {
    case 0x6:
      name = u8"dried_kelp_block";
      break;
    }
    break;
  case 270:
    switch (data) {
    case 0:
      name = u8"seagrass";
      break;
    case 1:
      name = u8"tall_seagrass";
      props[u8"half"] = u8"upper";
      break;
    case 2:
      name = u8"tall_seagrass";
      props[u8"half"] = u8"lower";
      break;
    }
    break;
  case 271:
    name = u8"sea_pickle";
    props[u8"waterlogged"] = (data & 0x8) == 0x8 ? u8"false" : u8"true";
    props[u8"pickles"] = mcfile::String::ToString((data & 0x7) + 1);
    break;
  case 272:
    switch (data) {
    case 1:
      name = u8"bubble_column";
      break;
    }
    break;
  case 273:
    name = u8"blue_ice";
    break;
  case 274:
    name = u8"spruce_trapdoor";
    mcfile::je::Flatten::Trapdoor(data, props);
    break;
  case 275:
    name = u8"birch_trapdoor";
    mcfile::je::Flatten::Trapdoor(data, props);
    break;
  case 276:
    name = u8"jungle_trapdoor";
    mcfile::je::Flatten::Trapdoor(data, props);
    break;
  case 277:
    name = u8"acacia_trapdoor";
    mcfile::je::Flatten::Trapdoor(data, props);
    break;
  case 278:
    name = u8"dark_oak_trapdoor";
    mcfile::je::Flatten::Trapdoor(data, props);
    break;
  case 279:
    name = u8"turtle_egg";
    props[u8"eggs"] = mcfile::String::ToString((data & 0x3) + 1);
    break;
  case 291:
    name = u8"prismarine_stairs";
    mcfile::je::Flatten::Stairs(data, props);
    break;
  case 292:
    name = u8"prismarine_brick_stairs";
    mcfile::je::Flatten::Stairs(data, props);
    break;
  case 293:
    name = u8"dark_prismarine_stairs";
    mcfile::je::Flatten::Stairs(data, props);
    break;
  case 295:
    name = u8"stripped_spruce_log";
    StrippedLog(data, props);
    break;
  case 296:
    name = u8"stripped_birch_log";
    StrippedLog(data, props);
    break;
  case 297:
    name = u8"stripped_jungle_log";
    StrippedLog(data, props);
    break;
  case 298:
    name = u8"stripped_acacia_log";
    StrippedLog(data, props);
    break;
  case 299:
    name = u8"stripped_dark_oak_log";
    StrippedLog(data, props);
    break;
  case 300:
    name = u8"stripped_oak_log";
    StrippedLog(data, props);
    break;
  case 301:
    name = u8"acacia_pressure_plate";
    break;
  case 302:
    name = u8"birch_pressure_plate";
    break;
  case 303:
    name = u8"dark_oak_pressure_plate";
    break;
  case 304:
    name = u8"jungle_pressure_plate";
    break;
  case 305:
    name = u8"spruce_pressure_plate";
    break;
  case 306:
    name = u8"acacia_button";
    mcfile::je::Flatten::Button(data, props);
    break;
  case 307:
    name = u8"birch_button";
    mcfile::je::Flatten::Button(data, props);
    break;
  case 308:
    name = u8"dark_oak_button";
    mcfile::je::Flatten::Button(data, props);
    break;
  case 309:
    name = u8"jungle_button";
    mcfile::je::Flatten::Button(data, props);
    break;
  case 310:
    name = u8"spruce_button";
    mcfile::je::Flatten::Button(data, props);
    break;
  case 311:
    switch (data & 0x7) {
    case 1:
      name = u8"prismarine_brick_slab";
      break;
    case 2:
      name = u8"dark_prismarine_slab";
      break;
    case 0:
    default:
      name = u8"prismarine_slab";
      break;
    }
    props[u8"type"] = u8"double";
    break;
  case 312:
    switch (data & 0x7) {
    case 1:
      name = u8"prismarine_brick_slab";
      break;
    case 2:
      name = u8"dark_prismarine_slab";
      break;
    case 0:
    default:
      name = u8"prismarine_slab";
      break;
    }
    props[u8"type"] = (data & 0x8) == 0x8 ? u8"bottom" : u8"top";
    break;
  }
  if (name.empty()) {
    auto b = mcfile::je::Flatten::Block(id, data);
    return b;
  } else {
    return make_shared<mcfile::je::Block const>(u8"minecraft:" + name, props);
  }
}

} // namespace je2be::box360
