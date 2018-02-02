#pragma once

#include "filesystem.hpp"

#if defined(__linux__)
#include <sys/types.h>
#endif

namespace glmlv {

class PlayWav;

class OsPlayWav {
    friend PlayWav;
    OsPlayWav(const fs::path& path);
    ~OsPlayWav();
    OsPlayWav(const OsPlayWav& o) = delete;
    OsPlayWav& operator=(const OsPlayWav& o) = delete;
    OsPlayWav(OsPlayWav&& o);
    OsPlayWav& operator=(OsPlayWav&& o);

#if defined(__linux__)
    bool m_HasAplayPid;
    pid_t m_AplayPid;
#elif defined(_WIN32)
#error "TODO: Support on Win32"
#else
#error "Usupported platform"
#endif
};

class PlayWav {
    OsPlayWav impl;
public:
    PlayWav(const fs::path& path): impl(path) {}
};

} // namespace glmlv
