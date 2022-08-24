#ifndef _UFILESDK_CPP_UCLOUD_ERRNO_ERRNO_
#define _UFILESDK_CPP_UCLOUD_ERRNO_ERRNO_

#include <string>

namespace ucloud {
namespace cppsdk {
namespace error {

enum {
  ERR_CPPSDK_BASE = -1,
  ERR_CPPSDK_MISS_AKSK = ERR_CPPSDK_BASE - 2,
  ERR_CPPSDK_FILE_READ = ERR_CPPSDK_BASE - 3,
  ERR_CPPSDK_SEEKABLE = ERR_CPPSDK_BASE - 4,
  ERR_CPPSDK_CONNECT_HOST = ERR_CPPSDK_BASE - 5,
  ERR_CPPSDK_INVALID_HOST = ERR_CPPSDK_BASE - 6,
  ERR_CPPSDK_RESOLVE_DOMAIN = ERR_CPPSDK_BASE - 7,
  ERR_CPPSDK_SOCKET = ERR_CPPSDK_BASE - 8,
  ERR_CPPSDK_CONNECT = ERR_CPPSDK_BASE - 9,
  ERR_CPPSDK_SEND_HTTP = ERR_CPPSDK_BASE - 10,
  ERR_CPPSDK_SET_CURLOPT = ERR_CPPSDK_BASE - 11,
  ERR_CPPSDK_CURL_PERFORM = ERR_CPPSDK_BASE - 12,
  ERR_CPPSDK_CURL = ERR_CPPSDK_BASE - 13,
  ERR_CPPSDK_CLIENT_INTERNAL = ERR_CPPSDK_BASE - 14,
  ERR_CPPSDK_PARSE_JSON = ERR_CPPSDK_BASE - 15,
  ERR_CPPSDK_INVALID_PARAM = ERR_CPPSDK_BASE - 16,
  ERR_CPPSDK_INVALID_ETAG = ERR_CPPSDK_BASE - 17,
  ERR_CPPSDK_NO_CONFIG = ERR_CPPSDK_BASE - 18,
  ERR_CPPSDK_INVALID_CONFIG = ERR_CPPSDK_BASE - 19,
  ERR_CPPSDK_FILE_ALREADY_EXIST = ERR_CPPSDK_BASE - 20,
};

class UFileError {

private:
  UFileError() {}

public:
  static UFileError &Instance() {
    static UFileError instance;
    return instance;
  }
  ~UFileError() {}

  inline void SetError(const int retcode, const std::string &errmsg) {
    m_last_retcode = retcode;
    m_last_errmsg = errmsg;
  }
  inline int LastRetcode() { return m_last_retcode; }
  inline std::string LastErrmsg() { return m_last_errmsg; }
  std::string ErrorDesc(const int retcode);

private:
  int m_last_retcode;
  std::string m_last_errmsg;
};

#define UFILE_ERROR_DESC(retcode)                                              \
  ucloud::cppsdk::error::UFileError::Instance().ErrorDesc(retcode)

#define UFILE_SET_ERROR(retcode)                                               \
  {                                                                            \
    std::string errdesc = UFILE_ERROR_DESC(retcode);                           \
    ucloud::cppsdk::error::UFileError::Instance().SetError(retcode, errdesc);  \
  }

#define UFILE_SET_ERROR2(retcode, errmsg)                                      \
  { ucloud::cppsdk::error::UFileError::Instance().SetError(retcode, errmsg); }

#define UFILE_LAST_RETCODE()                                                   \
  ucloud::cppsdk::error::UFileError::Instance().LastRetcode()

#define UFILE_LAST_ERRMSG()                                                    \
  ucloud::cppsdk::error::UFileError::Instance().LastErrmsg()

} // namespace error
} // namespace cppsdk
} // namespace ucloud

#endif
