#pragma once

TEST_CASE("command") {
  using namespace je2be;

  CHECK(Command::TranspileJavaToBedrock("/function  function_namespace:function_name ") == "/function  function_namespace/function_name ");
  CHECK(Command::TranspileJavaToBedrock("  function  function_namespace:function_name\x0d") == "  function  function_namespace/function_name\x0d");
  CHECK(Command::TranspileJavaToBedrock("say \"function foo:bar\"") == "say \"function foo:bar\"");
  CHECK(Command::TranspileJavaToBedrock("say  \"incomplete quoated string") == "say  \"incomplete quoated string");
  CHECK(Command::TranspileJavaToBedrock("say \"function foo:bar\\\"\"") == "say \"function foo:bar\\\"\"");
  CHECK(Command::TranspileJavaToBedrock("  say \"foo#\" #  function foo:bar #baz") == "  say \"foo#\" #  function foo:bar #baz");
  CHECK(Command::TranspileJavaToBedrock("@e[x=1,dx=4,y=2,dy=5,z=3,dz=6]") == "@e[x=1,dx=4,y=2,dy=5,z=3,dz=6]");
  CHECK(Command::TranspileJavaToBedrock("@e[x=1,scores={foo=10,bar=1..5},y=3]") == "@e[x=1,scores={foo=10,bar=1..5},y=3]");

  CHECK(Command::TranspileJavaToBedrock("kill @e[type=zombie,distance=..10]") == "kill @e[type=zombie,r=10]");
  CHECK(Command::TranspileJavaToBedrock("kill @e[type=zombie,distance=10]") == "kill @e[type=zombie,rm=9,r=10]");
  CHECK(Command::TranspileJavaToBedrock("kill @e[type=zombie,distance=10.2]") == "kill @e[type=zombie,rm=10.2,r=10.2]");
  CHECK(Command::TranspileJavaToBedrock("kill @e[type=zombie,distance=10..]") == "kill @e[type=zombie,rm=9]");
  CHECK(Command::TranspileJavaToBedrock("kill @e[type=zombie,distance=10..10.2]") == "kill @e[type=zombie,rm=9,r=10.2]");
  CHECK(Command::TranspileJavaToBedrock("kill @e[type=zombie,distance=10.1..10]") == "kill @e[type=zombie,rm=10.1,r=10]");
  CHECK(Command::TranspileJavaToBedrock("kill @e[type=zombie,distance=10.1..10.2]") == "kill @e[type=zombie,rm=10.1,r=10.2]");

  CHECK(Command::TranspileJavaToBedrock("tp @a[gamemode=survival] @s") == "tp @a[m=survival] @s");
  CHECK(Command::TranspileJavaToBedrock("tp @a[gamemode=survival] @s") == "tp @a[m=survival] @s");
}
