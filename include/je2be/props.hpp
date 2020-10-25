#pragma once

namespace j2b::props {

inline std::shared_ptr<mcfile::nbt::ByteTag> Bool(bool b) {
    return std::make_shared<mcfile::nbt::ByteTag>(b ? 1 : 0);
}

inline std::shared_ptr<mcfile::nbt::ByteTag> Byte(int8_t v) {
    uint8_t t = *(uint8_t*)&v;
    return std::make_shared<mcfile::nbt::ByteTag>(t);
}

inline std::shared_ptr<mcfile::nbt::IntTag> Int(int32_t v) {
    return std::make_shared<mcfile::nbt::IntTag>(v);
}

inline std::shared_ptr<mcfile::nbt::LongTag> Long(int64_t v) {
    return std::make_shared<mcfile::nbt::LongTag>(v);
}

inline std::shared_ptr<mcfile::nbt::StringTag> String(std::string v) {
    return std::make_shared<mcfile::nbt::StringTag>(v);
}

inline std::shared_ptr<mcfile::nbt::FloatTag> Float(float v) {
    return std::make_shared<mcfile::nbt::FloatTag>(v);
}

inline std::shared_ptr<mcfile::nbt::ShortTag> Short(int16_t v) {
    return std::make_shared<mcfile::nbt::ShortTag>(v);
}

inline std::optional<int32_t> GetInt(mcfile::nbt::CompoundTag const& tag, std::string const& name) {
    auto found = tag.fValue.find(name);
    if (found == tag.fValue.end()) {
        return std::nullopt;
    }
    auto itag = found->second->asInt();
    if (!itag) {
        return std::nullopt;
    }
    return itag->asInt()->fValue;
}

inline std::optional<bool> GetBool(mcfile::nbt::CompoundTag const& tag, std::string const& name) {
    auto found = tag.fValue.find(name);
    if (found == tag.fValue.end()) {
        return std::nullopt;
    }
    auto itag = found->second->asByte();
    if (!itag) {
        return std::nullopt;
    }
    return itag->asByte()->fValue != 0;
}

inline std::optional<int8_t> GetByte(mcfile::nbt::CompoundTag const& tag, std::string const& name) {
    auto found = tag.fValue.find(name);
    if (found == tag.fValue.end()) {
        return std::nullopt;
    }
    auto itag = found->second->asByte();
    if (!itag) {
        return std::nullopt;
    }
    return itag->asByte()->fValue;
}

inline std::optional<std::string> GetString(mcfile::nbt::CompoundTag const& tag, std::string const& name) {
    auto found = tag.fValue.find(name);
    if (found == tag.fValue.end()) {
        return std::nullopt;
    }
    auto itag = found->second->asString();
    if (!itag) {
        return std::nullopt;
    }
    return itag->asString()->fValue;
}

inline std::optional<int64_t> GetLong(mcfile::nbt::CompoundTag const& tag, std::string const& name) {
    auto found = tag.fValue.find(name);
    if (found == tag.fValue.end()) {
        return std::nullopt;
    }
    auto itag = found->second->asLong();
    if (!itag) {
        return std::nullopt;
    }
    return itag->asLong()->fValue;
}

inline std::optional<int16_t> GetShort(mcfile::nbt::CompoundTag const& tag, std::string const& name) {
    auto found = tag.fValue.find(name);
    if (found == tag.fValue.end()) {
        return std::nullopt;
    }
    auto itag = found->second->asShort();
    if (!itag) {
        return std::nullopt;
    }
    return itag->asShort()->fValue;
}

inline std::optional<float> GetFloat(mcfile::nbt::CompoundTag const& tag, std::string const& name) {
    auto found = tag.fValue.find(name);
    if (found == tag.fValue.end()) {
        return std::nullopt;
    }
    auto itag = found->second->asFloat();
    if (!itag) {
        return std::nullopt;
    }
    return itag->asFloat()->fValue;
}

}
