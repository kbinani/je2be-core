#pragma once

namespace je2be::toje {

class Converter {
public:
  Converter(std::string const &input, std::string const &output) = delete;
  Converter(std::string const &input, std::wstring const &output) = delete;
  Converter(std::wstring const &input, std::string const &output) = delete;
  Converter(std::wstring const &input, std::wstring const &output) = delete;
  Converter(std::filesystem::path const &input, std::filesystem::path const &output)
      : fInput(input), fOutput(output) {
  }

  bool run(unsigned concurrency) {
    return false;
  }

private:
  std::filesystem::path fInput;
  std::filesystem::path fOutput;
};

} // namespace je2be::toje
