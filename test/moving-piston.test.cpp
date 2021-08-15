#include <doctest/doctest.h>
#include <je2be.hpp>

using namespace j2b;
using namespace mcfile;
using namespace std;
namespace fs = std::filesystem;

static std::tuple<shared_ptr<Chunk>, shared_ptr<Region>> Load(string type) {
  fs::path thisFile(__FILE__);
  World world((thisFile.parent_path() / "data" / "piston" / type).string());
  auto region = world.region(0, 0);
  auto chunk = region->chunkAt(0, 0);
  return make_pair(chunk, region);
}

TEST_CASE("moving-piston") {
  SUBCASE("extending=0") {
    auto [chunk, region] = Load("extending=0");
    MovingPiston::PreprocessChunk(chunk, *region);
  }

  SUBCASE("extending=1") {
    auto [chunk, region] = Load("extending=1");
    MovingPiston::PreprocessChunk(chunk, *region);
  }

  SUBCASE("normal_extending=1") {
    auto [chunk, region] = Load("normal_extending=1");
    MovingPiston::PreprocessChunk(chunk, *region);

    int a = 0;
  }
}
