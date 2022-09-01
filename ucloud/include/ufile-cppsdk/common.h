#ifndef _H_UFILESDK_CPP_UCLOUD_API_COMMON_
#define _H_UFILESDK_CPP_UCLOUD_API_COMMON_

#include <istream>
#include <string>
#include <ufile-cppsdk/config.h>

#define SCHEME ("http://")

using namespace ucloud::cppsdk::config;

namespace ucloud {
namespace cppsdk {
namespace api {

/*
 * @brief: 构造bucket对应的域名
 * @bucket: bucket名称
 * @return: bucket对应的域名
 */
inline std::string UFileHost(const std::string &bucket) {

  std::string host = SCHEME + bucket + UCLOUD_HOST_SUFFIX;
  if (host[host.size() - 1] != '/')
    host += "/";
  return host;
}

/*
 * @brief: 解析错误回复中的信息
 * @body: 系统回包字串
 * @retcode: 错误码
 * @errmsg: 错误消息
 * @return: 0=成功，非0=失败
 */
int UFileErrorRsp(const char *body, int64_t *retcode, std::string &errmsg);
/*
 * @brief: 获取Content-Length
 * @is: 文件输入流
 * @fsize: content-length
 * @return: 0=成功，非0=失败
 */
int FetchContentLength(std::istream &is, std::streampos *fsize);
/*
 * @brief: 获取文件数据
 * @is: 文件输入流
 * @n: 需要获取的字节数
 * @data: 数据输出
 * @return: 0=成功，非0=失败
 */
int PeekData(std::istream &is, const int n, std::string &data);
/*
 * @brief: 获取文件MimeType
 * @filename: 文件本地路径
 * @mimetype: MimeType
 * @return: 0=成功，非0=失败
 */
int MimeType(const std::string &filename, std::string &mimetype);

} // namespace api
} // namespace cppsdk
} // namespace ucloud

#endif
