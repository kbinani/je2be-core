cmake_minimum_required(VERSION 3.16)
project(je2be)

if ("${CMAKE_PROJECT_NAME}" STREQUAL "${PROJECT_NAME}")
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  if ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    add_definitions(
      /source-charset:utf-8
      /we4828 # Treat "The file contains a character that cannot be represented in the current code page" warnings as errors
      /wd4100 # Suppress "unreferenced formal parameter" warnings
    )
  endif()
endif()

set(je2be_files
  #begin je2be source files
  include/je2be.hpp
  include/je2be/color/lab.hpp
  include/je2be/color/rgba.hpp
  include/je2be/color/sign-color.hpp
  include/je2be/defer.hpp
  include/je2be/dimension-ext.hpp
  include/je2be/entity/armor-stand.hpp
  include/je2be/entity/axolotl.hpp
  include/je2be/entity/boat.hpp
  include/je2be/entity/cat.hpp
  include/je2be/entity/effect.hpp
  include/je2be/entity/entity-attributes.hpp
  include/je2be/entity/painting.hpp
  include/je2be/entity/panda.hpp
  include/je2be/entity/tropical-fish.hpp
  include/je2be/enums/banner-color-code-bedrock.hpp
  include/je2be/enums/color-code-java.hpp
  include/je2be/enums/facing4.hpp
  include/je2be/enums/facing6.hpp
  include/je2be/enums/level-directory-structure.hpp
  include/je2be/enums/red-flower.hpp
  include/je2be/enums/skull-type.hpp
  include/je2be/enums/villager-profession.hpp
  include/je2be/enums/villager-type.hpp
  include/je2be/file.hpp
  include/je2be/fs.hpp
  include/je2be/future-support.hpp
  include/je2be/item/banner.hpp
  include/je2be/item/enchantments.hpp
  include/je2be/item/fireworks-explosion.hpp
  include/je2be/item/fireworks.hpp
  include/je2be/item/map-color.hpp
  include/je2be/item/potion.hpp
  include/je2be/item/tipped-arrow-potion.hpp
  include/je2be/loot-table.hpp
  include/je2be/nbt-ext.hpp
  include/je2be/nbt.hpp
  include/je2be/optional.hpp
  include/je2be/pos2.hpp
  include/je2be/pos3.hpp
  include/je2be/props.hpp
  include/je2be/rename-pair.hpp
  include/je2be/reversible-map.hpp
  include/je2be/rotation.hpp
  include/je2be/scoped-file.hpp
  include/je2be/size.hpp
  include/je2be/static-reversible-map.hpp
  include/je2be/strings.hpp
  include/je2be/structure/structure-piece.hpp
  include/je2be/tobe/chunk-conversion-mode.hpp
  include/je2be/tobe/chunk-data-package.hpp
  include/je2be/tobe/chunk-data.hpp
  include/je2be/tobe/context.hpp
  include/je2be/tobe/converter/axolotl.hpp
  include/je2be/tobe/converter/biome-map-legacy.hpp
  include/je2be/tobe/converter/block-data.hpp
  include/je2be/tobe/converter/block-palette.hpp
  include/je2be/tobe/converter/chunk.hpp
  include/je2be/tobe/converter/command.hpp
  include/je2be/tobe/converter/converter.hpp
  include/je2be/tobe/converter/datapacks.hpp
  include/je2be/tobe/converter/enchant-data.hpp
  include/je2be/tobe/converter/entity.hpp
  include/je2be/tobe/converter/flat-world-layers.hpp
  include/je2be/tobe/converter/height-map.hpp
  include/je2be/tobe/converter/item.hpp
  include/je2be/tobe/converter/java-edition-map.hpp
  include/je2be/tobe/converter/level.hpp
  include/je2be/tobe/converter/map.hpp
  include/je2be/tobe/converter/moving-piston.hpp
  include/je2be/tobe/converter/player-abilities.hpp
  include/je2be/tobe/converter/sub-chunk.hpp
  include/je2be/tobe/converter/tile-entity.hpp
  include/je2be/tobe/converter/world.hpp
  include/je2be/tobe/db/db-interface.hpp
  include/je2be/tobe/db/db.hpp
  include/je2be/tobe/db/null-db.hpp
  include/je2be/tobe/db/raw-db.hpp
  include/je2be/tobe/level-data.hpp
  include/je2be/tobe/options.hpp
  include/je2be/tobe/portal/oriented-portal-blocks.hpp
  include/je2be/tobe/portal/portal-blocks.hpp
  include/je2be/tobe/portal/portal.hpp
  include/je2be/tobe/portal/portals.hpp
  include/je2be/tobe/progress.hpp
  include/je2be/tobe/session-lock.hpp
  include/je2be/tobe/statistics.hpp
  include/je2be/tobe/structure/structure-piece-collection.hpp
  include/je2be/tobe/structure/structures.hpp
  include/je2be/tobe/tobe.hpp
  include/je2be/tobe/uuid-registrar.hpp
  include/je2be/tobe/versions.hpp
  include/je2be/tobe/world-data.hpp
  include/je2be/toje/block-data.hpp
  include/je2be/toje/block-entity-convert-result.hpp
  include/je2be/toje/block-entity.hpp
  include/je2be/toje/block-property-accessor.hpp
  include/je2be/toje/chunk-cache.hpp
  include/je2be/toje/chunk.hpp
  include/je2be/toje/constants.hpp
  include/je2be/toje/context.hpp
  include/je2be/toje/converter.hpp
  include/je2be/toje/entity.hpp
  include/je2be/toje/item.hpp
  include/je2be/toje/level-data.hpp
  include/je2be/toje/map-info.hpp
  include/je2be/toje/options.hpp
  include/je2be/toje/progress.hpp
  include/je2be/toje/region.hpp
  include/je2be/toje/structure-info.hpp
  include/je2be/toje/sub-chunk.hpp
  include/je2be/toje/terraform/attached-stem.hpp
  include/je2be/toje/terraform/beacon.hpp
  include/je2be/toje/terraform/campfire.hpp
  include/je2be/toje/terraform/cave-vines.hpp
  include/je2be/toje/terraform/chorus-plant.hpp
  include/je2be/toje/terraform/fence-connectable.hpp
  include/je2be/toje/terraform/kelp.hpp
  include/je2be/toje/terraform/note-block.hpp
  include/je2be/toje/terraform/piston.hpp
  include/je2be/toje/terraform/redstone-wire.hpp
  include/je2be/toje/terraform/shape-of-stairs.hpp
  include/je2be/toje/terraform/snowy.hpp
  include/je2be/toje/terraform/tripwire.hpp
  include/je2be/toje/terraform/twisting-vines.hpp
  include/je2be/toje/terraform/weeping-vines.hpp
  include/je2be/toje/toje.hpp
  include/je2be/toje/world.hpp
  include/je2be/uuid.hpp
  include/je2be/version.hpp
  include/je2be/volume.hpp
  include/je2be/xxhash.hpp
  #end je2be source files
  # (cd deps/libminecraft-file; git ls-files | grep ^include | sed 's:\(.*\):  deps/libminecraft-file/\1:g') | pbcopy
  #begin libminecraft-file source files
  deps/libminecraft-file/include/mcfile/algorithm.hpp
  deps/libminecraft-file/include/mcfile/be/biome-map.hpp
  deps/libminecraft-file/include/mcfile/be/biome-section.hpp
  deps/libminecraft-file/include/mcfile/be/biome.hpp
  deps/libminecraft-file/include/mcfile/be/block.hpp
  deps/libminecraft-file/include/mcfile/be/chunk.hpp
  deps/libminecraft-file/include/mcfile/be/db-interface.hpp
  deps/libminecraft-file/include/mcfile/be/db-key.hpp
  deps/libminecraft-file/include/mcfile/be/db.hpp
  deps/libminecraft-file/include/mcfile/be/pending-tick.hpp
  deps/libminecraft-file/include/mcfile/be/sub-chunk.hpp
  deps/libminecraft-file/include/mcfile/be/wrap-db.hpp
  deps/libminecraft-file/include/mcfile/biome-palette.hpp
  deps/libminecraft-file/include/mcfile/biomes/biome-id.hpp
  deps/libminecraft-file/include/mcfile/biomes/from-int.hpp
  deps/libminecraft-file/include/mcfile/biomes/from-name.hpp
  deps/libminecraft-file/include/mcfile/biomes/minecraft.hpp
  deps/libminecraft-file/include/mcfile/biomes/name.hpp
  deps/libminecraft-file/include/mcfile/blocks/block-id.hpp
  deps/libminecraft-file/include/mcfile/blocks/from-name.hpp
  deps/libminecraft-file/include/mcfile/blocks/minecraft.hpp
  deps/libminecraft-file/include/mcfile/blocks/name.hpp
  deps/libminecraft-file/include/mcfile/compression.hpp
  deps/libminecraft-file/include/mcfile/coordinate.hpp
  deps/libminecraft-file/include/mcfile/dimension.hpp
  deps/libminecraft-file/include/mcfile/endianness.hpp
  deps/libminecraft-file/include/mcfile/file.hpp
  deps/libminecraft-file/include/mcfile/je/block-palette.hpp
  deps/libminecraft-file/include/mcfile/je/block.hpp
  deps/libminecraft-file/include/mcfile/je/cached-chunk-loader.hpp
  deps/libminecraft-file/include/mcfile/je/chunk-section.hpp
  deps/libminecraft-file/include/mcfile/je/chunk.hpp
  deps/libminecraft-file/include/mcfile/je/chunksection/block-states-parser-113.hpp
  deps/libminecraft-file/include/mcfile/je/chunksection/block-states-parser-116.hpp
  deps/libminecraft-file/include/mcfile/je/chunksection/chunk-section-112.hpp
  deps/libminecraft-file/include/mcfile/je/chunksection/chunk-section-113-base.hpp
  deps/libminecraft-file/include/mcfile/je/chunksection/chunk-section-113.hpp
  deps/libminecraft-file/include/mcfile/je/chunksection/chunk-section-116.hpp
  deps/libminecraft-file/include/mcfile/je/chunksection/chunk-section-118.hpp
  deps/libminecraft-file/include/mcfile/je/chunksection/chunk-section-generator.hpp
  deps/libminecraft-file/include/mcfile/je/flatten.hpp
  deps/libminecraft-file/include/mcfile/je/mca-data-source.hpp
  deps/libminecraft-file/include/mcfile/je/region.hpp
  deps/libminecraft-file/include/mcfile/je/set-block-options.hpp
  deps/libminecraft-file/include/mcfile/je/ticking-block.hpp
  deps/libminecraft-file/include/mcfile/je/world.hpp
  deps/libminecraft-file/include/mcfile/je/writable-chunk.hpp
  deps/libminecraft-file/include/mcfile/nbt/byte-array-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/byte-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/compound-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/detail/scalar-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/detail/vector-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/double-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/end-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/float-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/int-array-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/int-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/list-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/long-array-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/long-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/print-as-json.hpp
  deps/libminecraft-file/include/mcfile/nbt/short-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/string-tag.hpp
  deps/libminecraft-file/include/mcfile/nbt/tag-factory-impl.hpp
  deps/libminecraft-file/include/mcfile/nbt/tag-factory.hpp
  deps/libminecraft-file/include/mcfile/nbt/tag.hpp
  deps/libminecraft-file/include/mcfile/palette-list.hpp
  deps/libminecraft-file/include/mcfile/pos2.hpp
  deps/libminecraft-file/include/mcfile/pos3.hpp
  deps/libminecraft-file/include/mcfile/stream/byte-stream.hpp
  deps/libminecraft-file/include/mcfile/stream/file-input-stream.hpp
  deps/libminecraft-file/include/mcfile/stream/file-output-stream.hpp
  deps/libminecraft-file/include/mcfile/stream/gz-file-input-stream.hpp
  deps/libminecraft-file/include/mcfile/stream/gz-file-output-stream.hpp
  deps/libminecraft-file/include/mcfile/stream/input-stream-reader.hpp
  deps/libminecraft-file/include/mcfile/stream/input-stream.hpp
  deps/libminecraft-file/include/mcfile/stream/output-stream-writer.hpp
  deps/libminecraft-file/include/mcfile/stream/output-stream.hpp
  deps/libminecraft-file/include/mcfile/string.hpp
  deps/libminecraft-file/include/minecraft-file.hpp
  #end libminecraft-file source files
  .editorconfig
  .clang-format)
