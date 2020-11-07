#pragma once

namespace j2b {

class DimensionDataFragment {
public:
  explicit DimensionDataFragment(Dimension dim) : fDim(dim) {}

  void addPortalBlock(int32_t x, int32_t y, int32_t z, bool xAxis) {
    fPortalBlocks.add(x, y, z, xAxis);
  }

  void addMap(int32_t javaMapId,
              std::shared_ptr<mcfile::nbt::CompoundTag> const &item) {
    fMapItems[javaMapId] = item;
  }

  void
  addAutonomousEntity(std::shared_ptr<mcfile::nbt::CompoundTag> const &entity) {
    fAutonomousEntities.push_back(entity);
  }

  void drain(WorldData &wd) {
    wd.fPortals.add(fPortalBlocks, fDim);
    for (auto const &it : fMapItems) {
      wd.fMapItems[it.first] = it.second;
    }
    for (auto const &e : fAutonomousEntities) {
      wd.fAutonomousEntities.push_back(e);
    }
  }

public:
  Dimension const fDim;

private:
  PortalBlocks fPortalBlocks;
  std::unordered_map<int32_t, std::shared_ptr<mcfile::nbt::CompoundTag>>
      fMapItems;
  std::vector<std::shared_ptr<mcfile::nbt::CompoundTag>> fAutonomousEntities;
};

} // namespace j2b
