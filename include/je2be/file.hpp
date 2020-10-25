#pragma once

namespace j2b::file {

inline FILE *Open(std::filesystem::path const& p, char const* mode) {
#if defined(_MSC_VER)
    wchar_t wmode[48] = {0};
    mbstowcs(wmode, mode, 48);
    return _wfopen(p.c_str(), wmode);
#else
    return fopen(p.c_str(), mode);
#endif
}

inline void RemoveAll(std::filesystem::path const& p) {
    using namespace std;
#if defined(_MSC_VER)
    auto str = p.native();
    vector<wchar_t> buf(str.size() + 2, 0); // needs double NULL characters for pFrom
    copy_n(str.begin(), str.size(), buf.begin());
    SHFILEOPSTRUCTW op = {0};
    op.pFrom = buf.data();
    op.wFunc = FO_DELETE;
    op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    SHFileOperationW(&op);
#else
    filesystem::remove_all(p);
#endif
}

inline std::optional<std::filesystem::path> CreateTempDir(std::filesystem::path const& tempDir) {
    namespace fs = std::filesystem;
    auto tmp = fs::temp_directory_path();
#if defined(_MSC_VER)
    wchar_t* dir = _wtempnam(tmp.native().c_str(), L"j2b-tmp-");
    if (dir) {
        fs::path ret(dir);
        fs::create_directory(ret);
        free(dir);
        return ret;
    } else {
        return std::nullopt;
    }
#else
    string tmpl("j2b-tmp-XXXXXX");
    vector<char> buffer;
    copy(tmpl.begin(), tmpl.end(), back_inserter(buffer));
    buffer.push_back(0);
    char* dir = mkdtemp(buffer.data());
    if (dir) {
        return std::string(dir, strlen(dir));
    } else {
        return std::nullopt;
    }
#endif

}

}
