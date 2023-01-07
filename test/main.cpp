#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

// clang-format off

#include <iostream>

#include "je2be-all.hpp"

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#include <shlobj_core.h>
#endif

#include "CheckTag.hpp"

using namespace std;
using namespace mcfile;
using namespace je2be;
namespace fs = std::filesystem;

// clang-format off

#include "block-data.test.hpp"
#include "command.test.hpp"
#include "j2b2j.hpp"
#include "j2b2j.test.hpp"
#include "research.hpp"
#include "moving-piston.test.hpp"
#include "shoulder-riders.test.hpp"
#include "uuid.test.hpp"
#include "volume.test.hpp"
#include "loot-table.test.hpp"
#include "bee-nest.test.hpp"
#include "end-gateway.test.hpp"
#include "strings.test.hpp"
