#include <doctest/doctest.h>
#include <je2be.hpp>

#include "on-memory-db.hpp"

using namespace j2b;
using namespace mcfile;
using namespace std;
namespace fs = std::filesystem;

TEST_CASE("converter") {
  fs::path thisFile(__FILE__);

  for (string extending : {"extending=0", "extending=1"}) {
    SUBCASE(extending.c_str()) {
      fs::path rootDir = thisFile.parent_path() / "data" / "piston" / extending;
      Converter converter("dummy", {}, "dummy", {});
      OnMemoryDb db;
      World world(rootDir.string());
      auto region = world.region(0, 0);
      auto chunk = region->chunkAt(0, 0);
      JavaEditionMap map("dummy", {});
      converter.putChunk(*chunk, Dimension::Overworld, db, map, *region);
      CHECK(db.fStorage.size() == 5);
    }
  }
}
