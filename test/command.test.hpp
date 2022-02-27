#pragma once

TEST_CASE("command") {
  using namespace je2be;
  CHECK(Command::TranspileJavaToBedrock("/function  function_namespace:function_name ") == "/function  function_namespace/function_name ");
  CHECK(Command::TranspileJavaToBedrock("  function  function_namespace:function_name\x0d") == "  function  function_namespace/function_name\x0d");
  CHECK(Command::TranspileJavaToBedrock("say \"function foo:bar\"") == "say \"function foo:bar\"");
  CHECK(Command::TranspileJavaToBedrock("say  \"incomplete quoated string") == "say  \"incomplete quoated string");
  CHECK(Command::TranspileJavaToBedrock("say \"function foo:bar\\\"\"") == "say \"function foo:bar\\\"\"");
  CHECK(Command::TranspileJavaToBedrock("  say \"foo#\" #  function foo:bar #baz") == "  say \"foo#\" #  function foo:bar #baz");
}
