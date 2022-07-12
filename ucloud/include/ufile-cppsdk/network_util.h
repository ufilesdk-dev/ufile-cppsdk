#ifndef _UFILESDK_CPP_UCLOUD_UTILS_NETWORK_UTIL_
#define _UFILESDK_CPP_UCLOUD_UTILS_NETWORK_UTIL_

#include <string>
#include <vector>

namespace ucloud {
namespace cppsdk {
namespace utils {

bool IsIPFormat(const char *ip);
int ResolveDomainIP(const char *domain, std::vector<std::string> &ips);
std::string HTTPPrefix(const char *url);

} // namespace utils
} // namespace cppsdk
} // namespace ucloud

#endif
