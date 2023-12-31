#pragma once

#include <je2be/integers.hpp>
#include <je2be/rational.hpp>
#include <je2be/status.hpp>

#include <functional>
#include <optional>
#include <string>

namespace leveldb {
class Slice;
}

namespace je2be {

class DbInterface {
public:
  virtual ~DbInterface() = default;
  virtual bool valid() const = 0;
  virtual Status put(std::string const &key, leveldb::Slice const &value) = 0;
  virtual Status del(std::string const &key) = 0;
  virtual Status close(std::function<bool(Rational<u64> const &progress)> progress = nullptr) = 0;
  virtual void abandon() = 0;
};

} // namespace je2be
