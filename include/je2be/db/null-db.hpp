#pragma once

namespace j2b {

class NullDb : public DbInterface {
public:
    bool valid() const override { return true;  }
    void put(std::string const& key, leveldb::Slice const& value) override {}
    void del(std::string const& key) override {}
};

}
