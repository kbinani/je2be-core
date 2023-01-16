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
      /MP
    )
  endif()
  if ("${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
    add_compile_options(-Wno-shorten-64-to-32)
  endif()
endif()

set(je2be_header_files
  #begin je2be-header files
  include/je2be.hpp
  include/je2be/box360/converter.hpp
  include/je2be/box360/minecraft-save-info.hpp
  include/je2be/box360/options.hpp
  include/je2be/box360/progress.hpp
  include/je2be/defer.hpp
  include/je2be/enums/level-directory-structure.hpp
  include/je2be/fs.hpp
  include/je2be/nbt.hpp
  include/je2be/pos2.hpp
  include/je2be/status.hpp
  include/je2be/strings.hpp
  include/je2be/tobe/converter.hpp
  include/je2be/tobe/options.hpp
  include/je2be/tobe/progress.hpp
  include/je2be/toje/converter.hpp
  include/je2be/toje/options.hpp
  include/je2be/toje/progress.hpp
  include/je2be/uuid.hpp
  include/je2be/zip-file.hpp
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
  deps/libminecraft-file/include/mcfile/data4b3d.hpp
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
  deps/libminecraft-file/include/mcfile/je/chunksection/chunk-section-empty.hpp
  deps/libminecraft-file/include/mcfile/je/chunksection/chunk-section-generator.hpp
  deps/libminecraft-file/include/mcfile/je/flatten.hpp
  deps/libminecraft-file/include/mcfile/je/heightmaps.hpp
  deps/libminecraft-file/include/mcfile/je/mca-chunk-locator.hpp
  deps/libminecraft-file/include/mcfile/je/mca-editor.hpp
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
  src/_data2d.hpp
  src/_data3d.hpp
  src/_dimension-ext.hpp
  src/_file.hpp
  src/_future-support.hpp
  src/_java-level-dat.hpp
  src/_mcfile-fwd.hpp
  src/_mem.hpp
  src/_nbt-ext.hpp
  src/_nullable.hpp
  src/_optional.hpp
  src/_parallel.hpp
  src/_poi-blocks.hpp
  src/_pos3.hpp
  src/_props.hpp
  src/_queue2d.hpp
  src/_rename-pair.hpp
  src/_reversible-map.hpp
  src/_rotation.hpp
  src/_size.hpp
  src/_static-reversible-map.hpp
  src/_version.hpp
  src/_volume.hpp
  src/_walk.hpp
  src/_xxhash.hpp
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
  src/box360/_biome.hpp
  src/box360/_block-data.hpp
  src/box360/_chunk.hpp
  src/box360/_context.hpp
  src/box360/_entity.hpp
  src/box360/_grid.hpp
  src/box360/_item.hpp
  src/box360/_lzx-decoder.hpp
  src/box360/_savegame.hpp
  src/box360/_stfs-ext.hpp
  src/box360/_stfs.hpp
  src/box360/_terraform.hpp
  src/box360/_tile-entity-convert-result.hpp
  src/box360/_tile-entity.hpp
  src/box360/_world.hpp
  src/color/_lab.hpp
  src/color/_rgba.hpp
  src/color/_sign-color.hpp
  src/command/_command.hpp
  src/command/_target-selector.hpp
  src/command/_token.hpp
  src/db/_async-iterator.hpp
  src/db/_concurrent-db.hpp
  src/db/_db-interface.hpp
  src/db/_db.hpp
  src/db/_null-db.hpp
  src/entity/_armor-stand.hpp
  src/entity/_axolotl.hpp
  src/entity/_boat.hpp
  src/entity/_cat.hpp
  src/entity/_effect.hpp
  src/entity/_entity-attributes.hpp
  src/entity/_frog.hpp
  src/entity/_painting.hpp
  src/entity/_panda.hpp
  src/entity/_tropical-fish.hpp
  src/enums/_banner-color-code-bedrock.hpp
  src/enums/_chunk-conversion-mode.hpp
  src/enums/_color-code-java.hpp
  src/enums/_facing4.hpp
  src/enums/_facing6.hpp
  src/enums/_game-mode.hpp
  src/enums/_red-flower.hpp
  src/enums/_skull-type.hpp
  src/enums/_villager-profession.hpp
  src/enums/_villager-type.hpp
  src/item/_banner.hpp
  src/item/_enchantments.hpp
  src/item/_fireworks-explosion.hpp
  src/item/_fireworks.hpp
  src/item/_goat-horn.hpp
  src/item/_map-color.hpp
  src/item/_potion.hpp
  src/item/_tipped-arrow-potion.hpp
  src/structure/_structure-piece.hpp
  src/terraform-block-property-accessor.cpp
  src/terraform-fence-connectable.cpp
  src/terraform-leaves.cpp
  src/terraform-note-block.cpp
  src/terraform-shape-of-stairs.cpp
  src/terraform-wall-connectable.cpp
  src/terraform/_block-accessor.hpp
  src/terraform/_block-property-accessor.hpp
  src/terraform/_chorus-plant.hpp
  src/terraform/_fence-connectable.hpp
  src/terraform/_leaves.hpp
  src/terraform/_note-block.hpp
  src/terraform/_redstone-wire.hpp
  src/terraform/_shape-of-stairs.hpp
  src/terraform/_snowy.hpp
  src/terraform/_wall-connectable.hpp
  src/terraform/bedrock/_attached-stem.hpp
  src/terraform/bedrock/_block-accessor-bedrock.hpp
  src/terraform/bedrock/_kelp.hpp
  src/terraform/box360/_attached-stem.hpp
  src/terraform/box360/_block-accessor-box360.hpp
  src/terraform/box360/_chest.hpp
  src/terraform/box360/_kelp.hpp
  src/terraform/java/_block-accessor-java-directory.hpp
  src/terraform/java/_block-accessor-java-mca.hpp
  src/terraform/java/_block-accessor-java.hpp
  src/tile-entity/_loot-table.hpp
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
  src/tobe/_axolotl.hpp
  src/tobe/_biome-map-legacy.hpp
  src/tobe/_block-data.hpp
  src/tobe/_block-palette.hpp
  src/tobe/_chunk-data-package.hpp
  src/tobe/_chunk-data.hpp
  src/tobe/_chunk.hpp
  src/tobe/_context.hpp
  src/tobe/_datapacks.hpp
  src/tobe/_enchant-data.hpp
  src/tobe/_entity.hpp
  src/tobe/_flat-world-layers.hpp
  src/tobe/_height-map.hpp
  src/tobe/_item.hpp
  src/tobe/_java-edition-map.hpp
  src/tobe/_level-data.hpp
  src/tobe/_level.hpp
  src/tobe/_map.hpp
  src/tobe/_moving-piston.hpp
  src/tobe/_player-abilities.hpp
  src/tobe/_region.hpp
  src/tobe/_session-lock.hpp
  src/tobe/_sub-chunk.hpp
  src/tobe/_tile-entity.hpp
  src/tobe/_uuid-registrar.hpp
  src/tobe/_versions.hpp
  src/tobe/_world-data.hpp
  src/tobe/_world.hpp
  src/tobe/portal/_oriented-portal-blocks.hpp
  src/tobe/portal/_portal-blocks.hpp
  src/tobe/portal/_portal.hpp
  src/tobe/portal/_portals.hpp
  src/tobe/structure/_structure-piece-collection.hpp
  src/tobe/structure/_structures.hpp
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
  src/toje/_block-accessor-wrapper.hpp
  src/toje/_block-data.hpp
  src/toje/_block-entity-convert-result.hpp
  src/toje/_block-entity.hpp
  src/toje/_chunk.hpp
  src/toje/_constants.hpp
  src/toje/_context.hpp
  src/toje/_entity.hpp
  src/toje/_item.hpp
  src/toje/_level-data.hpp
  src/toje/_map-info.hpp
  src/toje/_region.hpp
  src/toje/_structure-info.hpp
  src/toje/_sub-chunk.hpp
  src/toje/_world.hpp
  src/toje/terraform/_beacon.hpp
  src/toje/terraform/_campfire.hpp
  src/toje/terraform/_cave-vines.hpp
  src/toje/terraform/_door.hpp
  src/toje/terraform/_lighting.hpp
  src/toje/terraform/_piston.hpp
  src/toje/terraform/_tripwire.hpp
  src/toje/terraform/_twisting-vines.hpp
  src/toje/terraform/_weeping-vines.hpp
  src/zip-file.cpp
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
target_include_directories(test PRIVATE src deps/libminecraft-file/test/deps/doctest)

add_executable(j2b2j test/j2b2j.cpp)
target_link_libraries(j2b2j je2be)
target_include_directories(j2b2j PRIVATE src deps/libminecraft-file/test/deps/doctest)

list(APPEND all_files ${test_files} ${je2be_header_files} ${je2be_src_files} example/j2b.cpp example/b2j.cpp example/dump.cpp test/j2b2j.cpp)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${all_files})
