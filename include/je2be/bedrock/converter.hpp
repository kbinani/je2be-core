#pragma once

#include <je2be/status.hpp>

namespace je2be::bedrock {

class Options;
class Progress;

class Converter {
  class Impl;
  Converter() = delete;

public:
  static Status Run(std::filesystem::path const &input, std::filesystem::path const &output, Options const &o, unsigned concurrency, Progress *progress = nullptr);
};

} // namespace je2be::bedrock
