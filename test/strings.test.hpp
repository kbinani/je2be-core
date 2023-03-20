TEST_CASE("strings") {
  using namespace std;

  SUBCASE("Split") {
    SUBCASE("basic") {
      vector<string> result;
      strings::Split("Pillager_Outpost", "_", [&result](int idx, string const &s) { result.push_back(s); return true; });
      CHECK(result.size() == 2);
      CHECK(result[0] == "Pillager");
      CHECK(result[1] == "Outpost");
    }
    SUBCASE("empty") {
      vector<string> result;
      strings::Split("Pillager_Outpost", "", [&result](int idx, string const &s) { result.push_back(s); return true; });
      CHECK(result.size() == 1);
      CHECK(result[0] == "Pillager_Outpost");
    }
  }
  SUBCASE("CapitalizeSnake") {
    CHECK(strings::CapitalizeSnake("pillager_outpost") == "Pillager_Outpost");
  }
  SUBCASE("Increment") {
    CHECK(strings::Increment("") == "\x1");
    CHECK(strings::Increment("\x1") == "\x2");
    CHECK(strings::Increment("\xff") == string("\x1\x0", 2));
    CHECK(strings::Increment("\x1\xff\xff") == string("\x2\x0\x0", 3));
  }
}
