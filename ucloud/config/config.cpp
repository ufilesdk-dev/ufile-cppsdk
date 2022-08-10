#include <fstream>
#include <string.h>
#include <ufile-cppsdk/config.h>
#include <ufile-cppsdk/errno.h>
#include <ufile-cppsdk/json_util.h>
#include <unistd.h>

using namespace ucloud::cppsdk::error;
using namespace ucloud::cppsdk::utils;

namespace ucloud {
namespace cppsdk {
namespace config {

std::string UCLOUD_PUBLIC_KEY;
std::string UCLOUD_PRIVATE_KEY;
std::string UCLOUD_HOST_SUFFIX;

int InitGlobalConfig() {

  static bool inited = false;
  if (inited)
    return 0;

  if (!UCLOUD_PUBLIC_KEY.empty() && !UCLOUD_PRIVATE_KEY.empty() &&
      !UCLOUD_HOST_SUFFIX.empty()) {
    inited = true;
    return 0;
  }

  std::string conf("");
  if (access("./ufilesdk.conf", R_OK) == 0) {
    conf = "./ufilesdk.conf";
  } else if (access("/etc/ufilesdk.conf", R_OK) == 0) {
    conf = "/etc/ufilesdk.conf";
  } else {
    UFILE_SET_ERROR(ERR_CPPSDK_NO_CONFIG);
    return ERR_CPPSDK_NO_CONFIG;
  }

  std::ifstream ifs(conf.c_str(), std::ifstream::in);
  if (!ifs) {
    UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
    return ERR_CPPSDK_CLIENT_INTERNAL;
  }

  char buf[2048];
  std::streamsize sz = ifs.readsome(buf, sizeof(buf) - 1);
  if (sz < 0) {
    UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
    return ERR_CPPSDK_CLIENT_INTERNAL;
  } else if (sz == 0) {
    UFILE_SET_ERROR(ERR_CPPSDK_INVALID_CONFIG);
    return ERR_CPPSDK_INVALID_CONFIG;
  }

  std::string dataconf(buf, sz);
  if (UCLOUD_PUBLIC_KEY.empty()) {
    int ret = JsonGetString(dataconf, "public_key", UCLOUD_PUBLIC_KEY);
    if (ret) {
      ifs.close();
      UFILE_SET_ERROR(ERR_CPPSDK_INVALID_CONFIG);
      return ERR_CPPSDK_INVALID_CONFIG;
    }
  }

  if (UCLOUD_PRIVATE_KEY.empty()) {
    int ret = JsonGetString(dataconf, "private_key", UCLOUD_PRIVATE_KEY);
    if (ret) {
      ifs.close();
      UFILE_SET_ERROR(ERR_CPPSDK_INVALID_CONFIG);
      return ERR_CPPSDK_INVALID_CONFIG;
    }
  }

  if (UCLOUD_HOST_SUFFIX.empty()) {
    int ret = JsonGetString(dataconf, "proxy_host", UCLOUD_HOST_SUFFIX);
    if (ret) {
      ifs.close();
      UFILE_SET_ERROR(ERR_CPPSDK_INVALID_CONFIG);
      return ERR_CPPSDK_INVALID_CONFIG;
    }
  }

  ifs.close();
  inited = true;
  return 0;
}

void UFileSetPublicKey(std::string value) { UCLOUD_PUBLIC_KEY = value; }

void UFileSetPrivateKey(std::string value) { UCLOUD_PRIVATE_KEY = value; }

void UFileSetProxyHost(std::string value) { UCLOUD_HOST_SUFFIX = value; }

} // namespace config
} // namespace cppsdk
} // namespace ucloud
