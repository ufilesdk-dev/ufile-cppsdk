#ifndef _UFILESDK_CPP_UCLOUD_UTILS_STRING_UTIL_
#define _UFILESDK_CPP_UCLOUD_UTILS_STRING_UTIL_

#include <stdio.h>
#include <stdint.h>
#include <string>

namespace ucloud {
namespace cppsdk {
namespace utils {

inline const char *UINT2STR(const uint64_t &u) {

    static char buf[1024];
    snprintf(buf, sizeof(buf)-1, "%lu", u);
    return buf;
}

inline const char *SIZET2STR(const size_t &sz) {

    static char buf[1024];
    snprintf(buf, sizeof(buf)-1, "%zd", sz);
    return buf;
}

inline bool STR2UINT(const char *s, uint64_t *u) {

    if (!s || !u) return false;
    return sscanf(s, "%lu", u) == 1;
}

inline bool STR2SIZET(const char *s, size_t *st) {

    if (!s || !st) return false;
    return sscanf(s, "%zd", st) == 1;
}

inline void TrimLeft(std::string &strSource, const char *pChars) {
	std::string::size_type iPos = strSource.find_first_not_of(pChars);
	if ( iPos == std::string::npos ) {
		strSource = "";
		return;
	}

	strSource = strSource.substr(iPos);
}

inline void TrimRight(std::string &strSource, const char *pChars) {
	std::string::size_type iPos = strSource.find_last_not_of(pChars);
	if ( iPos == std::string::npos ) {
		strSource = "";
		return;
	}

	strSource = strSource.substr(0, iPos + 1);
}

inline void Trim(std::string &strSource, const char *pChars) {
	TrimLeft(strSource, pChars);
	TrimRight(strSource, pChars);
}

}
}
}



#endif
