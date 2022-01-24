#include <doctest/doctest.h>
#include <je2be.hpp>

using namespace std;
namespace fs = std::filesystem;

TEST_CASE("j2b2j") {
  fs::path thisFile(__FILE__);
  auto dataDir = thisFile.parent_path() / "data";
  auto in = dataDir / "je2be-test";
  auto tmp = mcfile::File::CreateTempDir(fs::temp_directory_path());
  CHECK(tmp);
  defer {
    fs::remove_all(*tmp);
  };

  // java -> bedrock
  auto outB = mcfile::File::CreateTempDir(*tmp);
  CHECK(outB);
  je2be::tobe::InputOption io;
  je2be::tobe::OutputOption oo;
  je2be::tobe::Converter tobe(in, io, *outB, oo);
  CHECK(tobe.run(thread::hardware_concurrency()));

  // bedrock -> java
  auto outJ = mcfile::File::CreateTempDir(*tmp);
  CHECK(outJ);
  je2be::toje::Converter toje(*outB, *outJ);
  CHECK(toje.run(thread::hardware_concurrency()));

  // Compare initial Java input and final Java output.
  //TODO:
}
