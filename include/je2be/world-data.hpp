#pragma once

namespace j2b {

class WorldData {
public:
	void put(DbInterface& db, std::filesystem::path const& input) {
		fPortals.putInto(db);
		for (auto id : fMapIdList) {
			Map::Convert(id, input, db);
		}
	}

public:
	Portals fPortals;
	std::unordered_set<int32_t> fMapIdList;
};

}
