#ifndef _UFILESDK_CPP_UCLOUD_HMAC_HMAC_
#define _UFILESDK_CPP_UCLOUD_HMAC_HMAC_

namespace ucloud {
namespace cppsdk {
namespace hmac {

void HMAC_SHA1(unsigned char hmac[20], const unsigned char *key, int key_len,
               const unsigned char *message, int message_len);

}
} // namespace cppsdk
} // namespace ucloud

#endif
