#ifndef _UFILESDK_CPP_UCLOUD_UTILS_NETWORK_UTIL_
#define _UFILESDK_CPP_UCLOUD_UTILS_NETWORK_UTIL_

#include <vector>
#include <string>

namespace ucloud {
namespace cppsdk {
namespace utils  {

bool IsIPFormat(const char *ip);
int  ResolveDomainIP(const char *domain, std::vector<std::string> &ips);
std::string HTTPPrefix(const char *url);

}
}
}

#endif
