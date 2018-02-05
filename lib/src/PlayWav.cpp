#include <glmlv/PlayWav.hpp>
#include <iostream>
#include <string.h>

#if defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

namespace glmlv {

#if defined(__linux__)

OsPlayWav::OsPlayWav(const fs::path& path): m_HasAplayPid(true), m_AplayPid(fork()) {
    switch(m_AplayPid) {
    case 0:
        if(-1 == execlp("aplay", "aplay", path.string().c_str(), nullptr))
            std::cerr << "Error launching aplay: " << strerror(errno) << std::endl;
        break;
    case -1:
        std::cerr << "Error launching aplay: " << strerror(errno) << std::endl;
        m_HasAplayPid = false;
        break;
    }
}
OsPlayWav::~OsPlayWav() {
    if(m_HasAplayPid)
        kill(m_AplayPid, SIGKILL);
}
OsPlayWav::OsPlayWav(OsPlayWav&& o): 
    m_HasAplayPid(o.m_HasAplayPid),
    m_AplayPid(o.m_AplayPid)
{ 
    o.m_HasAplayPid = false;
}
OsPlayWav& OsPlayWav::operator=(OsPlayWav&& o) { 
    std::swap(m_AplayPid, o.m_AplayPid);
    std::swap(m_HasAplayPid, o.m_HasAplayPid);
    return *this;
}

#elif defined(_WIN32)

OsPlayWav::OsPlayWav(const fs::path& path) {
	const auto flags = SND_FILENAME | SND_ASYNC | SND_NODEFAULT | SND_NOSTOP;
	const BOOL success = PlaySoundW(path.wstring().c_str(), nullptr, flags);
    if(!success) {
		auto err = GetLastError();
		wchar_t buf[256];
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			buf, sizeof(buf), NULL);
        std::cerr << "Could not play \"" << path << "\": PlaySound error " << err << ": " << buf << std::endl;
    }
}
OsPlayWav::~OsPlayWav() {}
OsPlayWav::OsPlayWav(OsPlayWav&& o) {}
OsPlayWav& OsPlayWav::operator=(OsPlayWav&& o) { return *this; }

#endif

} // namespace glmlv