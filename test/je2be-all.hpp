#pragma once

#undef small

// clang-format off

#include <je2be.hpp>

#include <je2be/nbt.hpp>
#include "_version.hpp"
#include "_mem.hpp"

#include "toje/_constants.hpp"
#include "tobe/_versions.hpp"

#include "_nullable.hpp"
#include "_file.hpp"
#include "_pos3.hpp"
#include "_volume.hpp"
#include "_optional.hpp"
#include "_rotation.hpp"
#include "color/_rgba.hpp"
#include "color/_lab.hpp"
#include "_size.hpp"
#include "_xxhash.hpp"
#include "_props.hpp"
#include "_future-support.hpp"
#include "_parallel.hpp"
#include "_rename-pair.hpp"
#include "_nbt-ext.hpp"
#include "_reversible-map.hpp"
#include "_static-reversible-map.hpp"
#include "_dimension-ext.hpp"
#include "_java-level-dat.hpp"
#include "_poi-blocks.hpp"
#include "_data3d.hpp"

#include "enums/_banner-color-code-bedrock.hpp"
#include "enums/_color-code-java.hpp"
#include "enums/_villager-profession.hpp"
#include "enums/_villager-type.hpp"
#include "enums/_red-flower.hpp"
#include "enums/_skull-type.hpp"
#include "enums/_facing6.hpp"
#include "enums/_facing4.hpp"
#include "enums/_game-mode.hpp"
#include "enums/_chunk-conversion-mode.hpp"

#include "color/_sign-color.hpp"

#include "item/_potion.hpp"
#include "item/_enchantments.hpp"
#include "item/_fireworks-explosion.hpp"
#include "item/_fireworks.hpp"
#include "item/_banner.hpp"
#include "item/_tipped-arrow-potion.hpp"
#include "item/_map-color.hpp"
#include "item/_goat-horn.hpp"

#include "entity/_painting.hpp"
#include "entity/_boat.hpp"
#include "entity/_armor-stand.hpp"
#include "entity/_entity-attributes.hpp"
#include "entity/_tropical-fish.hpp"
#include "entity/_cat.hpp"
#include "entity/_panda.hpp"
#include "entity/_effect.hpp"
#include "entity/_axolotl.hpp"
#include "entity/_frog.hpp"

#include "command/_token.hpp"
#include "command/_target-selector.hpp"
#include "command/_command.hpp"

#include "tile-entity/_loot-table.hpp"

#include "structure/_structure-piece.hpp"

#include "terraform/_block-property-accessor.hpp"
#include "terraform/_block-accessor.hpp"
#include "terraform/bedrock/_block-accessor-bedrock.hpp"
#include "terraform/box360/_block-accessor-box360.hpp"
#include "terraform/_shape-of-stairs.hpp"
#include "terraform/_fence-connectable.hpp"
#include "terraform/_redstone-wire.hpp"
#include "terraform/_chorus-plant.hpp"
#include "terraform/_wall-connectable.hpp"
#include "terraform/_snowy.hpp"
#include "terraform/_leaves.hpp"
#include "terraform/_note-block.hpp"
#include "terraform/bedrock/_kelp.hpp"
#include "terraform/bedrock/_attached-stem.hpp"
#include "terraform/box360/_kelp.hpp"
#include "terraform/box360/_attached-stem.hpp"
#include "terraform/box360/_chest.hpp"
#include "terraform/java/_block-accessor-java-directory.hpp"
#include "terraform/java/_block-accessor-java-mca.hpp"

#include "db/_db-interface.hpp"
#include "db/_db.hpp"
#include "db/_null-db.hpp"
#include "db/_concurrent-db.hpp"

#include "tobe/_block-data.hpp"
#include "toje/_block-data.hpp"
#include "toje/terraform/_lighting.hpp"
