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
      /ZI
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
  GIT_TAG ed92e06a8726993df399bf5f03264ef15c9c536a)

CPMAddPackage(
  NAME defer
  GITHUB_REPOSITORY kbinani/defer
  GIT_TAG b1fa5b1d3d5886c7d07487499febc8d77095d170)

set(je2be_header_files
  #begin je2be-header files
  include/je2be.hpp
  include/je2be/bedrock/converter.hpp
  include/je2be/bedrock/options.hpp
  include/je2be/bedrock/progress.hpp
  include/je2be/enums/level-directory-structure.hpp
  include/je2be/errno.hpp
  include/je2be/fs.hpp
  include/je2be/integers.hpp
  include/je2be/java/converter.hpp
  include/je2be/java/options.hpp
  include/je2be/java/progress.hpp
  include/je2be/lce/behavior.hpp
  include/je2be/lce/converter.hpp
  include/je2be/lce/options.hpp
  include/je2be/lce/progress.hpp
  include/je2be/nbt.hpp
  include/je2be/pos2.hpp
  include/je2be/ps3/converter.hpp
  include/je2be/rational.hpp
  include/je2be/status.hpp
  include/je2be/strings.hpp
  include/je2be/uuid.hpp
  include/je2be/xbox360/converter.hpp
  include/je2be/xbox360/minecraft-save-info.hpp
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
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/je/region-compression.hpp
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
  ${libminecraft-file_SOURCE_DIR}/include/mcfile/xxhash.hpp
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
  src/bedrock-block-data.cpp
  src/bedrock-block-entity-before-components-introduced.hpp
  src/bedrock-block-entity.cpp
  src/bedrock-chunk.cpp
  src/bedrock-context.cpp
  src/bedrock-converter.cpp
  src/bedrock-entity-before-components-introduced.hpp
  src/bedrock-entity.cpp
  src/bedrock-item-before-components-introduced.hpp
  src/bedrock-item.cpp
  src/bedrock-region.cpp
  src/bedrock-sub-chunk.cpp
  src/bedrock-world.cpp
  src/bedrock/_block-accessor-wrapper.hpp
  src/bedrock/_block-data.hpp
  src/bedrock/_block-entity-convert-result.hpp
  src/bedrock/_block-entity.hpp
  src/bedrock/_chunk.hpp
  src/bedrock/_constants.hpp
  src/bedrock/_context.hpp
  src/bedrock/_entity.hpp
  src/bedrock/_item.hpp
  src/bedrock/_legacy-block.hpp
  src/bedrock/_level-data.hpp
  src/bedrock/_map-info.hpp
  src/bedrock/_region.hpp
  src/bedrock/_structure-info.hpp
  src/bedrock/_sub-chunk.hpp
  src/bedrock/_world.hpp
  src/bedrock/terraform/_beacon.hpp
  src/bedrock/terraform/_campfire.hpp
  src/bedrock/terraform/_cave-vines.hpp
  src/bedrock/terraform/_double_plant.hpp
  src/bedrock/terraform/_piston.hpp
  src/bedrock/terraform/_tripwire.hpp
  src/bedrock/terraform/_twisting-vines.hpp
  src/bedrock/terraform/_weeping-vines.hpp
  src/block/_trial-spawner.hpp
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
  src/entity/_entity-attributes.hpp
  src/entity/_frog.hpp
  src/entity/_painting.hpp
  src/entity/_panda.hpp
  src/entity/_tropical-fish.hpp
  src/entity/_wolf.hpp
  src/enums/_banner-color-code-bedrock.hpp
  src/enums/_chunk-conversion-mode.hpp
  src/enums/_color-code-java.hpp
  src/enums/_effect.hpp
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
  src/item/_map-type.hpp
  src/item/_potion.hpp
  src/item/_tipped-arrow-potion.hpp
  src/java-block-data.cpp
  src/java-chunk-data-package.cpp
  src/java-chunk.cpp
  src/java-converter.cpp
  src/java-entity.cpp
  src/java-item.cpp
  src/java-map.cpp
  src/java-moving-piston.cpp
  src/java-region.cpp
  src/java-sub-chunk.cpp
  src/java-tile-entity.cpp
  src/java-world.cpp
  src/java/_axolotl.hpp
  src/java/_biome-map-legacy.hpp
  src/java/_block-data.hpp
  src/java/_block-palette.hpp
  src/java/_chunk-data-package.hpp
  src/java/_chunk-data.hpp
  src/java/_chunk.hpp
  src/java/_components.hpp
  src/java/_context.hpp
  src/java/_datapacks.hpp
  src/java/_enchant-data.hpp
  src/java/_entity-store.hpp
  src/java/_entity.hpp
  src/java/_flat-world-layers.hpp
  src/java/_height-map.hpp
  src/java/_item.hpp
  src/java/_java-edition-map.hpp
  src/java/_level-data.hpp
  src/java/_level.hpp
  src/java/_map.hpp
  src/java/_moving-piston.hpp
  src/java/_player-abilities.hpp
  src/java/_region.hpp
  src/java/_session-lock.hpp
  src/java/_sub-chunk.hpp
  src/java/_tile-entity.hpp
  src/java/_uuid-registrar.hpp
  src/java/_versions.hpp
  src/java/_world-data.hpp
  src/java/_world.hpp
  src/java/portal/_oriented-portal-blocks.hpp
  src/java/portal/_portal-blocks.hpp
  src/java/portal/_portal.hpp
  src/java/portal/_portals.hpp
  src/java/structure/_structure-piece-collection.hpp
  src/java/structure/_structures.hpp
  src/lce-block-data.cpp
  src/lce-chunk.cpp
  src/lce-converter.cpp
  src/lce-entity.cpp
  src/lce-item.cpp
  src/lce-minecraft-save-info.cpp
  src/lce-savegame.cpp
  src/lce-terraform.cpp
  src/lce-tile-entity.cpp
  src/lce-world.cpp
  src/lce/_attribute.hpp
  src/lce/_biome.hpp
  src/lce/_block-data.hpp
  src/lce/_chunk.hpp
  src/lce/_context.hpp
  src/lce/_entity.hpp
  src/lce/_grid.hpp
  src/lce/_item.hpp
  src/lce/_lzx-decoder.hpp
  src/lce/_savegame.hpp
  src/lce/_terraform.hpp
  src/lce/_tile-entity-convert-result.hpp
  src/lce/_tile-entity.hpp
  src/lce/_world.hpp
  src/ps3-converter.cpp
  src/ps3/_behavior.hpp
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
  src/terraform/_door.hpp
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
  src/terraform/java/_block-accessor-java-directory.hpp
  src/terraform/java/_block-accessor-java-mca.hpp
  src/terraform/java/_block-accessor-java.hpp
  src/terraform/lighting/_chunk-light-cache.hpp
  src/terraform/lighting/_chunk-lighting-model.hpp
  src/terraform/lighting/_light-cache.hpp
  src/terraform/lighting/_lighting-model.hpp
  src/terraform/lighting/_lighting.hpp
  src/terraform/xbox360/_attached-stem.hpp
  src/terraform/xbox360/_bed.hpp
  src/terraform/xbox360/_block-accessor-box360.hpp
  src/terraform/xbox360/_chest.hpp
  src/terraform/xbox360/_kelp.hpp
  src/terraform/xbox360/_nether-portal.hpp
  src/tile-entity/_beacon.hpp
  src/tile-entity/_loot-table.hpp
  src/xbox360-converter.cpp
  src/xbox360-save-bin.cpp
  src/xbox360/_behavior.hpp
  src/xbox360/_save-bin.hpp
  src/xbox360/_stfs-ext.hpp
  src/xbox360/_stfs.hpp
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
  test/bedrock-legacy-block.test.hpp
  test/parallel.test.hpp)

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

