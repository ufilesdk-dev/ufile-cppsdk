#include <map>
#include <ufile-cppsdk/errno.h>

namespace ucloud {
namespace cppsdk {
namespace error {

static std::map<int, std::string> errorDesc = {
    {ERR_CPPSDK_MISS_AKSK, "miss publickey/privatekey config"},
    {ERR_CPPSDK_FILE_READ, "file read error"},
    {ERR_CPPSDK_SEEKABLE, "stream is not seekable"},
    {ERR_CPPSDK_CONNECT_HOST, "connect to host failed"},
    {ERR_CPPSDK_INVALID_HOST, "invalid host"},
    {ERR_CPPSDK_RESOLVE_DOMAIN, "resolve domain failed"},
    {ERR_CPPSDK_SOCKET, "socket error"},
    {ERR_CPPSDK_CONNECT, "socket connect failed"},
    {ERR_CPPSDK_SEND_HTTP, "send http failed"},
    {ERR_CPPSDK_SET_CURLOPT, "set curl opt failed"},
    {ERR_CPPSDK_CURL_PERFORM, "curl preform failed"},
    {ERR_CPPSDK_CURL, "curl error"},
    {ERR_CPPSDK_CLIENT_INTERNAL, "sdk client internal error"},
    {ERR_CPPSDK_PARSE_JSON, "parse json error"},
    {ERR_CPPSDK_INVALID_PARAM, "invalid input param"},
    {ERR_CPPSDK_INVALID_ETAG, "invalid etag"},
    {ERR_CPPSDK_NO_CONFIG, "no config found"},
    {ERR_CPPSDK_INVALID_CONFIG, "invalid config"},
    {ERR_CPPSDK_FILE_ALREADY_EXIST, "file already exist"},
};

std::string UFileError::ErrorDesc(const int retcode) {

  std::map<int, std::string>::iterator it = errorDesc.find(retcode);
  if (it == errorDesc.end())
    return "";
  return it->second;
}

} // namespace error
} // namespace cppsdk
} // namespace ucloud
