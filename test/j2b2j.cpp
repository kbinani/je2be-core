#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <BS_thread_pool.hpp>
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

#include "je2be-all.hpp"

#include "j2b2j.hpp"

fs::path gInput;

TEST_CASE("j2b2j") {
  REQUIRE(fs::exists(gInput));
  auto tmp = mcfile::File::CreateTempDir(fs::temp_directory_path());
  REQUIRE(tmp);
  defer {
    fs::remove_all(*tmp);
  };
  fs::path in;
  if (fs::is_regular_file(gInput)) {
    auto unzip = mcfile::File::CreateTempDir(*tmp);
    REQUIRE(unzip);
    REQUIRE(ZipFile::Unzip(gInput, *unzip));
    bool found = false;
    for (auto it : fs::recursive_directory_iterator(*unzip)) {
      auto path = it.path();
      if (!fs::is_regular_file(path)) {
        continue;
      }
      if (!path.has_filename()) {
        continue;
      }
      if (path.filename().string() == "level.dat") {
        in = path.parent_path();
        found = true;
        break;
      }
    }
    REQUIRE(found);
  } else if (fs::is_directory(gInput)) {
    in = gInput;
  } else {
    REQUIRE(false);
    return;
  }
  TestJavaToBedrockToJava(in);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << "Error: too few argument" << endl;
    cerr << "Usage:" << endl;
    cerr << "    j2b2j <world directory or zip archive>" << endl;
    return -1;
  }
  gInput = fs::path(argv[1]);

  doctest::Context context;
  return context.run();
}
