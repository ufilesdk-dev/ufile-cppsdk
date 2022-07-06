#ifndef _UFILESDK_CPP_UCLOUD_BASE63_BASE64_
#define _UFILESDK_CPP_UCLOUD_BASE63_BASE64_

#include <string>

namespace ucloud {
namespace cppsdk {
namespace base64 {

enum {
    BASE64_URL = 0,
    BASE64_STD,
};

std::string base64_encode(unsigned char const* , unsigned int len, int type);
std::string base64_decode(std::string const& s, int type);

}
}
}

#endif
