#pragma once

#include <functional>
#include <optional>
#include <string>

namespace leveldb {
class Slice;
}

namespace je2be {

class DbInterface {
public:
  virtual ~DbInterface() {}
  virtual bool valid() const = 0;
  virtual void put(std::string const &key, leveldb::Slice const &value) = 0;
  virtual void del(std::string const &key) = 0;
  virtual bool close(std::function<void(double progress)> progress = nullptr) = 0;
  virtual void abandon() = 0;
};

} // namespace je2be
