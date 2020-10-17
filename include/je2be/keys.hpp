#pragma once

namespace j2e {

class keys {
public:
	static leveldb::Slice SubChunk(int32_t chunkX, int32_t chunkY, int32_t chunkZ, Dimension dim) {
		size_t const size = dim == Dimension::Overworld ? 10 : 14;
		std::vector<char> b(size);
		*(uint32_t*)b.data() = mcfile::detail::Int32LEFromNative(*(uint32_t*)&chunkX);
		*(uint32_t*)(b.data() + 4) = mcfile::detail::Int32LEFromNative(*(uint32_t*)&chunkZ);
		uint8_t const tag = 0x2f;
		uint8_t const y = (uint8_t)(std::min)((std::max)(chunkY, 0), 255);
		if (dim == Dimension::Overworld) {
			// xxxxzzzzty; t = 0x2f
			b[8] = tag;
			b[9] = y;
		} else {
			// xxxxzzzzddddty; t = 0x2f
			uint32_t const v = static_cast<uint8_t>(dim);
			*(uint32_t*)(b.data() + 8) = mcfile::detail::Int32LEFromNative(v);
			b[12] = tag;
			b[13] = y;
		}
		return leveldb::Slice(b.data(), b.size());
	}

private:
	keys() = delete;
};

}
