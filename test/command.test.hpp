#pragma once

TEST_CASE("command") {
  using namespace je2be::command;
  using namespace std;

  unordered_map<u8string, u8string> expected = {
      {u8"/function  function_namespace:function_name ", u8"/function  function_namespace/function_name "},
      {u8"  function  function_namespace:function_name\x0d", u8"  function  function_namespace/function_name\x0d"},
      {u8"say \"function foo:bar\" #foo", u8"say \"function foo:bar\" #foo"},
      {u8"say  \"incomplete quoated string", u8"say  \"incomplete quoated string"},
      {u8R"(say "incomplete quoted string with function/something @e[type=zombie,distance=..2])", u8R"(say "incomplete quoted string with function/something @e[type=zombie,distance=..2])"},
      {u8"say \"function foo:bar\\\"\"", u8"say \"function foo:bar\\\"\""},
      {u8"  say \"foo#\" #  function foo:bar #baz", u8"  say \"foo#\" #  function foo:bar #baz"},
      {u8"@e[x=1,dx=4,y=2,dy=5,z=3,dz=6]", u8"@e[x=1,dx=4,y=2,dy=5,z=3,dz=6]"},
      {u8"@e[x=1,scores={foo=10,bar=1..5},y=3]", u8"@e[x=1,scores={foo=10,bar=1..5},y=3]"},

      {u8"kill @e[type=zombie,distance=..10]", u8"kill @e[type=zombie,r=10]"},
      {u8"kill @e[type=zombie,distance=10]", u8"kill @e[type=zombie,rm=10,r=10]"},
      {u8"kill @e[type=zombie,distance=10.2]", u8"kill @e[type=zombie,rm=10.2,r=10.2]"},
      {u8"kill @e[type=zombie,distance=10..]", u8"kill @e[type=zombie,rm=10]"},
      {u8"kill @e[type=zombie,distance=10..10.2]", u8"kill @e[type=zombie,rm=10,r=10.2]"},
      {u8"kill @e[type=zombie,distance=10.1..10]", u8"kill @e[type=zombie,rm=10.1,r=10]"},
      {u8"kill @e[type=zombie,distance=10.1..10.2]", u8"kill @e[type=zombie,rm=10.1,r=10.2]"},

      {u8"tp @a[gamemode=survival] @s", u8"tp @a[m=survival] @s"},

      {u8R"(tp @a[team="@e[type=foo,distance=1]",limit=1] @s)", u8R"(tp @a[team="@e[type=foo,distance=1]",c=1] @s)"},
      {u8"tp @a[distance=1..invalid]", u8"tp @a[rm=1,r=invalid]"},
  };
  for (auto const &it : expected) {
    CHECK(Command::TranspileJavaToBedrock(it.first) == it.second);
    CHECK(Command::TranspileBedrockToJava(it.second) == it.first);
  }

  SUBCASE("token") {
    vector<pair<u8string, bool>> actual;
    Token::IterateStringLiterals(u8R"(/give @p written_book{pages:['{"text":"\\"a\\""}']})", [&actual](u8string s, bool literal) {
      actual.push_back(make_pair(s, literal));
    });
    REQUIRE(actual.size() == 3);
    CHECK(actual[0].first == u8R"(/give @p written_book{pages:[)");
    CHECK(actual[0].second == false);
    CHECK(actual[1].first == u8R"('{"text":"\\"a\\""}')");
    CHECK(actual[1].second == true);
    CHECK(actual[2].first == u8R"(]})");
    CHECK(actual[2].second == false);
  }
}
