#pragma once

namespace je2be {

class NullDb : public DbInterface {
public:
  bool valid() const override { return true; }
  void put(std::string const &key, leveldb::Slice const &value) override {}
  void del(std::string const &key) override {}
  bool close(std::function<void(Rational<u64> const &progress)> progress = nullptr) override { return true; }
  void abandon() override{};
};

} // namespace je2be