set(test_files
  test/main.cpp
  test/moving-piston.test.hpp
  test/command.test.hpp
  test/research.hpp
  test/block-data.test.hpp
  test/CheckTag.hpp
  test/j2b2j.test.hpp
  test/volume.test.hpp)

add_subdirectory(deps/libminecraft-file)

if (NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  list(APPEND je2be_link_libraries pthread)
endif()

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(/bigobj -D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
elseif(NOT "${EMSCRIPTEN}" STREQUAL "1")
  list(APPEND je2be_link_libraries tbb)
endif()

set(LEVELDB_ZSTD OFF)
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC" OR "${EMSCRIPTEN}" STREQUAL "1")
  set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/libminecraft-file/deps/zlib-ng")
  set(ZLIB_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/deps/libminecraft-file/deps/zlib-ng")
  include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/libminecraft-file/deps/zlib-ng"
    "${CMAKE_CURRENT_BINARY_DIR}/deps/libminecraft-file/deps/zlib-ng")
  link_directories("${CMAKE_CURRENT_BINARY_DIR}/deps/libminecraft-file/deps/zlib-ng")
endif()
add_subdirectory(deps/leveldb EXCLUDE_FROM_ALL)

list(APPEND je2be_link_libraries mcfile leveldb)


add_library(je2be INTERFACE ${je2be_files})
target_include_directories(je2be INTERFACE
  ./include
  deps/leveldb
  deps/leveldb/include
  deps/xxhash
  deps/hwm.task
  deps/json/include)
target_link_libraries(je2be INTERFACE ${je2be_link_libraries})

add_executable(j2b example/j2b.cpp ${je2be_files})
target_link_libraries(j2b je2be)

add_executable(b2j example/b2j.cpp ${je2be_files})
target_link_libraries(b2j je2be)

add_executable(dump example/dump.cpp ${je2be_files})
target_link_libraries(dump je2be)

add_executable(test ${test_files} ${je2be_files})
target_include_directories(test PRIVATE deps/doctest)
target_link_libraries(test je2be)

setup_source_groups("example/j2b.cpp;example/b2j.cpp;example/dump.cpp;${test_files};${je2be_files}" "${CMAKE_CURRENT_SOURCE_DIR}")
