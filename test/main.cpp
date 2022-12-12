#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <je2be.hpp>

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
#include "j2b2j.test.hpp"
#include "research.hpp"
#include "moving-piston.test.hpp"
#include "shoulder-riders.test.hpp"
#include "uuid.test.hpp"
#include "volume.test.hpp"
#include "loot-table.test.hpp"
#include "bee-nest.test.hpp"
#include "end-gateway.test.hpp"

// clang-format on

#include <catch2/catch_session.hpp>
#include <catch2/internal/catch_compiler_capabilities.hpp>
#include <catch2/internal/catch_config_wchar.hpp>
#include <catch2/internal/catch_leak_detector.hpp>
#include <catch2/internal/catch_platform.hpp>

namespace Catch {
CATCH_INTERNAL_START_WARNINGS_SUPPRESSION
CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS
static LeakDetector leakDetector;
CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION
} // namespace Catch

int main(int argc, char *argv[]) {
  (void)&Catch::leakDetector;
  return Catch::Session().run(argc, argv);
}
