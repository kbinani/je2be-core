#include <je2be.hpp>

class OnMemoryDb : public j2b::DbInterface {
public:
  bool valid() const override {
    return true;
  }

  void put(std::string const &key, leveldb::Slice const &value) override {
    fStorage[key] = value.ToString();
  }

  void del(std::string const &key) override {
    fStorage.erase(key);
  }

public:
  std::unordered_map<std::string, std::string> fStorage;
};
