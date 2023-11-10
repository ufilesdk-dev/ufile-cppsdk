#ifndef _UFILESDK_CPP_UCLOUD_API_LISTPARTS_
#define _UFILESDK_CPP_UCLOUD_API_LISTPARTS_

#include <list>
#include <string>
#include <ufile-cppsdk/apibase.h>

namespace ucloud {
namespace cppsdk {
namespace api {

struct ListPartsResultEntry {
  int32_t part_number;      // 分片编号
  std::string etag;   // 分片etag值
  uint64_t last_modified;   // Part上传的时间
  uint64_t size; // 已上传Part大小
};

typedef std::list<ListPartsResultEntry> ListPartsResult;

class UFileListParts : public APIBase {
public:
  UFileListParts();
  ~UFileListParts();

  /*
   * @brief: 获取已上传成功的分片列表
   * @bucket: 目标bucket名称
   * @uploadid: 上传id
   * @count: 一次要列的part最大数量, 最大为1000
   * @result: 要保存返回结果的ListPartsResult对象的地址
   * @is_truncated: 指示此次返回结果是否被截断, 若值
   * 为true, 则表示返回结果由于达到count的限制而被截断
   * @next_marker: 当is_truncated为true时，next_marker被设置,
   * 下次ListParts可以使用此值作为书签, 实现迭代查询
   * marker: 用于迭代查询, 如要上次查询如果被截断, 这次查询可以将
   * 此参数设置为上次查询结果返回的next_marker, 继续查询。默认为空,
   * 表示从头查询
   * @return: 0=成功，非0=失败
   */
  int ListParts(const std::string &bucket, const std::string &uploadid,
                uint32_t count, ListPartsResult *result, bool *is_truncated,
                int32_t *next_marker, int32_t marker = 0);

private:
  void SetURL(const std::string &bucket, std::map<std::string, std::string> params);

  int ParseRsp(const char *body, ListPartsResult *result, bool *is_truncated,
               int32_t *next_marker);
};

} // namespace api
} // namespace cppsdk
} // namespace ucloud

#endif // _UFILESDK_CPP_UCLOUD_API_LISTPARTS_
