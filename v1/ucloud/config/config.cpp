#include "config/config.h"
#include "errno/errno.h"
#include "utils/json_util.h"
#include <string.h>
#include <fstream>

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
    if (inited) return 0;

    std::string conf("");
    if (access("./ufilesdk.conf", R_OK) == 0) {
        conf = "./ufilesdk.conf";
    } else if ( access("/etc/ufilesdk.conf", R_OK) == 0) {
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
    std::streamsize sz = ifs.readsome(buf, sizeof(buf)-1);
    if (sz < 0) {
        UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
        return ERR_CPPSDK_CLIENT_INTERNAL;
    } else if (sz == 0) {
        UFILE_SET_ERROR(ERR_CPPSDK_INVALID_CONFIG);
        return ERR_CPPSDK_INVALID_CONFIG;
    }

    std::string dataconf(buf, sz);
    int ret = JsonGetString(dataconf, "public_key", UCLOUD_PUBLIC_KEY);
    if (ret) {
        ifs.close();
        UFILE_SET_ERROR(ERR_CPPSDK_INVALID_CONFIG);
        return ERR_CPPSDK_INVALID_CONFIG;
    }
    ret = JsonGetString(dataconf, "private_key", UCLOUD_PRIVATE_KEY);
    if (ret) {
        ifs.close();
        UFILE_SET_ERROR(ERR_CPPSDK_INVALID_CONFIG);
        return ERR_CPPSDK_INVALID_CONFIG;
    }
    ret = JsonGetString(dataconf, "proxy_host", UCLOUD_HOST_SUFFIX);
    if (ret) {
        ifs.close();
        UFILE_SET_ERROR(ERR_CPPSDK_INVALID_CONFIG);
        return ERR_CPPSDK_INVALID_CONFIG;
    }
    ifs.close();
    inited = true;
    return 0;
}


}
}
}
