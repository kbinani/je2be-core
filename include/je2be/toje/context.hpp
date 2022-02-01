#pragma once

namespace je2be::toje {

class Context {
public:
  void addMap(int index, int64_t uuid) {
    fMaps[index] = uuid;
  }

  void mergeInto(Context &other) const {
    for (auto it : fMaps) {
      other.fMaps[it.first] = it.second;
    }
  }

  bool postProcess(std::filesystem::path root, leveldb::DB &db) {
    //TODO:
    return true;
  }

public:
  std::unordered_map<int, int64_t> fMaps;
};

} // namespace je2be::toje
