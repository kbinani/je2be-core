#pragma once

TEST_CASE("command") {
  using namespace je2be::command;
  using namespace std;

  unordered_map<string, string> expected = {
      {"/function  function_namespace:function_name ", "/function  function_namespace/function_name "},
      {"  function  function_namespace:function_name\x0d", "  function  function_namespace/function_name\x0d"},
      {"say \"function foo:bar\" #foo", "say \"function foo:bar\" #foo"},
      {"say  \"incomplete quoated string", "say  \"incomplete quoated string"},
      {R"(say "incomplete quoted string with function/something @e[type=zombie,distance=..2])", R"(say "incomplete quoted string with function/something @e[type=zombie,distance=..2])"},
      {"say \"function foo:bar\\\"\"", "say \"function foo:bar\\\"\""},
      {"  say \"foo#\" #  function foo:bar #baz", "  say \"foo#\" #  function foo:bar #baz"},
      {"@e[x=1,dx=4,y=2,dy=5,z=3,dz=6]", "@e[x=1,dx=4,y=2,dy=5,z=3,dz=6]"},
      {"@e[x=1,scores={foo=10,bar=1..5},y=3]", "@e[x=1,scores={foo=10,bar=1..5},y=3]"},

      {"kill @e[type=zombie,distance=..10]", "kill @e[type=zombie,r=10]"},
      {"kill @e[type=zombie,distance=10]", "kill @e[type=zombie,rm=10,r=10]"},
      {"kill @e[type=zombie,distance=10.2]", "kill @e[type=zombie,rm=10.2,r=10.2]"},
      {"kill @e[type=zombie,distance=10..]", "kill @e[type=zombie,rm=10]"},
      {"kill @e[type=zombie,distance=10..10.2]", "kill @e[type=zombie,rm=10,r=10.2]"},
      {"kill @e[type=zombie,distance=10.1..10]", "kill @e[type=zombie,rm=10.1,r=10]"},
      {"kill @e[type=zombie,distance=10.1..10.2]", "kill @e[type=zombie,rm=10.1,r=10.2]"},

      {"tp @a[gamemode=survival] @s", "tp @a[m=survival] @s"},
      {"tp @a[gamemode=survival] @s", "tp @a[m=survival] @s"},

      {R"(tp @a[team="@e[type=foo,distance=1]",limit=1] @s)", R"(tp @a[team="@e[type=foo,distance=1]",c=1] @s)"},
      {"tp @a[distance=1..invalid]", "tp @a[distance=1..invalid]"},
  };
  for (auto const &it : expected) {
    CHECK(Command::TranspileJavaToBedrock(it.first) == it.second);
    CHECK(Command::TranspileBedrockToJava(it.second) == it.first);
  }
}
