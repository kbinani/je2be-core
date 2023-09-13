cmake_minimum_required(VERSION 3.16)
project(je2be)

include(cmake/CPM.cmake)

option(JE2BE_CORE_USE_INTEL_COMPILER OFF)

if ("${CMAKE_PROJECT_NAME}" STREQUAL "${PROJECT_NAME}")
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  if (MSVC)
    add_compile_options(
      /source-charset:utf-8
      /we4828 # Treat "The file contains a character that cannot be represented in the current code page" warnings as errors
      /wd4100 # Suppress "unreferenced formal parameter" warnings
      /MP
      /Zc:__cplusplus # Enable updated __cplusplus macro
      /Zc:preprocessor # Enable preprocessor conformance mode
    )
    if (JE2BE_CORE_USE_INTEL_COMPILER)
      add_link_options(
        $<IF:$<BOOL:JE2BE_CORE_USE_INTEL_COMPILER>,/LIBPATH:$(IPP1ALibDir),>
        $<IF:$<BOOL:JE2BE_CORE_USE_INTEL_COMPILER>,/LIBPATH:$(IPP1ACPPLibDir),>
        $<IF:$<BOOL:JE2BE_CORE_USE_INTEL_COMPILER>,/LIBPATH:$(ICXLibDir),>)
    endif()
  endif()
  if ("${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
    add_compile_options(-Wno-shorten-64-to-32)
  endif()
endif()

CPMAddPackage(
  NAME libminecraft-file
  GITHUB_REPOSITORY kbinani/libminecraft-file
  GIT_TAG d31da8a727a57dfad72fe2137a9671d295fc377b)

set(je2be_header_files
  #begin je2be-header files
  include/je2be.hpp
  include/je2be/box360/converter.hpp
  include/je2be/box360/minecraft-save-info.hpp
  include/je2be/box360/options.hpp
  include/je2be/box360/progress.hpp
  include/je2be/defer.hpp
  include/je2be/enums/level-directory-structure.hpp
  include/je2be/errno.hpp
  include/je2be/fs.hpp
  include/je2be/integers.hpp
  include/je2be/nbt.hpp
  include/je2be/pos2.hpp
  include/je2be/rational.hpp
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
  .editorconfig
  .clang-format)
set(mcfile_header_files
  # (cd deps/libminecraft-file; git ls-files | grep ^include | sed 's:\(.*\):  deps/libminecraft-file/\1:g') | pbcopy
  #begin libminecraft-file source files
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/algorithm.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/be/biome-map.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/be/biome-section.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/be/biome.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/be/block.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/be/chunk.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/be/db-interface.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/be/db-key.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/be/db.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/be/pending-tick.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/be/sub-chunk.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/be/wrap-db.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/biome-palette.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/biomes/biome-id.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/biomes/biome.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/biomes/minecraft.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/blocks/block-data.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/blocks/block-id.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/blocks/data/ageable.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/blocks/data/bisected.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/blocks/data/directional.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/blocks/data/type/stairs.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/blocks/from-name.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/blocks/make-block-data.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/blocks/minecraft.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/blocks/name.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/compression.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/coordinate.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/data4b3d.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/dimension.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/endianness.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/file.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/block-palette.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/block.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/cached-chunk-loader.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/chunk-section.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/chunk.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/chunksection/block-states-parser-113.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/chunksection/block-states-parser-116.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/chunksection/chunk-section-112.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/chunksection/chunk-section-113-base.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/chunksection/chunk-section-113.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/chunksection/chunk-section-116.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/chunksection/chunk-section-118.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/chunksection/chunk-section-empty.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/chunksection/chunk-section-generator.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/flatten.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/heightmaps.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/mca-chunk-locator.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/mca-editor.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/region.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/set-block-options.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/ticking-block.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/world.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/writable-chunk.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/mem.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/byte-array-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/byte-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/compound-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/detail/scalar-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/detail/vector-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/double-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/end-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/float-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/int-array-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/int-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/list-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/long-array-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/long-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/print-as-json.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/short-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/string-tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/tag-factory-impl.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/tag-factory.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/nbt/tag.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/palette-list.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/pos2.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/pos3.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/scoped-file.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/stream/byte-input-stream.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/stream/byte-stream.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/stream/defer-opening-output-stream.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/stream/file-input-stream.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/stream/file-output-stream.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/stream/gz-file-input-stream.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/stream/gz-file-output-stream.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/stream/input-stream-reader.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/stream/input-stream.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/stream/output-stream-writer.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/stream/output-stream.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/string.hpp
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/u8stream.hpp
  ${libminecraft-file_SOURCE_DIR}/include/minecraft-file.hpp
  #end libminecraft-file source files
)
set(je2be_src_files
  #begin je2be-src files
  src/_closed-range.hpp
  src/_data2d.hpp
  src/_data3d.hpp
  src/_default-map.hpp
  src/_dimension-ext.hpp
  src/_directory-iterator.hpp
  src/_file.hpp
  src/_future-support.hpp
  src/_java-level-dat.hpp
  src/_mcfile-fwd.hpp
  src/_mem.hpp
  src/_namespace.hpp
  src/_nbt-ext.hpp
  src/_nullable.hpp
  src/_optional.hpp
  src/_parallel.hpp
  src/_poi-blocks.hpp
  src/_pos2i-set.hpp
  src/_pos3.hpp
  src/_props.hpp
  src/_queue2d.hpp
  src/_rename-pair.hpp
  src/_reversible-map.hpp
  src/_rotation.hpp
  src/_size.hpp
  src/_static-reversible-map.hpp
  src/_system.hpp
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
  src/db/_firewall-env.hpp
  src/db/_null-db.hpp
  src/db/_proxy-env.hpp
  src/db/_readonly-db.hpp
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
  src/item/_map-decoration.hpp
  src/item/_potion.hpp
  src/item/_tipped-arrow-potion.hpp
  src/structure/_structure-piece.hpp
  src/system.cpp
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
  src/terraform/box360/_nether-portal.hpp
  src/terraform/java/_block-accessor-java-directory.hpp
  src/terraform/java/_block-accessor-java-mca.hpp
  src/terraform/java/_block-accessor-java.hpp
  src/terraform/lighting/_chunk-light-cache.hpp
  src/terraform/lighting/_chunk-lighting-model.hpp
  src/terraform/lighting/_light-cache.hpp
  src/terraform/lighting/_lighting-model.hpp
  src/terraform/lighting/_lighting.hpp
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
  src/tobe/_entity-store.hpp
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
  src/toje/_legacy-block.hpp
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
  test/je2be-all.hpp
  test/pos2i-set.test.hpp
  test/system.test.hpp
  test/b2j2b.test.hpp
  test/toje-legacy-block.test.hpp)

if (NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  list(APPEND je2be_link_libraries pthread)
endif()

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(/bigobj -D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
elseif(NOT "${EMSCRIPTEN}" STREQUAL "1" AND NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
  add_definitions(-D_GLIBCXX_USE_TBB_PAR_BACKEND=0)
endif()

list(FIND CPM_PACKAGES "ZLIB" zlib_imported_with_cpm)
if (zlib_imported_with_cpm GREATER -1)
  include_directories(
    ${zlib_SOURCE_DIR}
    ${zlib_BINARY_DIR})
  list(APPEND je2be_include_directories
    ${zlib_SOURCE_DIR}
    ${zlib_BINARY_DIR})
endif()

CPMAddPackage(
  NAME leveldb
  GITHUB_REPOSITORY kbinani/leveldb-stdfs
  GIT_TAG 02db051ba84b6b71d7cc3eff4565870a989aec46
  OPTIONS "LEVELDB_ZSTD OFF"
          "LEVELDB_BUILD_TESTS OFF"
          "LEVELDB_BUILD_BENCHMARKS OFF"
          "LEVELDB_INSTALL OFF"
          "LEVELDB_SNAPPY OFF")

CPMAddPackage(
  NAME cxxopts
  GITHUB_REPOSITORY jarro2783/cxxopts
  VERSION 3.1.1
  OPTIONS "CXXOPTS_BUILD_EXAMPLES OFF"
          "CXXOPTS_BUILD_TESTS OFF")

CPMAddPackage(
  NAME nlohmann_json
  VERSION 3.11.2
  # the git repo is incredibly large, so we download the archived include directory
  URL https://github.com/nlohmann/json/releases/download/v3.11.2/include.zip
  URL_HASH SHA256=e5c7a9f49a16814be27e4ed0ee900ecd0092bfb7dbfca65b5a421b774dccaaed)

if (nlohmann_json_ADDED)
  add_library(nlohmann_json INTERFACE IMPORTED)
  target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/include)
endif()

CPMAddPackage(
  NAME xxhash
  GIT_TAG c2866db364b6ea3a11933e62235ddc166ba18565
  GIT_REPOSITORY https://github.com/stbrumme/xxhash.git)

CPMAddPackage(
  NAME minizip
  GITHUB_REPOSITORY zlib-ng/minizip-ng
  GIT_TAG 3.0.8
  OPTIONS "MZ_FETCH_LIBS OFF"
          "MZ_BZIP2 OFF"
          "MZ_LZMA OFF"
          "MZ_ZSTD OFF"
          "MZ_PKCRYPT OFF"
          "MZ_WZAES OFF"
          "MZ_OPENSSL OFF"
          "MZ_LIBBSD OFF"
          "MZ_SIGNING OFF"
          "MZ_LIBCOMP OFF")

list(APPEND je2be_link_libraries mcfile leveldb minizip nlohmann_json)
list(APPEND je2be_exe_link_libraries je2be)

if (MSVC)
  CPMAddPackage(
    NAME mimalloc
    GITHUB_REPOSITORY microsoft/mimalloc
    VERSION 2.0.9
    OPTIONS "MI_OVERRIDE ON"
            "MI_BUILD_TESTS OFF"
            "MI_BUILD_OBJECT OFF"
            "MI_BUILD_STATIC OFF"
            "MI_WIN_REDIRECT ON")
  list(APPEND je2be_exe_link_libraries mimalloc)
endif()

CPMAddPackage(
  NAME pbar
  GIT_TAG 4aedbcaa1ea8f192a20546a5120517762c827ab6
  GIT_REPOSITORY https://github.com/estshorter/pbar.git)


add_library(je2be STATIC ${je2be_header_files} ${mcfile_header_files} ${je2be_src_files})
target_include_directories(je2be PUBLIC
  ./include
  ${leveldb_SOURCE_DIR}
  ${leveldb_SOURCE_DIR}/include
  ${minizip_SOURCE_DIR}
  ${xxhash_SOURCE_DIR}
  ${nlohmann_json_SOURCE_DIR}/include
  ${je2be_include_directories})
target_include_directories(je2be PRIVATE ./src ${libdeflate_SOURCE_DIR} ${libminecraft-file_SOURCE_DIR}/include)
target_link_libraries(je2be INTERFACE ${je2be_link_libraries})
target_link_directories(je2be INTERFACE ${je2be_link_directories})

add_executable(j2b example/j2b.cpp ${je2be_header_files})
target_link_libraries(j2b ${je2be_exe_link_libraries} cxxopts)
target_include_directories(j2b PRIVATE ${pbar_SOURCE_DIR})

add_executable(b2j example/b2j.cpp ${je2be_header_files})
target_link_libraries(b2j ${je2be_exe_link_libraries} cxxopts)
target_include_directories(b2j PRIVATE ${pbar_SOURCE_DIR})

add_executable(x2j example/x2j.cpp ${je2be_header_files})
target_link_libraries(x2j ${je2be_exe_link_libraries} cxxopts)
target_include_directories(x2j PRIVATE ${pbar_SOURCE_DIR})

add_executable(dump example/dump.cpp ${je2be_header_files})
target_link_libraries(dump je2be)

add_executable(je2be-test ${test_files} ${je2be_header_files})
target_link_libraries(je2be-test ${je2be_exe_link_libraries} doctest)
target_include_directories(je2be-test PRIVATE src)

add_executable(j2b2j test/j2b2j.cpp)
target_link_libraries(j2b2j je2be doctest)
target_include_directories(j2b2j PRIVATE src)

if (MSVC)
  foreach(target IN ITEMS j2b b2j x2j je2be-test)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy "${mimalloc_BINARY_DIR}/\$(Configuration)/mimalloc.dll" "$<TARGET_FILE_DIR:${target}>/"
      COMMAND "${CMAKE_COMMAND}" -E copy "${mimalloc_BINARY_DIR}/\$(Configuration)/mimalloc-redirect.dll" "$<TARGET_FILE_DIR:${target}>/"
      COMMENT "Copy mimalloc dlls to output directory")
  endforeach()
  if (JE2BE_CORE_USE_INTEL_COMPILER)
    foreach(target IN ITEMS je2be leveldb libdeflate_static zlibstatic mimalloc)
      set_target_properties(${target} PROPERTIES VS_PLATFORM_TOOLSET "Intel C++ Compiler 2023")
      target_compile_options(${target} PRIVATE /arch:SSE3)
    endforeach()
  endif()
endif()

list(APPEND all_files ${test_files} ${je2be_header_files} ${je2be_src_files} example/j2b.cpp example/b2j.cpp example/x2j.cpp example/dump.cpp test/j2b2j.cpp)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${all_files})
source_group(TREE ${libminecraft-file_SOURCE_DIR} PREFIX deps/mcfile FILES ${mcfile_header_files})
