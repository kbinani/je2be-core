#pragma once

namespace je2be::toje {

class Converter {
public:
  Converter(std::filesystem::path in, std::filesystem::path out)
      : fIn(in), fOut(out) {
  }

  bool run(unsigned concurrency) {
    return false;
  }

private:
  std::filesystem::path fIn;
  std::filesystem::path fOut;
};

} // namespace je2be::toje
