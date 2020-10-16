cmake_minimum_required(VERSION 3.0)
project(je2be)

include(CheckCCompilerFlag)

add_definitions(-std=c++17 /std:c++latest -DDLLX= -D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
add_executable(je2be
  src/main.cpp
  include/je2be/level.hpp
  include/je2be/props.hpp
  include/je2be/version.hpp
  include/je2be/db.hpp
  include/je2be/enums.hpp
  include/je2be/keys.hpp
  include/je2be.hpp
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
  ext/libminecraft-file/include/minecraft-file.hpp)
list(APPEND je2be_link_libraries "leveldb")
list(APPEND je2be_include_directories "build/deps/zlib")
list(APPEND je2be_link_directories "build/deps/leveldb/${CMAKE_BUILD_TYPE}")
list(APPEND je2be_link_directories "build/deps/zlib/${CMAKE_BUILD_TYPE}")

set(CMAKE_REQUIRED_FLAGS "-lstdc++fs")
check_c_compiler_flag("" TEST_RESULT_STDCXXFS)
if (TEST_RESULT_STDCXXFS AND "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  list(APPEND je2be_link_libraries stdc++fs)
endif()

target_include_directories(je2be PRIVATE
  include
  ext/libminecraft-file/include
  ext/zlib
  ext/leveldb/include
  ext/leveldb
  ${je2be_include_directories})
target_link_libraries(je2be ${je2be_link_libraries} zlib leveldb)
target_link_directories(je2be PRIVATE ${je2be_link_directories})
