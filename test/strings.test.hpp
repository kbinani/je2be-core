TEST_CASE("strings") {
  using namespace std;

  SUBCASE("Split") {
    SUBCASE("basic") {
      vector<u8string> result;
      strings::Split(u8"Pillager_Outpost", u8"_", [&result](int idx, u8string const &s) { result.push_back(s); return true; });
      CHECK(result.size() == 2);
      CHECK(result[0] == u8"Pillager");
      CHECK(result[1] == u8"Outpost");
    }
    SUBCASE("empty") {
      vector<u8string> result;
      strings::Split(u8"Pillager_Outpost", u8"", [&result](int idx, u8string const &s) { result.push_back(s); return true; });
      CHECK(result.size() == 1);
      CHECK(result[0] == u8"Pillager_Outpost");
    }
  }
  SUBCASE("CapitalizeSnake") {
    CHECK(strings::CapitalizeSnake(u8"pillager_outpost") == u8"Pillager_Outpost");
  }
  SUBCASE("Increment") {
    CHECK(strings::Increment("") == "\x1");
    CHECK(strings::Increment("\x1") == "\x2");
    CHECK(strings::Increment("\xff") == string("\x1\x0", 2));
    CHECK(strings::Increment("\x1\xff\xff") == string("\x2\x0\x0", 3));
  }
  SUBCASE("SnakeFromUpperCamel") {
    CHECK(strings::SnakeFromUpperCamel(u8"Ender_Chest") == u8"ender_chest");
  }
}
