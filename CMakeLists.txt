cmake_minimum_required(VERSION 3.16)
project(je2be)

include(CheckCCompilerFlag)
include(ext/setup_source_groups/setup_source_groups.cmake)

set(BUILD_SHARED_LIBS OFF)

set(je2be_files
  #begin je2be source files
  include/je2be.hpp
  include/je2be/algorithm.hpp
  include/je2be/axolotl.hpp
  include/je2be/biome-map.hpp
  include/je2be/block-data.hpp
  include/je2be/block-palette.hpp
  include/je2be/chunk-data-package.hpp
  include/je2be/chunk-data.hpp
  include/je2be/color/lab.hpp
  include/je2be/color/rgba.hpp
  include/je2be/converter.hpp
  include/je2be/db/async-db.hpp
  include/je2be/db/db-interface.hpp
  include/je2be/db/db.hpp
  include/je2be/db/null-db.hpp
  include/je2be/db/raw-db.hpp
  include/je2be/dimension-data-fragment.hpp
  include/je2be/enchant-data.hpp
  include/je2be/entity-attributes.hpp
  include/je2be/entity.hpp
  include/je2be/enums/banner-color-code-bedrock.hpp
  include/je2be/enums/color-code-java.hpp
  include/je2be/enums/level-directory-structure.hpp
  include/je2be/enums/villager-profession.hpp
  include/je2be/enums/villager-type.hpp
  include/je2be/file.hpp
  include/je2be/fireworks-explosion.hpp
  include/je2be/fireworks.hpp
  include/je2be/height-map.hpp
  include/je2be/item.hpp
  include/je2be/java-edition-map.hpp
  include/je2be/level-data.hpp
  include/je2be/map.hpp
  include/je2be/moving-piston.hpp
  include/je2be/optional.hpp
  include/je2be/options.hpp
  include/je2be/portal/oriented-portal-blocks.hpp
  include/je2be/portal/portal-blocks.hpp
  include/je2be/portal/portal.hpp
  include/je2be/portal/portals.hpp
  include/je2be/pos2.hpp
  include/je2be/pos3.hpp
  include/je2be/potion-data.hpp
  include/je2be/progress.hpp
  include/je2be/props.hpp
  include/je2be/rotation.hpp
  include/je2be/sign.hpp
  include/je2be/size.hpp
  include/je2be/statistics.hpp
  include/je2be/strings.hpp
  include/je2be/structure/structure-piece-collection.hpp
  include/je2be/structure/structure-piece.hpp
  include/je2be/structure/structures.hpp
  include/je2be/tile-entity.hpp
  include/je2be/tropical-fish.hpp
  include/je2be/vec.hpp
  include/je2be/version.hpp
  include/je2be/volume.hpp
  include/je2be/world-data.hpp
  include/je2be/xxhash.hpp
  #end je2be source files
  # (cd ext/libminecraft-file; git ls-files | grep ^include | sed 's:\(.*\):  ext/libminecraft-file/\1:g') | pbcopy
  #begin libminecraft-file source files
  ext/libminecraft-file/include/mcfile/be/biome.hpp
  ext/libminecraft-file/include/mcfile/be/block.hpp
  ext/libminecraft-file/include/mcfile/be/chunk.hpp
  ext/libminecraft-file/include/mcfile/be/db-interface.hpp
  ext/libminecraft-file/include/mcfile/be/db-key.hpp
  ext/libminecraft-file/include/mcfile/be/pending-tick.hpp
  ext/libminecraft-file/include/mcfile/be/sub-chunk.hpp
  ext/libminecraft-file/include/mcfile/biomes/biome-id.hpp
  ext/libminecraft-file/include/mcfile/biomes/from-int.hpp
  ext/libminecraft-file/include/mcfile/biomes/minecraft.hpp
  ext/libminecraft-file/include/mcfile/biomes/name.hpp
  ext/libminecraft-file/include/mcfile/blocks/block-id.hpp
  ext/libminecraft-file/include/mcfile/blocks/from-name.hpp
  ext/libminecraft-file/include/mcfile/blocks/minecraft.hpp
  ext/libminecraft-file/include/mcfile/blocks/name.hpp
  ext/libminecraft-file/include/mcfile/compression.hpp
  ext/libminecraft-file/include/mcfile/coordinate.hpp
  ext/libminecraft-file/include/mcfile/dimension.hpp
  ext/libminecraft-file/include/mcfile/endianness.hpp
  ext/libminecraft-file/include/mcfile/file.hpp
  ext/libminecraft-file/include/mcfile/je/block.hpp
  ext/libminecraft-file/include/mcfile/je/cached-chunk-loader.hpp
  ext/libminecraft-file/include/mcfile/je/chunk-section.hpp
  ext/libminecraft-file/include/mcfile/je/chunk.hpp
  ext/libminecraft-file/include/mcfile/je/chunksection/block-states-parser-113.hpp
  ext/libminecraft-file/include/mcfile/je/chunksection/block-states-parser-116.hpp
  ext/libminecraft-file/include/mcfile/je/chunksection/chunk-section-112.hpp
  ext/libminecraft-file/include/mcfile/je/chunksection/chunk-section-113-base.hpp
  ext/libminecraft-file/include/mcfile/je/chunksection/chunk-section-113.hpp
  ext/libminecraft-file/include/mcfile/je/chunksection/chunk-section-116.hpp
  ext/libminecraft-file/include/mcfile/je/chunksection/chunk-section-generator.hpp
  ext/libminecraft-file/include/mcfile/je/mca-data-source.hpp
  ext/libminecraft-file/include/mcfile/je/region.hpp
  ext/libminecraft-file/include/mcfile/je/set-block-options.hpp
  ext/libminecraft-file/include/mcfile/je/ticking-block.hpp
  ext/libminecraft-file/include/mcfile/je/world.hpp
  ext/libminecraft-file/include/mcfile/je/writable-chunk.hpp
  ext/libminecraft-file/include/mcfile/nbt/byte-array-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/byte-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/compound-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/detail/scalar-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/detail/vector-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/double-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/end-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/float-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/int-array-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/int-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/list-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/long-array-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/long-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/print-as-json.hpp
  ext/libminecraft-file/include/mcfile/nbt/short-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/string-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/tag-factory-impl.hpp
  ext/libminecraft-file/include/mcfile/nbt/tag-factory.hpp
  ext/libminecraft-file/include/mcfile/nbt/tag.hpp
  ext/libminecraft-file/include/mcfile/pos2.hpp
  ext/libminecraft-file/include/mcfile/pos3.hpp
  ext/libminecraft-file/include/mcfile/stream/byte-stream.hpp
  ext/libminecraft-file/include/mcfile/stream/file-input-stream.hpp
  ext/libminecraft-file/include/mcfile/stream/file-output-stream.hpp
  ext/libminecraft-file/include/mcfile/stream/input-stream-reader.hpp
  ext/libminecraft-file/include/mcfile/stream/input-stream.hpp
  ext/libminecraft-file/include/mcfile/stream/output-stream-writer.hpp
  ext/libminecraft-file/include/mcfile/stream/output-stream.hpp
  ext/libminecraft-file/include/mcfile/string.hpp
  ext/libminecraft-file/include/minecraft-file.hpp
  #end libminecraft-file source files
  .editorconfig
  .clang-format)
