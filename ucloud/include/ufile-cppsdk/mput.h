#ifndef _H_UFILESDK_CPP_UCLOUD_API_MPUT_
#define _H_UFILESDK_CPP_UCLOUD_API_MPUT_

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <ufile-cppsdk/apibase.h>

namespace ucloud {
namespace cppsdk {
namespace api {

#define UPLOADID_KEY ("UploadId")
#define BLKSIZE_KEY ("BlkSize")
#define PART_NUMBER_KEY ("PartNumber")
#define FILESIZE_KEY ("FileSize")

class UFileMput : public APIBase {

public:
  UFileMput();
  ~UFileMput();

  /*
   * @brief: 设置分片上传的基本信息
   * @bucket: 目标Bucket名称
   * @key: 保存在Bucket上的文件对象名称
   */
  void SetResource(const std::string &bucket, const std::string &key);
  /*
   * @brief: 设置本次分片上传的上传Id
   * @uploadid: 分片上传初始化返回的上传Id
   */
  void SetUploadId(const std::string &uploadid);
  /*
   * @brief: 设置上传文件路径，并打开文件
   * @filepath: 文件的本地路径
   * @return: 0=成功，非0=失败
   */
  int SetUploadFile(const std::string &filepath);
  /*
   * @brief: 设置上传文件的输入流
   * @is: 文件输入流
   * @return: 0=成功，非0=失败
   */
  int SetUploadStream(std::istream *is);

  /*
   * @brief: 分片上传初始化
   * @return: 0=成功，非0=失败
   */
  int MInit();
  /*
   * @brief: 上传文件分片
   * @blk_idx: 分片编号
   * @return: 0=成功，非0=失败
   */
  int MUpload(ssize_t blk_idx = -1);

  /*
   * @brief: copy文件分片
   * @blk_idx: 分片编号
   * @src_bucket_name: 源Bucket名称
   * @src_object: 源文件对象名称
   * @offset: 源文件偏移
   * @length: 要copy的长度
   * @mimetype: 目标文件的MIME类型
   * @return: 0=成功，非0=失败
   */
  int MUploadCopyPart(ssize_t blk_idx, std::string src_bucket_name,
                      std::string src_object, size_t offset, size_t length,
                      std::string mimetype = "application/octet-stream");

  /*
   * @brief: 分片上传完成
   * @etags: 分片编号与ETag的映射表
   * @return: 0=成功，非0=失败
   */
  int MFinish(const std::map<size_t, std::string> *etags);
  /*
   * @brief: 上传完成后得到的文件ETag
   */
  inline const std::string &ETag() { return m_final_etag; }
  /*
   * @brief: 上传完成后得到的文件大小
   */
  inline size_t FileSize() { return m_finished_fsize; }
  /*
   * @brief: 已上传的文件大小
   */
  inline size_t UploadedSize() { return m_uploaded_size; }
  /*
   * @brief: 分片上传的块大小
   */
  inline size_t BlkSize() { return m_blksize; }

private:
  int ParseInitResult(const std::string &data);
  int ParseMuploadResult(const std::string &body, const std::string &header);
  int ParseMFinishResult(const std::string &body, const std::string &header);
  int JudgeBlksize(const size_t blk_idx, size_t &bsize);
  std::string MUploadURL();
  std::string MFinishURL();
  int ETagStream(const std::map<size_t, std::string> &etag_list,
                 std::stringstream &sss);

private:
  size_t m_blksize;
  size_t m_blk_idx;
  size_t m_finished_fsize;
  size_t m_uploaded_size;
  std::string m_uploadid;
  std::string m_bucket;
  std::string m_key;
  std::string m_final_etag;
  std::string m_mimetype;
  std::map<size_t, std::string> m_etag_list;

  std::istream *m_is;
  std::ifstream *m_file_stream;
  std::string m_filename;
};

} // namespace api
} // namespace cppsdk
} // namespace ucloud
#endif
