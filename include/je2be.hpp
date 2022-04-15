#pragma once

// clang-format off

#include <minecraft-file.hpp>

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <leveldb/comparator.h>
#include <leveldb/table_builder.h>
#include <leveldb/env.h>
#include <leveldb/table.h>
#include <table/compression/compressor_factory.h>
#include <table/block_builder.h>
#include <table/filter_block.h>
#include <table/format.h>
#include <db/dbformat.h>
#include <db/version_edit.h>
#include <db/log_writer.h>
#include <util/crc32c.h>

#include <xxhash32.h>
#include <xxhash64.h>
#include <hwm/task/task_queue.hpp>
#include <nlohmann/json.hpp>

#include <mz.h>
#include <mz_zip.h>
#include <mz_strm.h>
#include <mz_strm_os.h>
#include <mz_os.h>

#if __has_include(<windows.h>)
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#undef small
#include <io.h>
#endif

#if __has_include(<fcntl.h>)
#include <fcntl.h>
#endif

#include <set>
#include <unordered_set>
#include <execution>
#include <random>
#include <iostream>
#include <numbers>
#include <charconv>
#include <regex>
#include <fstream>
#include <variant>

#include <je2be/status.hpp>
#include <je2be/nullable.hpp>
#include <je2be/nbt.hpp>
#include <je2be/fs.hpp>
#include <je2be/scoped-file.hpp>
#include <je2be/file.hpp>
#include <je2be/strings.hpp>
#include <je2be/pos2.hpp>
#include <je2be/pos3.hpp>
#include <je2be/volume.hpp>
#include <je2be/optional.hpp>
#include <je2be/rotation.hpp>
#include <je2be/color/rgba.hpp>
#include <je2be/color/lab.hpp>
#include <je2be/size.hpp>
#include <je2be/xxhash.hpp>
#include <je2be/uuid.hpp>
#include <je2be/props.hpp>
#include <je2be/version.hpp>
#include <je2be/future-support.hpp>
#include <je2be/defer.hpp>
#include <je2be/rename-pair.hpp>
#include <je2be/nbt-ext.hpp>
#include <je2be/reversible-map.hpp>
#include <je2be/static-reversible-map.hpp>
#include <je2be/dimension-ext.hpp>
#include <je2be/java-level-dat.hpp>
#include <je2be/zip-file.hpp>

#include <je2be/enums/banner-color-code-bedrock.hpp>
#include <je2be/enums/color-code-java.hpp>
#include <je2be/enums/level-directory-structure.hpp>
#include <je2be/enums/villager-profession.hpp>
#include <je2be/enums/villager-type.hpp>
#include <je2be/enums/red-flower.hpp>
#include <je2be/enums/skull-type.hpp>
#include <je2be/enums/facing6.hpp>
#include <je2be/enums/facing4.hpp>

#include <je2be/color/sign-color.hpp>

#include <je2be/item/potion.hpp>
#include <je2be/item/enchantments.hpp>
#include <je2be/item/fireworks-explosion.hpp>
#include <je2be/item/fireworks.hpp>
#include <je2be/item/banner.hpp>
#include <je2be/item/tipped-arrow-potion.hpp>
#include <je2be/item/map-color.hpp>

#include <je2be/entity/painting.hpp>
#include <je2be/entity/boat.hpp>
#include <je2be/entity/armor-stand.hpp>
#include <je2be/entity/entity-attributes.hpp>
#include <je2be/entity/tropical-fish.hpp>
#include <je2be/entity/cat.hpp>
#include <je2be/entity/panda.hpp>
#include <je2be/entity/effect.hpp>
#include <je2be/entity/axolotl.hpp>

#include <je2be/command/token.hpp>
#include <je2be/command/target-selector.hpp>
#include <je2be/command/command.hpp>

#include <je2be/tile-entity/loot-table.hpp>

#include <je2be/structure/structure-piece.hpp>

#include <je2be/terraform/block-property-accessor.hpp>
#include <je2be/terraform/block-accessor.hpp>
#include <je2be/terraform/bedrock/block-accessor-bedrock.hpp>
#include <je2be/terraform/box360/block-accessor-box360.hpp>
#include <je2be/terraform/shape-of-stairs.hpp>
#include <je2be/terraform/fence-connectable.hpp>
#include <je2be/terraform/redstone-wire.hpp>
#include <je2be/terraform/chorus-plant.hpp>
#include <je2be/terraform/wall-connectable.hpp>
#include <je2be/terraform/snowy.hpp>
#include <je2be/terraform/leaves.hpp>
#include <je2be/terraform/note-block.hpp>
#include <je2be/terraform/bedrock/kelp.hpp>
#include <je2be/terraform/bedrock/attached-stem.hpp>
#include <je2be/terraform/box360/kelp.hpp>
#include <je2be/terraform/box360/attached-stem.hpp>
#include <je2be/terraform/box360/chest.hpp>

#include <je2be/box360/box360.hpp>
#include <je2be/tobe/tobe.hpp>
#include <je2be/toje/toje.hpp>
