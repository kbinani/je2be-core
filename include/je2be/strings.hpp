#pragma once

namespace j2b::strings {

inline bool Equals(std::string const& a, std::string const& b) {
    if (a.size() != b.size()) {
        return false;
    }
    return memcmp(a.c_str(), b.c_str(), a.size()) == 0;
}

inline std::string LTrim(std::string const& s, std::string const& left) {
    std::string ret = s;
    while (ret.starts_with(left)) {
        ret = ret.substr(left.size());
    }
    return ret;
}

inline std::string RTrim(std::string const& s, std::string const& right) {
    std::string ret = s;
    while (ret.ends_with(right)) {
        ret = ret.substr(0, ret.size() - right.size());
    }
    return ret;
}

}
