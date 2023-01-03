#pragma once

#undef small

// clang-format off

#include <je2be.hpp>

#include <je2be/nbt.hpp>
#include <je2be/_version.hpp>
#include <je2be/_mem.hpp>

#include <je2be/toje/_constants.hpp>
#include <je2be/tobe/_versions.hpp>

#include <je2be/status.hpp>
#include <je2be/_nullable.hpp>
#include <je2be/_fs.hpp>
#include <je2be/_file.hpp>
#include <je2be/strings.hpp>
#include <je2be/pos2.hpp>
#include <je2be/_pos3.hpp>
#include <je2be/_volume.hpp>
#include <je2be/_optional.hpp>
#include <je2be/_rotation.hpp>
#include <je2be/color/_rgba.hpp>
#include <je2be/color/_lab.hpp>
#include <je2be/_size.hpp>
#include <je2be/_xxhash.hpp>
#include <je2be/uuid.hpp>
#include <je2be/_props.hpp>
#include <je2be/_future-support.hpp>
#include <je2be/_parallel.hpp>
#include <je2be/defer.hpp>
#include <je2be/_rename-pair.hpp>
#include <je2be/_nbt-ext.hpp>
#include <je2be/_reversible-map.hpp>
#include <je2be/_static-reversible-map.hpp>
#include <je2be/_dimension-ext.hpp>
#include <je2be/_java-level-dat.hpp>
#include <je2be/_zip-file.hpp>
#include <je2be/_poi-blocks.hpp>
#include <je2be/_data3d.hpp>

#include <je2be/enums/_banner-color-code-bedrock.hpp>
#include <je2be/enums/_color-code-java.hpp>
#include <je2be/enums/level-directory-structure.hpp>
#include <je2be/enums/_villager-profession.hpp>
#include <je2be/enums/_villager-type.hpp>
#include <je2be/enums/_red-flower.hpp>
#include <je2be/enums/_skull-type.hpp>
#include <je2be/enums/_facing6.hpp>
#include <je2be/enums/_facing4.hpp>
#include <je2be/enums/_game-mode.hpp>
#include <je2be/enums/_chunk-conversion-mode.hpp>

#include <je2be/color/_sign-color.hpp>

#include <je2be/item/_potion.hpp>
#include <je2be/item/_enchantments.hpp>
#include <je2be/item/_fireworks-explosion.hpp>
#include <je2be/item/_fireworks.hpp>
#include <je2be/item/_banner.hpp>
#include <je2be/item/_tipped-arrow-potion.hpp>
#include <je2be/item/_map-color.hpp>
#include <je2be/item/_goat-horn.hpp>

#include <je2be/entity/_painting.hpp>
#include <je2be/entity/_boat.hpp>
#include <je2be/entity/_armor-stand.hpp>
#include <je2be/entity/_entity-attributes.hpp>
#include <je2be/entity/_tropical-fish.hpp>
#include <je2be/entity/_cat.hpp>
#include <je2be/entity/_panda.hpp>
#include <je2be/entity/_effect.hpp>
#include <je2be/entity/_axolotl.hpp>
#include <je2be/entity/_frog.hpp>

#include <je2be/command/_token.hpp>
#include <je2be/command/_target-selector.hpp>
#include <je2be/command/_command.hpp>

#include <je2be/tile-entity/_loot-table.hpp>

#include <je2be/structure/_structure-piece.hpp>

#include <je2be/terraform/_block-property-accessor.hpp>
#include <je2be/terraform/_block-accessor.hpp>
#include <je2be/terraform/bedrock/_block-accessor-bedrock.hpp>
#include <je2be/terraform/box360/_block-accessor-box360.hpp>
#include <je2be/terraform/_shape-of-stairs.hpp>
#include <je2be/terraform/_fence-connectable.hpp>
#include <je2be/terraform/_redstone-wire.hpp>
#include <je2be/terraform/_chorus-plant.hpp>
#include <je2be/terraform/_wall-connectable.hpp>
#include <je2be/terraform/_snowy.hpp>
#include <je2be/terraform/_leaves.hpp>
#include <je2be/terraform/_note-block.hpp>
#include <je2be/terraform/bedrock/_kelp.hpp>
#include <je2be/terraform/bedrock/_attached-stem.hpp>
#include <je2be/terraform/box360/_kelp.hpp>
#include <je2be/terraform/box360/_attached-stem.hpp>
#include <je2be/terraform/box360/_chest.hpp>

#include <je2be/db/_db-interface.hpp>
#include <je2be/db/_db.hpp>
#include <je2be/db/_null-db.hpp>
#include <je2be/db/_raw-db.hpp>

#include <je2be/tobe/_block-data.hpp>
#include <je2be/toje/_block-data.hpp>
