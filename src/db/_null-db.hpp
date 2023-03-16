#pragma once

namespace je2be {

class NullDb : public DbInterface {
public:
  bool valid() const override { return true; }
  Status put(std::string const &key, leveldb::Slice const &value) override { return Status::Ok(); }
  Status del(std::string const &key) override { return Status::Ok(); }
  Status close(std::function<void(Rational<u64> const &progress)> progress = nullptr) override { return Status::Ok(); }
  void abandon() override{};
};

} // namespace je2be
