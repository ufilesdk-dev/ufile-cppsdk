#ifndef _H_UFILESDK_CPP_UCLOUD_API_HEADFILE_
#define _H_UFILESDK_CPP_UCLOUD_API_HEADFILE_

#include <list>
#include <ostream>
#include <string>
#include <ufile-cppsdk/apibase.h>

namespace ucloud {
namespace cppsdk {
namespace api {

struct FileInfo {
  std::string type;          // 文件类型
  uint64_t size;             // 文件大小
  std::string etag;          // 文件的etag
  std::string last_modified; // 上次修改时间
  std::string create_time;   // 创建时间
  std::string storage_class; // 存储类型
};

class UFileHeadfile : public APIBase {
public:
  /*
   * @brief: 获取文件信息
   * @bucket: 目标bucket名称
   * @key: 文件对象名
   * @result: 要保存返回结果的FileInfo对象的地址
   * @return: 0=成功，非0=失败
   */
  int GetFileInfo(const std::string &bucket, const std::string &key,
                  FileInfo *result);

private:
  void SetResource(const std::string &bucket, const std::string &key);

  int ParseRsp(const char *header, FileInfo *result);

  std::string bucket_;
  std::string key_;
};

} // namespace api
} // namespace cppsdk
} // namespace ucloud

#endif