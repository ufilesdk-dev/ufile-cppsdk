#ifndef _UFILESDK_CPP_UCLOUD_API_TAG_
#define _UFILESDK_CPP_UCLOUD_API_TAG_

#include <ufile-cppsdk/apibase.h>

namespace ucloud {
namespace cppsdk {
namespace api {

class UFileTagging : public APIBase {

public:
  UFileTagging();
  ~UFileTagging();

  /*
   * @brief: 给object添加tag
   * @bucket: 目标Bucket名称
   * @key: 保存在Bucket上的文件对象名称
   * @tags: tag集合
   */
  int PutTagging(const std::string &bucket, const std::string &key,
                 std::map<std::string, std::string> &tags);

  /*
   * @brief: 获取object的tag
   * @bucket: 目标Bucket名称
   * @key: 保存在Bucket上的文件对象名称
   * @tags: tag集合
   */
  int GetTagging(const std::string &bucket, const std::string &key,
                 std::map<std::string, std::string> *tags);

  /*
   * @brief: 删除object的tag
   * @bucket: 目标Bucket名称
   * @key: 保存在Bucket上的文件对象名称
   */
  int DeleteTagging(const std::string &bucket, const std::string &key);

private:
  int TaggingToJsonString(std::map<std::string, std::string> &tags,
                          std::string *json_str);

  int ParseTaggingFromJsonString(const std::string &json_str,
                                 std::map<std::string, std::string> *tags);
};

} // namespace api
} // namespace cppsdk
} // namespace ucloud

#endif