set(test_files
  test/main.cpp
  test/block-data.test.cpp
  test/moving-piston.test.cpp)
add_executable(cli src/cli.cpp ${je2be_files})
add_executable(dump src/dump.cpp ${je2be_files})
add_executable(test ${test_files} ${je2be_files})
setup_source_groups("src/cli.cpp;src/dump.cpp;${test_files};${je2be_files}" "${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_REQUIRED_FLAGS "-lstdc++fs")
check_c_compiler_flag("" TEST_RESULT_STDCXXFS)
if (TEST_RESULT_STDCXXFS)
  if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    list(APPEND je2be_link_libraries stdc++fs)
  endif()
endif()

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  list(APPEND je2be_link_libraries pthread)
endif()

list(APPEND je2be_include_directories include)
list(APPEND je2be_include_directories ext/libminecraft-file/include)
list(APPEND je2be_include_directories ext/leveldb)
list(APPEND je2be_include_directories ext/thread-pool/include)
list(APPEND je2be_include_directories ext/json/single_include)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(/std:c++latest /bigobj -D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)

  set(ZLIB_COMPAT ON)
  add_subdirectory(ext/zlib-ng EXCLUDE_FROM_ALL)
  set(ZLIB_INCLUDE_DIR $<TARGET_PROPERTY:zlibstatic,INCLUDE_DIRECTORIES>)
  set(ZLIB_LIBRARY $<TARGET_LINKER_FILE:zlibstatic>)

  list(APPEND je2be_link_libraries zlibstatic)
else()
  add_definitions(-std=c++17)

  list(APPEND je2be_link_libraries z)
endif()

add_subdirectory(ext/xxHash/cmake_unofficial EXCLUDE_FROM_ALL)

set(LEVELDB_ZSTD OFF)
add_subdirectory(ext/leveldb EXCLUDE_FROM_ALL)

list(APPEND je2be_link_libraries xxhash leveldb)

target_include_directories(cli PRIVATE ${je2be_include_directories})
target_link_libraries(cli ${je2be_link_libraries})
target_link_directories(cli PRIVATE ${je2be_link_directories})

target_include_directories(dump PRIVATE ${je2be_include_directories})
target_link_libraries(dump ${je2be_link_libraries})
target_link_directories(dump PRIVATE ${je2be_link_directories})

target_include_directories(test PRIVATE ext/doctest ${je2be_include_directories})
target_link_libraries(test ${je2be_link_libraries})
target_link_directories(test PRIVATE ${je2be_link_directories})
