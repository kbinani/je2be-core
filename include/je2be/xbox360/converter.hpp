#pragma once

#include <je2be/status.hpp>

namespace je2be::lce {

class Options;
class Progress;

} // namespace je2be::lce

namespace je2be::xbox360 {

class Converter {
  Converter() = delete;

public:
  static Status Run(std::filesystem::path const &inputSaveBin,
                    std::filesystem::path const &outputDirectory,
                    unsigned int concurrency,
                    je2be::lce::Options const &options,
                    je2be::lce::Progress *progress = nullptr);
};

} // namespace je2be::xbox360
