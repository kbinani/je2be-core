cmake_minimum_required(VERSION 3.0)
project(je2be)

include(CheckCCompilerFlag)
include(ext/setup_source_groups/setup_source_groups.cmake)

set(je2be_srcs
  src/main.cpp
  include/je2be/level-data.hpp
  include/je2be/props.hpp
  include/je2be/version.hpp
  include/je2be/db/db-interface.hpp
  include/je2be/db/db.hpp
  include/je2be/enums.hpp
  include/je2be/db/key.hpp
  include/je2be.hpp
  include/je2be/converter.hpp
  include/je2be/block-data.hpp
  include/je2be/height-map.hpp
  include/je2be/biome-map.hpp
  include/je2be/chunk-data.hpp
  include/je2be/portal/portals.hpp
  include/je2be/portal/portal-blocks.hpp
  include/je2be/portal/portal.hpp
  include/je2be/portal/oriented-portal-blocks.hpp
  include/je2be/chunk-data-package.hpp
  include/je2be/file.hpp
  include/je2be/db/null-db.hpp
  include/je2be/entity.hpp
  include/je2be/tile-entity.hpp
  include/je2be/pos.hpp
  include/je2be/db/async-db.hpp
  include/je2be/vec.hpp
  include/je2be/rotation.hpp
  include/je2be/db/raw-db.hpp
  include/je2be/strings.hpp
  include/je2be/item.hpp
  include/je2be/enchant-data.hpp
  include/je2be/potion-data.hpp
  include/je2be/map.hpp
  include/je2be/world-data.hpp
  include/je2be/dimension-data-fragment.hpp
  include/je2be/java-edition-map.hpp
  include/je2be/color/rgba.hpp
  include/je2be/size.hpp
  include/je2be/entity-attributes.hpp
  include/je2be/fireworks-explosion.hpp
  include/je2be/fireworks.hpp
  include/je2be/color/lab.hpp
  include/je2be/xxhash.hpp
  include/je2be/optional.hpp
  ext/libminecraft-file/include/mcfile/biomes/biome-id.hpp
  ext/libminecraft-file/include/mcfile/biomes/from-int.hpp
  ext/libminecraft-file/include/mcfile/biomes/minecraft.hpp
  ext/libminecraft-file/include/mcfile/biomes/name.hpp
  ext/libminecraft-file/include/mcfile/block.hpp
  ext/libminecraft-file/include/mcfile/blocks/block-id.hpp
  ext/libminecraft-file/include/mcfile/blocks/from-name.hpp
  ext/libminecraft-file/include/mcfile/blocks/minecraft.hpp
  ext/libminecraft-file/include/mcfile/blocks/name.hpp
  ext/libminecraft-file/include/mcfile/chunk-section.hpp
  ext/libminecraft-file/include/mcfile/chunk.hpp
  ext/libminecraft-file/include/mcfile/coordinate.hpp
  ext/libminecraft-file/include/mcfile/detail/chunk-data-source.hpp
  ext/libminecraft-file/include/mcfile/detail/chunk-section-112.hpp
  ext/libminecraft-file/include/mcfile/detail/chunk-section-113.hpp
  ext/libminecraft-file/include/mcfile/detail/chunk-section-116.hpp
  ext/libminecraft-file/include/mcfile/detail/chunk-section-generator.hpp
  ext/libminecraft-file/include/mcfile/detail/compression.hpp
  ext/libminecraft-file/include/mcfile/detail/endianness.hpp
  ext/libminecraft-file/include/mcfile/detail/filesystem.hpp
  ext/libminecraft-file/include/mcfile/detail/string.hpp
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
  ext/libminecraft-file/include/mcfile/nbt/short-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/string-tag.hpp
  ext/libminecraft-file/include/mcfile/nbt/tag-factory-impl.hpp
  ext/libminecraft-file/include/mcfile/nbt/tag-factory.hpp
  ext/libminecraft-file/include/mcfile/nbt/tag.hpp
  ext/libminecraft-file/include/mcfile/region.hpp
  ext/libminecraft-file/include/mcfile/stream/byte-stream.hpp
  ext/libminecraft-file/include/mcfile/stream/file-input-stream.hpp
  ext/libminecraft-file/include/mcfile/stream/file-output-stream.hpp
  ext/libminecraft-file/include/mcfile/stream/input-stream-reader.hpp
  ext/libminecraft-file/include/mcfile/stream/input-stream.hpp
  ext/libminecraft-file/include/mcfile/stream/output-stream-writer.hpp
  ext/libminecraft-file/include/mcfile/stream/output-stream.hpp
  ext/libminecraft-file/include/mcfile/world.hpp
  ext/libminecraft-file/include/minecraft-file.hpp
  .editorconfig
  .clang-format)
add_executable(je2be ${je2be_srcs})
setup_source_groups("${je2be_srcs}" "${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_REQUIRED_FLAGS "-lstdc++fs")
check_c_compiler_flag("" TEST_RESULT_STDCXXFS)
if (TEST_RESULT_STDCXXFS)
  if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    list(APPEND je2be_link_libraries stdc++fs)
  endif()
endif()

message("${CMAKE_C_COMPILER_ID}")
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(/std:c++latest /bigobj -D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
  list(APPEND je2be_include_directories "${CMAKE_BINARY_DIR}/deps/zlib")
  list(APPEND je2be_include_directories "ext/zlib")
  list(APPEND je2be_link_directories "${CMAKE_BINARY_DIR}/deps/zlib/${CMAKE_BUILD_TYPE}")
  list(APPEND je2be_link_libraries zlib xxhash)
else()
  add_definitions(-std=c++17)
  list(APPEND je2be_link_libraries z xxhash)
endif()
list(APPEND je2be_include_directories "ext/xxHash")
list(APPEND je2be_link_directories "${CMAKE_BINARY_DIR}/deps/leveldb/${CMAKE_BUILD_TYPE}")
list(APPEND je2be_link_directories "${CMAKE_BINARY_DIR}/deps/xxHash/${CMAKE_BUILD_TYPE}")

target_include_directories(je2be PRIVATE
  include
  ext/libminecraft-file/include
  ext/leveldb/include
  ext/leveldb
  ext/thread-pool/include
  ext/json/single_include
  ${je2be_include_directories})
target_link_libraries(je2be ${je2be_link_libraries} leveldb)
target_link_directories(je2be PRIVATE ${je2be_link_directories})
