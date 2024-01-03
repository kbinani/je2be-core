#pragma once

#include <je2be/status.hpp>

namespace je2be::lce {

class Options;
class Progress;

class Converter {
  class Impl;
  Converter() = delete;

public:
  static Status Run(std::filesystem::path const &inputSaveBin,
                    std::filesystem::path const &outputDirectory,
                    unsigned int concurrency,
                    Options const &options,
                    Progress *progress = nullptr);
};

} // namespace je2be::lce
