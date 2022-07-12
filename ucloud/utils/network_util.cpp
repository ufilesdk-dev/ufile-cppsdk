#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ufile-cppsdk/network_util.h>

namespace ucloud {
namespace cppsdk {
namespace utils {

bool IsIPFormat(const char *ip) {

  // only for IPV4
  if (!ip || strlen(ip) > 15)
    return false;

  ssize_t a = -1, b = -1, c = -1, d = -1;
  int n = sscanf(ip, "%zd.%zd.%zd.%zd", &a, &b, &c, &d);
  if (n != 4 || a == -1 || b == -1 || c == -1 || d == -1)
    return false;
  return a >= 0 && a <= 255 && b >= 0 && b <= 255 && c >= 0 && c <= 255 &&
         d >= 0 && d <= 255;
}

int ResolveDomainIP(const char *domain, std::vector<std::string> &ips) {

  if (!domain)
    return -1;

  struct addrinfo hint;
  struct addrinfo *res;
  hint.ai_family = AF_INET;
  hint.ai_socktype = SOCK_STREAM;
  hint.ai_protocol = IPPROTO_TCP;
  hint.ai_flags = AI_PASSIVE;
  int ret = getaddrinfo(domain, NULL, &hint, &res);
  if (ret) {
    return -1;
  }

  struct addrinfo *ai = res;
  while (ai) {
    char ip[16];
    struct sockaddr_in *addr = (struct sockaddr_in *)ai->ai_addr;
    unsigned nip = addr->sin_addr.s_addr;
    inet_ntop(AF_INET, &nip, ip, 16);
    ips.push_back(ip);
    ai = ai->ai_next;
  }
  freeaddrinfo(res);
  return 0;
}

std::string HTTPPrefix(const char *url) {

  const char *pos = strstr(url, "http://");
  if (pos) {
    return std::string(url);
  }
  return "http://" + std::string(url);
}

} // namespace utils
} // namespace cppsdk
} // namespace ucloud
