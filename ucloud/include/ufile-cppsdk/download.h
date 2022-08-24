#ifndef _H_UFILESDK_CPP_UCLOUD_API_DOWNLOAD_
#define _H_UFILESDK_CPP_UCLOUD_API_DOWNLOAD_

#include <ostream>
#include <string>
#include <ufile-cppsdk/apibase.h>

namespace ucloud {
namespace cppsdk {
namespace api {

class UFileDownload : public APIBase {

public:
  UFileDownload();
  ~UFileDownload();

  /*
   * @brief: 下载文件到一个输出流
   * @bucket: 目标bucket名称
   * @key: 要下载的文件对象名
   * @os: 文件输出流
   * @range: 分片下载的区间，区间为前闭后闭, [begin, end]
   * @return: 0=成功，非0=失败
   */
  int Download(const std::string &bucket, const std::string &key,
               std::ostream *os,
               const std::pair<ssize_t, ssize_t> *range = NULL);
  /*
   * @brief: 下载文件到本地指定路径
   * @bucket: 目标bucket名称
   * @key: 要下载的文件对象名
   * @filepath: 文件本地路径
   * @range: 分片下载的区间，区间为前闭后闭, [begin, end]
   * @return: 0=成功，非0=失败
   */
  int DownloadAsFile(const std::string &bucket, const std::string &key,
                     const std::string &filepath,
                     const std::pair<ssize_t, ssize_t> *range = NULL);

  /*
   * @brief: 生成带签名的下载url
   * @bucket: 目标bucket名称
   * @key: 要下载的文件对象名
   * @expires: url的过期时间，0表示不过期
   * @return: 下载url
   */
  std::string DownloadURL(const std::string &bucket, const std::string &key,
                          const size_t expires = 0);

  std::string GetTempFilePath(const std::string &filepath);

private:
  void SetResource(const std::string &bucket, const std::string &key);

private:
  std::string m_bucket;
  std::string m_key;
  std::ostream *m_os;
};

} // namespace api
} // namespace cppsdk
} // namespace ucloud

#endif
