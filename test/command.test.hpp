#pragma once

TEST_CASE("command") {
  using namespace je2be::tobe;
  CHECK(Command::Transpile("/function  function_namespace:function_name ") == "/function  function_namespace/function_name ");
  CHECK(Command::Transpile("  function  function_namespace:function_name\x0d") == "  function  function_namespace/function_name\x0d");
  CHECK(Command::Transpile("say \"function foo:bar\"") == "say \"function foo:bar\"");
  CHECK(Command::Transpile("say  \"incomplete quoated string") == "say  \"incomplete quoated string");
  CHECK(Command::Transpile("say \"function foo:bar\\\"\"") == "say \"function foo:bar\\\"\"");
  CHECK(Command::Transpile("  say \"foo#\" #  function foo:bar #baz") == "  say \"foo#\" #  function foo:bar #baz");
}