list(APPEND je2be_include_directories ${LZ4_BINARY_DIR}/lib)

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
  VERSION 3.11.3
  # the git repo is incredibly large, so we download the archived include directory
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/include.zip
  URL_HASH SHA256=a22461d13119ac5c78f205d3df1db13403e58ce1bb1794edc9313677313f4a9d)

if (nlohmann_json_ADDED)
  add_library(nlohmann_json INTERFACE IMPORTED)
  target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/include)
endif()

CPMAddPackage(
  NAME minizip
  GITHUB_REPOSITORY zlib-ng/minizip-ng
  GIT_TAG 4.0.4
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

list(APPEND je2be_link_libraries mcfile leveldb minizip nlohmann_json defer)
list(APPEND je2be_exe_link_libraries je2be)

if (MSVC)
  CPMAddPackage(
    NAME mimalloc
    GITHUB_REPOSITORY microsoft/mimalloc
    VERSION 2.1.2
    OPTIONS "MI_OVERRIDE ON"
            "MI_BUILD_TESTS OFF"
            "MI_BUILD_OBJECT OFF"
            "MI_BUILD_STATIC OFF"
            "MI_WIN_REDIRECT ON")
  list(APPEND je2be_exe_link_libraries mimalloc)
endif()

CPMAddPackage(
  NAME pbar
  GIT_TAG 226b9ad291d72c6859456d5188f33c932e2e730b
  GIT_REPOSITORY https://github.com/estshorter/pbar.git)

CPMAddPackage(
  NAME sparse
  GIT_TAG c3b933bb70817a6e20fa5ef23c89c2afcb5c0963
  GIT_REPOSITORY https://github.com/Arzaghi/sparse.git)

add_library(je2be STATIC ${je2be_header_files} ${mcfile_header_files} ${je2be_src_files})
target_include_directories(je2be PUBLIC
  ./include
  ${leveldb_SOURCE_DIR}
  ${leveldb_SOURCE_DIR}/include
  ${minizip_SOURCE_DIR}
  ${nlohmann_json_SOURCE_DIR}/include
  ${sparse_SOURCE_DIR}
  ${je2be_include_directories})
target_include_directories(je2be PRIVATE ./src ${libdeflate_SOURCE_DIR} ${libminecraft-file_SOURCE_DIR}/include ${defer_SOURCE_DIR}/include)
target_link_libraries(je2be INTERFACE ${je2be_link_libraries})
target_link_directories(je2be INTERFACE ${je2be_link_directories})

add_executable(j2b example/j2b.cpp ${je2be_header_files})
target_link_libraries(j2b ${je2be_exe_link_libraries} cxxopts)
target_include_directories(j2b PRIVATE ${pbar_SOURCE_DIR})

add_executable(b2j example/b2j.cpp ${je2be_header_files})
target_link_libraries(b2j ${je2be_exe_link_libraries} cxxopts)
target_include_directories(b2j PRIVATE ${pbar_SOURCE_DIR})

add_executable(x2j example/x2j.cpp example/lce-progress.hpp ${je2be_header_files})
target_link_libraries(x2j ${je2be_exe_link_libraries} cxxopts)
target_include_directories(x2j PRIVATE ${pbar_SOURCE_DIR})

add_executable(p2j example/p2j.cpp example/lce-progress.hpp ${je2be_header_files})
target_link_libraries(p2j ${je2be_exe_link_libraries} cxxopts)
target_include_directories(p2j PRIVATE ${pbar_SOURCE_DIR})

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

list(APPEND all_files ${test_files} ${je2be_header_files} ${je2be_src_files} example/j2b.cpp example/b2j.cpp example/lce-progress.hpp example/x2j.cpp example/p2j.cpp example/dump.cpp test/j2b2j.cpp)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${all_files})
source_group(TREE ${libminecraft-file_SOURCE_DIR} PREFIX deps/mcfile FILES ${mcfile_header_files})
