#include <doctest/doctest.h>
#include <je2be.hpp>

using namespace j2b;
using namespace mcfile;
using namespace std;
namespace fs = std::filesystem;

TEST_CASE("moving-piston") {
  fs::path thisFile(__FILE__);

  for (string extending : {"extending=0", "extending=1", "normal_extending=1"}) {
    SUBCASE(extending.c_str()) {
      fs::path rootDir = thisFile.parent_path() / "data" / "piston" / extending;
      World world(rootDir.string());
      auto region = world.region(0, 0);
      auto chunk = region->chunkAt(0, 0);
      MovingPiston::PreprocessChunk(chunk, *region);
    }
  }
}
