#pragma once

namespace j2b {

class WorldData {
public:
	explicit WorldData(std::filesystem::path const& input) : fInput(input), fJavaEditionMap(input) {}

	void put(DbInterface& db) {
		fPortals.putInto(db);
		fJavaEditionMap.each([this, &db](int32_t mapId) {
			auto found = fMapItems.find(mapId);
			if (found == fMapItems.end()) return;
			Map::Convert(mapId, *found->second, fInput, db);
		});
	}

private:
	std::filesystem::path fInput;

public:
	Portals fPortals;
	JavaEditionMap fJavaEditionMap;
	std::unordered_map<int32_t, std::shared_ptr<mcfile::nbt::CompoundTag>> fMapItems;
};

}
