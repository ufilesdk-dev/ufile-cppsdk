#ifndef _H_UFILESDK_CPP_UCLOUD_API_LIST_
#define _H_UFILESDK_CPP_UCLOUD_API_LIST_

#include <list>
#include <ostream>
#include <string>
#include <ufile-cppsdk/apibase.h>

namespace ucloud {
namespace cppsdk {
namespace api {

struct ListResultEntry {
  std::string filename;      // 文件名称
  std::string mime_type;     // 文件mimetype
  std::string etag;          // 标识文件内容
  uint64_t size;             // 文件大小
  std::string storage_class; // 文件存储类型
  uint64_t last_modified;    //文件最后修改时间
  uint64_t create_time;      // 文件创建时间
};

typedef std::list<ListResultEntry> ListResult;

class UFileList : public APIBase {

public:
  UFileList();
  ~UFileList();

  /*
   * @brief: 列出指定前缀的文件
   * @bucket: 目标bucket名称
   * @prefix: 前缀, 若传空, 表示列整个bucket的文件
   * @count: 要列的目录文件列表的最大数量, 最大为1000
   * @result: 要保存返回结果的list对象的地址
   * @is_truncated: 指示此次返回结果是否被截断, 若值
   * 为true, 则表示返回结果由于达到count的限制而被截断
   * @next_marker: 当is_truncated为true时，next_marker被设置,
   * 下次List可以使用此值作为prefix, 实现迭代查询
   * marker: 用于迭代查询, 如要上次查询如果被截断, 这次查询可以将
   * 此参数设置为上次查询结果返回的next_marker, 继续查询。默认为空,
   * 表示从头查询
   * @return: 0=成功，非0=失败
   */
  int List(const std::string &bucket, const std::string &prefix, uint32_t count,
           ListResult *result, bool *is_truncated, std::string *next_marker,
           const std::string &marker = "");

  /*
   * @brief: 列出指定前缀的文件
   * @bucket: 目标bucket名称
   * @prefix: 前缀, 若传空, 表示列整个bucket的文件
   * @count: 要列的目录文件列表的最大数量, 最大为1000
   * @result: 要保存返回结果的list对象的地址
   * @is_truncated: 指示此次返回结果是否被截断, 若值
   * 为true, 则表示返回结果由于达到count的限制而被截断
   * @next_marker: 当is_truncated为true时，next_marker被设置,
   * 下次List可以使用此值作为prefix, 实现迭代查询
   * marker: 用于迭代查询, 如要上次查询如果被截断, 这次查询可以将
   * 此参数设置为上次查询结果返回的next_marker, 继续查询。默认为空,
   * 表示从头查询
   * @return: 0=成功，非0=失败
   */
  int ListDir(const std::string &bucket, const std::string &prefix,
              uint32_t count, std::list<std::string> *result,
              bool *is_truncated, std::string *next_marker,
              const std::string &marker = "");

private:
  void SetResource(const std::string &bucket, const std::string &prefix,
                   uint32_t count, const std::string &marker,
                   std::string delimiter = "");

  void SetURL(std::map<std::string, std::string> params);

  int ParseRsp(const char *body, ListResult *result, bool *is_truncated,
               std::string *next_marker,
               std::list<std::string> *prefixes = nullptr);

  std::string bucket_;
  std::string prefix_;
  std::string marker_;
  std::string delimiter_;
  uint32_t max_key_;
};

} // namespace api
} // namespace cppsdk
} // namespace ucloud

#endif
