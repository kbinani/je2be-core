#pragma once

namespace j2b {

class Map {
    using CompoundTag = mcfile::nbt::CompoundTag;

public:
	static int64_t UUID(int32_t javaMapId) {
        XXH64_state_t* state = XXH64_createState();
        XXH64_hash_t seed = 0;
        XXH64_reset(state, seed);
        XXH64_update(state, &javaMapId, sizeof(javaMapId));
        XXH64_hash_t hash = XXH64_digest(state);
        XXH64_freeState(state);
        return *(int64_t*)&hash;
	}

    static bool Convert(int32_t javaMapId, std::filesystem::path const& input, DbInterface &db) {
        using namespace std;
        namespace fs = std::filesystem;

        auto jeFilePath = input / "data" / ("map_" + to_string(javaMapId) + ".dat");
        if (!fs::is_regular_file(jeFilePath)) return false;

        int64_t uuid = UUID(javaMapId);
        auto ret = make_shared<CompoundTag>();

        //TODO:

        return false;
    }

private:
	Map() = delete;
};

}
