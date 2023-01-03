cmake_minimum_required(VERSION 3.16)
project(je2be)

if ("${CMAKE_PROJECT_NAME}" STREQUAL "${PROJECT_NAME}")
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  if ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options(
      /source-charset:utf-8
      /we4828 # Treat "The file contains a character that cannot be represented in the current code page" warnings as errors
      /wd4100 # Suppress "unreferenced formal parameter" warnings
    )
  endif()
  if ("${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
    add_compile_options(-Wno-shorten-64-to-32)
  endif()
endif()

set(je2be_header_files
  #begin je2be-header files
  include/je2be.hpp
  include/je2be/_data3d.hpp
  include/je2be/_dimension-ext.hpp
  include/je2be/_file.hpp
  include/je2be/_fs.hpp
  include/je2be/_future-support.hpp
  include/je2be/_java-level-dat.hpp
  include/je2be/_mcfile-fwd.hpp
  include/je2be/_mem.hpp
  include/je2be/_nbt-ext.hpp
  include/je2be/_nullable.hpp
  include/je2be/_optional.hpp
  include/je2be/_parallel.hpp
  include/je2be/_poi-blocks.hpp
  include/je2be/_pos3.hpp
  include/je2be/_props.hpp
  include/je2be/_rename-pair.hpp
  include/je2be/_reversible-map.hpp
  include/je2be/_rotation.hpp
  include/je2be/_size.hpp
  include/je2be/_static-reversible-map.hpp
  include/je2be/_version.hpp
  include/je2be/_volume.hpp
  include/je2be/_xxhash.hpp
  include/je2be/_zip-file.hpp
  include/je2be/box360/_biome.hpp
  include/je2be/box360/_block-data.hpp
  include/je2be/box360/_chunk.hpp
  include/je2be/box360/_context.hpp
  include/je2be/box360/_entity.hpp
  include/je2be/box360/_grid.hpp
  include/je2be/box360/_item.hpp
  include/je2be/box360/_lzx-decoder.hpp
  include/je2be/box360/_savegame.hpp
  include/je2be/box360/_stfs-ext.hpp
  include/je2be/box360/_stfs.hpp
  include/je2be/box360/_terraform.hpp
  include/je2be/box360/_tile-entity-convert-result.hpp
  include/je2be/box360/_tile-entity.hpp
  include/je2be/box360/_world.hpp
  include/je2be/box360/converter.hpp
  include/je2be/box360/minecraft-save-info.hpp
  include/je2be/box360/options.hpp
  include/je2be/box360/progress.hpp
  include/je2be/color/_lab.hpp
  include/je2be/color/_rgba.hpp
  include/je2be/color/_sign-color.hpp
  include/je2be/command/_command.hpp
  include/je2be/command/_target-selector.hpp
  include/je2be/command/_token.hpp
  include/je2be/db/_async-iterator.hpp
  include/je2be/db/_db-interface.hpp
  include/je2be/db/_db.hpp
  include/je2be/db/_null-db.hpp
  include/je2be/db/_raw-db.hpp
  include/je2be/defer.hpp
  include/je2be/entity/_armor-stand.hpp
  include/je2be/entity/_axolotl.hpp
  include/je2be/entity/_boat.hpp
  include/je2be/entity/_cat.hpp
  include/je2be/entity/_effect.hpp
  include/je2be/entity/_entity-attributes.hpp
  include/je2be/entity/_frog.hpp
  include/je2be/entity/_painting.hpp
  include/je2be/entity/_panda.hpp
  include/je2be/entity/_tropical-fish.hpp
  include/je2be/enums/_banner-color-code-bedrock.hpp
  include/je2be/enums/_chunk-conversion-mode.hpp
  include/je2be/enums/_color-code-java.hpp
  include/je2be/enums/_facing4.hpp
  include/je2be/enums/_facing6.hpp
  include/je2be/enums/_game-mode.hpp
  include/je2be/enums/_red-flower.hpp
  include/je2be/enums/_skull-type.hpp
  include/je2be/enums/_villager-profession.hpp
  include/je2be/enums/_villager-type.hpp
  include/je2be/enums/level-directory-structure.hpp
  include/je2be/item/_banner.hpp
  include/je2be/item/_enchantments.hpp
  include/je2be/item/_fireworks-explosion.hpp
  include/je2be/item/_fireworks.hpp
  include/je2be/item/_goat-horn.hpp
  include/je2be/item/_map-color.hpp
  include/je2be/item/_potion.hpp
  include/je2be/item/_tipped-arrow-potion.hpp
  include/je2be/nbt.hpp
  include/je2be/pos2.hpp
  include/je2be/status.hpp
  include/je2be/strings.hpp
  include/je2be/structure/_structure-piece.hpp
  include/je2be/terraform/_block-accessor.hpp
  include/je2be/terraform/_block-property-accessor.hpp
  include/je2be/terraform/_chorus-plant.hpp
  include/je2be/terraform/_fence-connectable.hpp
  include/je2be/terraform/_leaves.hpp
  include/je2be/terraform/_note-block.hpp
  include/je2be/terraform/_redstone-wire.hpp
  include/je2be/terraform/_shape-of-stairs.hpp
  include/je2be/terraform/_snowy.hpp
  include/je2be/terraform/_wall-connectable.hpp
  include/je2be/terraform/bedrock/_attached-stem.hpp
  include/je2be/terraform/bedrock/_block-accessor-bedrock.hpp
  include/je2be/terraform/bedrock/_kelp.hpp
  include/je2be/terraform/box360/_attached-stem.hpp
  include/je2be/terraform/box360/_block-accessor-box360.hpp
  include/je2be/terraform/box360/_chest.hpp
  include/je2be/terraform/box360/_kelp.hpp
  include/je2be/tile-entity/_loot-table.hpp
  include/je2be/tobe/_axolotl.hpp
  include/je2be/tobe/_biome-map-legacy.hpp
  include/je2be/tobe/_block-data.hpp
  include/je2be/tobe/_block-palette.hpp
  include/je2be/tobe/_chunk-data-package.hpp
  include/je2be/tobe/_chunk-data.hpp
  include/je2be/tobe/_chunk.hpp
  include/je2be/tobe/_context.hpp
  include/je2be/tobe/_datapacks.hpp
  include/je2be/tobe/_enchant-data.hpp
  include/je2be/tobe/_entity.hpp
  include/je2be/tobe/_flat-world-layers.hpp
  include/je2be/tobe/_height-map.hpp
  include/je2be/tobe/_item.hpp
  include/je2be/tobe/_java-edition-map.hpp
  include/je2be/tobe/_level-data.hpp
  include/je2be/tobe/_level.hpp
  include/je2be/tobe/_map.hpp
  include/je2be/tobe/_moving-piston.hpp
  include/je2be/tobe/_player-abilities.hpp
  include/je2be/tobe/_region.hpp
  include/je2be/tobe/_session-lock.hpp
  include/je2be/tobe/_sub-chunk.hpp
  include/je2be/tobe/_tile-entity.hpp
  include/je2be/tobe/_uuid-registrar.hpp
  include/je2be/tobe/_versions.hpp
  include/je2be/tobe/_world-data.hpp
  include/je2be/tobe/_world.hpp
  include/je2be/tobe/converter.hpp
  include/je2be/tobe/options.hpp
  include/je2be/tobe/portal/_oriented-portal-blocks.hpp
  include/je2be/tobe/portal/_portal-blocks.hpp
  include/je2be/tobe/portal/_portal.hpp
  include/je2be/tobe/portal/_portals.hpp
  include/je2be/tobe/progress.hpp
  include/je2be/tobe/structure/_structure-piece-collection.hpp
  include/je2be/tobe/structure/_structures.hpp
  include/je2be/toje/_block-accessor-wrapper.hpp
  include/je2be/toje/_block-data.hpp
  include/je2be/toje/_block-entity-convert-result.hpp
  include/je2be/toje/_block-entity.hpp
  include/je2be/toje/_chunk.hpp
  include/je2be/toje/_constants.hpp
  include/je2be/toje/_context.hpp
  include/je2be/toje/_entity.hpp
  include/je2be/toje/_item.hpp
  include/je2be/toje/_level-data.hpp
  include/je2be/toje/_map-info.hpp
  include/je2be/toje/_region.hpp
  include/je2be/toje/_structure-info.hpp
  include/je2be/toje/_sub-chunk.hpp
  include/je2be/toje/_world.hpp
  include/je2be/toje/converter.hpp
  include/je2be/toje/options.hpp
  include/je2be/toje/progress.hpp
  include/je2be/toje/terraform/_beacon.hpp
  include/je2be/toje/terraform/_campfire.hpp
  include/je2be/toje/terraform/_cave-vines.hpp
  include/je2be/toje/terraform/_door.hpp
  include/je2be/toje/terraform/_piston.hpp
  include/je2be/toje/terraform/_tripwire.hpp
  include/je2be/toje/terraform/_twisting-vines.hpp
  include/je2be/toje/terraform/_weeping-vines.hpp
  include/je2be/uuid.hpp
  #end je2be-header files
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
  deps/libminecraft-file/include/mcfile/blocks/block-data.hpp
  deps/libminecraft-file/include/mcfile/blocks/block-id.hpp
  deps/libminecraft-file/include/mcfile/blocks/data/ageable.hpp
  deps/libminecraft-file/include/mcfile/blocks/data/bisected.hpp
  deps/libminecraft-file/include/mcfile/blocks/data/directional.hpp
  deps/libminecraft-file/include/mcfile/blocks/data/type/stairs.hpp
  deps/libminecraft-file/include/mcfile/blocks/from-name.hpp
  deps/libminecraft-file/include/mcfile/blocks/make-block-data.hpp
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
  deps/libminecraft-file/include/mcfile/je/mca-chunk-locator.hpp
  deps/libminecraft-file/include/mcfile/je/region.hpp
  deps/libminecraft-file/include/mcfile/je/set-block-options.hpp
  deps/libminecraft-file/include/mcfile/je/ticking-block.hpp
  deps/libminecraft-file/include/mcfile/je/world.hpp
  deps/libminecraft-file/include/mcfile/je/writable-chunk.hpp
  deps/libminecraft-file/include/mcfile/mem.hpp
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
  deps/libminecraft-file/include/mcfile/scoped-file.hpp
  deps/libminecraft-file/include/mcfile/stream/byte-input-stream.hpp
  deps/libminecraft-file/include/mcfile/stream/byte-stream.hpp
  deps/libminecraft-file/include/mcfile/stream/defer-opening-output-stream.hpp
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
set(je2be_src_files
  #begin je2be-src files
  src/box360-block-data.cpp
  src/box360-chunk.cpp
  src/box360-converter.cpp
  src/box360-entity.cpp
  src/box360-item.cpp
  src/box360-minecraft-save-info.cpp
  src/box360-savegame.cpp
  src/box360-terraform.cpp
  src/box360-tile-entity.cpp
  src/box360-world.cpp
  src/db-raw-db.cpp
  src/terraform-block-property-accessor.cpp
  src/terraform-fence-connectable.cpp
  src/terraform-leaves.cpp
  src/terraform-note-block.cpp
  src/terraform-shape-of-stairs.cpp
  src/terraform-wall-connectable.cpp
  src/tobe-block-data.cpp
  src/tobe-chunk-data-package.cpp
  src/tobe-chunk.cpp
  src/tobe-converter.cpp
  src/tobe-entity.cpp
  src/tobe-item.cpp
  src/tobe-map.cpp
  src/tobe-moving-piston.cpp
  src/tobe-region.cpp
  src/tobe-sub-chunk.cpp
  src/tobe-tile-entity.cpp
  src/tobe-world.cpp
  src/toje-block-data.cpp
  src/toje-block-entity.cpp
  src/toje-chunk.cpp
  src/toje-context.cpp
  src/toje-converter.cpp
  src/toje-entity.cpp
  src/toje-item.cpp
  src/toje-region.cpp
  src/toje-sub-chunk.cpp
  src/toje-world.cpp
  #end je2be-src files
  )
set(test_files
  test/main.cpp
  test/moving-piston.test.hpp
  test/command.test.hpp
  test/research.hpp
  test/block-data.test.hpp
  test/CheckTag.hpp
  test/j2b2j.hpp
  test/j2b2j.test.hpp
  test/volume.test.hpp
  test/shoulder-riders.test.hpp
  test/uuid.test.hpp
  test/loot-table.test.hpp
  test/bee-nest.test.hpp
  test/end-gateway.test.hpp
  test/strings.test.hpp
  test/je2be-all.hpp)

add_subdirectory(deps/libminecraft-file)

if (NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  list(APPEND je2be_link_libraries pthread)
endif()

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(/bigobj -D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
elseif(NOT "${EMSCRIPTEN}" STREQUAL "1" AND NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
  add_definitions(-D_GLIBCXX_USE_TBB_PAR_BACKEND=0)
endif()

find_package(ZLIB QUIET)
if (NOT ZLIB_FOUND)
  set(ZLIB_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/deps/libminecraft-file/deps/zlib/$<CONFIG>/zlibstatic$<IF:$<CONFIG:Debug>,d,>.lib)
  set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/libminecraft-file/deps/zlib)
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/libminecraft-file/deps/zlib
    ${CMAKE_CURRENT_BINARY_DIR}/deps/libminecraft-file/deps/zlib)
  list(APPEND je2be_include_directories
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/libminecraft-file/deps/zlib
    ${CMAKE_CURRENT_BINARY_DIR}/deps/libminecraft-file/deps/zlib)
endif()

option(LEVELDB_ZSTD OFF)
option(LEVELDB_BUILD_TESTS OFF)
option(LEVELDB_BUILD_BENCHMARKS OFF)
option(LEVELDB_INSTALL OFF)
add_subdirectory(deps/leveldb EXCLUDE_FROM_ALL)

option(MZ_FETCH_LIBS OFF)
option(MZ_BZIP2 OFF)
option(MZ_LZMA OFF)
option(MZ_ZSTD OFF)
option(MZ_PKCRYPT OFF)
option(MZ_WZAES OFF)
option(MZ_OPENSSL OFF)
option(MZ_LIBBSD OFF)
option(MZ_SIGNING OFF)
add_subdirectory(deps/minizip-ng EXCLUDE_FROM_ALL)

add_subdirectory(deps/cxxopts EXCLUDE_FROM_ALL)

list(APPEND je2be_link_libraries mcfile leveldb minizip)


add_library(je2be STATIC ${je2be_header_files} ${je2be_src_files})
target_include_directories(je2be PUBLIC
  ./include
  deps/leveldb
  deps/leveldb/include
  deps/xxhash
  deps/thread-pool
  deps/json/include
  deps/minizip-ng
  deps/libminecraft-file/include
  deps/libminecraft-file/deps/libdeflate
  ${je2be_include_directories})
target_include_directories(je2be PRIVATE ./src)
target_link_libraries(je2be INTERFACE ${je2be_link_libraries})
target_link_directories(je2be INTERFACE ${je2be_link_directories})

add_executable(j2b example/j2b.cpp ${je2be_header_files})
target_link_libraries(j2b je2be cxxopts)

add_executable(b2j example/b2j.cpp ${je2be_header_files})
target_link_libraries(b2j je2be cxxopts)

add_executable(dump example/dump.cpp ${je2be_header_files})
target_link_libraries(dump je2be)

add_executable(test ${test_files} ${je2be_header_files})
target_link_libraries(test je2be)
target_include_directories(test PRIVATE deps/libminecraft-file/test/deps/doctest)

add_executable(j2b2j test/j2b2j.cpp)
target_link_libraries(j2b2j je2be)
target_include_directories(j2b2j PRIVATE deps/libminecraft-file/test/deps/doctest)

list(APPEND all_files ${test_files} ${je2be_header_files} ${je2be_src_files} example/j2b.cpp example/b2j.cpp example/dump.cpp test/j2b2j.cpp)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${all_files})
