#pragma once

namespace j2b {

class WorldData {
public:
	explicit WorldData(std::filesystem::path const& input) : fInput(input), fJavaEditionMap(input) {}

	void put(DbInterface& db) {
		fPortals.putInto(db);
		fJavaEditionMap.each([this, &db](int32_t mapId) {
			Map::Convert(mapId, fInput, db);
		});
	}

private:
	std::filesystem::path fInput;

public:
	Portals fPortals;
	JavaEditionMap fJavaEditionMap;
};

}
