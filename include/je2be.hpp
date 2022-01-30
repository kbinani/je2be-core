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

#if __has_include(<windows.h>)
#define NOMINMAX
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

#include <je2be/fs.hpp>
#include <je2be/scoped-file.hpp>
#include <je2be/file.hpp>
#include <je2be/strings.hpp>
#include <je2be/pos2.hpp>
#include <je2be/pos3.hpp>
#include <je2be/volume.hpp>
#include <je2be/optional.hpp>
#include <je2be/vec.hpp>
#include <je2be/rotation.hpp>
#include <je2be/color/rgba.hpp>
#include <je2be/color/lab.hpp>
#include <je2be/size.hpp>
#include <je2be/xxhash.hpp>
#include <je2be/uuid.hpp>
#include <je2be/props.hpp>
#include <je2be/progress.hpp>
#include <je2be/version.hpp>
#include <je2be/future-support.hpp>
#include <je2be/defer.hpp>

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

#include <je2be/tobe/tobe.hpp>
#include <je2be/toje/toje.hpp>
