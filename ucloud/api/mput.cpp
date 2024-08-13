#include <fcntl.h>
#include <json-c/json.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ufile-cppsdk/common.h>
#include <ufile-cppsdk/digest.h>
#include <ufile-cppsdk/errno.h>
#include <ufile-cppsdk/http.h>
#include <ufile-cppsdk/json_util.h>
#include <ufile-cppsdk/mput.h>
#include <ufile-cppsdk/stream_util.h>
#include <ufile-cppsdk/string_util.h>
#include <ufile-cppsdk/urlcodec.h>
#include <unistd.h>

using namespace ucloud::cppsdk::error;
using namespace ucloud::cppsdk::digest;
using namespace ucloud::cppsdk::http;
using namespace ucloud::cppsdk::utils;

namespace ucloud {
namespace cppsdk {
namespace api {

UFileMput::UFileMput()
    : m_blksize(0), m_blk_idx(0), m_finished_fsize(0), m_uploaded_size(0),
      m_uploadid(""), m_bucket(""), m_key(""), m_final_etag(""), m_mimetype(""),
      m_is(NULL), m_file_stream(NULL) {}

UFileMput::~UFileMput() {

  if (m_file_stream) {
    if (m_file_stream->is_open()) {
      m_file_stream->close();
    }
    delete m_file_stream;
  }
}

void UFileMput::SetResource(const std::string &bucket, const std::string &key) {

  m_bucket = bucket;
  m_key = key;
}

void UFileMput::SetUploadId(const std::string &uploadid) {

  m_uploadid = uploadid;
}

std::string UFileMput::GetUploadId() {
  return m_uploadid;
}

int UFileMput::SetUploadFile(const std::string &filepath) {

  if (access(filepath.c_str(), R_OK)) {
    UFILE_SET_ERROR2(ERR_CPPSDK_INVALID_PARAM, std::string(strerror(errno)));
    return ERR_CPPSDK_INVALID_PARAM;
  }

  //关闭之前的文件流
  if (!m_file_stream)
    m_file_stream = new std::ifstream();
  if (m_file_stream && m_file_stream->is_open())
    m_file_stream->close();
  m_file_stream->open(filepath.c_str(), std::ifstream::in);
  if (m_file_stream->fail()) {
    UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
    return ERR_CPPSDK_CLIENT_INTERNAL;
  }
  m_is = m_file_stream;
  m_filename = filepath;
  return 0;
}

int UFileMput::SetUploadStream(std::istream *is) {

  if (!is)
    return -1;
  //如果有文件流,需要先关闭
  if (m_file_stream && m_file_stream->is_open())
    m_file_stream->close();
  m_is = is;
  return 0;
}

int UFileMput::MInit() {

  int64_t ret = InitGlobalConfig();
  if (ret)
    return ret;

  std::string signature("");
  //构建 HTTP 头部
  m_http->Reset();
  m_http->SetVerb("POST");
  m_http->SetURL(UFileHost(m_bucket) + EasyPathEscape(m_key) + "?uploads");
  m_http->AddHeader("User-Agent", USERAGENT);

  //使用 HTTP 信息构建签名
  UFileDigest digestor;
  ret = digestor.SignWithRequest(m_http, HEAD_FIELD_CHECK, m_bucket, m_key, "",
                                 signature);
  if (ret) {
    return ret;
  }
  m_http->AddHeader("Authorization", digestor.Token(signature));

  //设置输出
  std::ostringstream oss;
  UCloudOStream data_stream(&oss);
  UCloudHTTPWriteParam wp = {f : NULL, os : &data_stream};
  ret = m_http->RoundTrip(NULL, &wp, NULL);
  if (ret) {
    UFILE_SET_ERROR2(ERR_CPPSDK_SEND_HTTP, UFILE_LAST_ERRMSG());
    return ERR_CPPSDK_SEND_HTTP;
  }
  //解析回应
  long code = 200;
  ret = m_http->ResponseCode(&code);
  if (ret) {
    UFILE_SET_ERROR(ERR_CPPSDK_CURL);
    return ERR_CPPSDK_CURL;
  }

  std::string errmsg;
  if (code != 200) {
    int parse_ret = UFileErrorRsp(oss.str().c_str(), &ret, errmsg);
    if (parse_ret) {
      UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
      return ERR_CPPSDK_CLIENT_INTERNAL;
    }
    UFILE_SET_ERROR2(ret, errmsg);
  } else {
    ret = ParseInitResult(oss.str());
    if (ret) {
      return ret;
    }
  }
  return ret;
}

int UFileMput::MUpload(ssize_t blk_idx) {

  int64_t ret = InitGlobalConfig();
  if (ret)
    return ret;

  if (!m_is) {
     UFILE_SET_ERROR2(ERR_CPPSDK_INVALID_PARAM,
        "stream has not been set up yet");
     return ERR_CPPSDK_INVALID_PARAM;
  }

  if (blk_idx != -1)
    m_blk_idx = blk_idx;

  m_is->clear();
  m_is->seekg(m_blksize * m_blk_idx, m_is->beg);
  if (m_is->fail()) {
    UFILE_SET_ERROR2(ERR_CPPSDK_SEEKABLE, "seek blk error");
    return ERR_CPPSDK_SEEKABLE;
  }
  size_t bsize;
  ret = JudgeBlksize(m_blk_idx, bsize);
  if (ret) {
    UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
    return ERR_CPPSDK_CLIENT_INTERNAL;
  }

  //获取文件 mimetype
  if (m_mimetype == "") {
    ret = MimeType(m_filename, m_mimetype);
    if (ret)
      return ret;
  }

  //开始上传数据
  std::string signature("");
  //构建 HTTP 头部
  m_http->Reset();
  m_http->SetVerb("PUT");
  m_http->AddHeader("Content-Type", m_mimetype);
  m_http->AddHeader("Content-Length", SIZET2STR(bsize));
  m_http->AddHeader("User-Agent", USERAGENT);
  m_http->SetURL(MUploadURL());

  //使用 HTTP 信息构建签名
  UFileDigest digestor;
  ret = digestor.SignWithRequest(m_http, HEAD_FIELD_CHECK, m_bucket, m_key, "",
                                 signature);
  if (ret) {
    return ret;
  }
  m_http->AddHeader("Authorization", digestor.Token(signature));

  //设置输出
  std::ostringstream oss, hss;
  UCloudOStream data_stream(&oss);
  UCloudOStream header_stream(&hss);
  UCloudHTTPReadParam rp =
      {f : NULL, is : *m_is, fsize : bsize, need_total_n : bsize};
  UCloudHTTPWriteParam wp = {f : NULL, os : &data_stream};
  UCloudHTTPHeaderParam hp = {f : NULL, os : &header_stream};
  ret = m_http->RoundTrip(&rp, &wp, &hp);
  if (ret) {
    UFILE_SET_ERROR2(ERR_CPPSDK_SEND_HTTP, UFILE_LAST_ERRMSG());
    return ERR_CPPSDK_SEND_HTTP;
  }

  //解析回应
  long code = 200;
  ret = m_http->ResponseCode(&code);
  if (ret) {
    UFILE_SET_ERROR(ERR_CPPSDK_CURL);
    return ERR_CPPSDK_CURL;
  }

  std::string errmsg;
  if (code != 200) {
    int parse_ret = UFileErrorRsp(oss.str().c_str(), &ret, errmsg);
    if (parse_ret) {
      UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
      return ERR_CPPSDK_CLIENT_INTERNAL;
    }
    UFILE_SET_ERROR2(ret, errmsg);
  } else {
    ret = ParseMuploadResult(oss.str(), hss.str());
    if (ret) {
      return ret;
    }
    m_uploaded_size += bsize;
  }
  return ret;
}

int UFileMput::MUpload(ssize_t blk_idx, const char *data, size_t len) {
  int64_t ret = InitGlobalConfig();
  if (ret)
    return ret;

  m_blk_idx = blk_idx;

  if (m_mimetype == "") {
    ret = MimeType(m_filename, m_mimetype);
    if (ret)
      return ret;
  }

  UCloudStreamBuf sbuf(data, len);
  std::istream iss(&sbuf);
  if (!iss) {
    UFILE_SET_ERROR(ERR_CPPSDK_FILE_READ);
    return ERR_CPPSDK_FILE_READ;
  }

  //开始上传数据
  std::string signature("");
  //构建 HTTP 头部
  m_http->Reset();
  m_http->SetVerb("PUT");
  m_http->AddHeader("Content-Type", m_mimetype);
  m_http->AddHeader("Content-Length", SIZET2STR(len));
  m_http->AddHeader("User-Agent", USERAGENT);
  m_http->SetURL(MUploadURL());

  //使用 HTTP 信息构建签名
  UFileDigest digestor;
  ret = digestor.SignWithRequest(m_http, HEAD_FIELD_CHECK, m_bucket, m_key, "",
                                 signature);
  if (ret) {
    return ret;
  }
  m_http->AddHeader("Authorization", digestor.Token(signature));

  //设置输出
  std::ostringstream oss, hss;
  UCloudOStream data_stream(&oss);
  UCloudOStream header_stream(&hss);
  UCloudHTTPReadParam rp =
      {f : NULL, is : iss, fsize : len, need_total_n : len};
  UCloudHTTPWriteParam wp = {f : NULL, os : &data_stream};
  UCloudHTTPHeaderParam hp = {f : NULL, os : &header_stream};
  ret = m_http->RoundTrip(&rp, &wp, &hp);
  if (ret) {
    UFILE_SET_ERROR2(ERR_CPPSDK_SEND_HTTP, UFILE_LAST_ERRMSG());
    return ERR_CPPSDK_SEND_HTTP;
  }

  //解析回应
  long code = 200;
  ret = m_http->ResponseCode(&code);
  if (ret) {
    UFILE_SET_ERROR(ERR_CPPSDK_CURL);
    return ERR_CPPSDK_CURL;
  }

  std::string errmsg;
  if (code != 200) {
    int parse_ret = UFileErrorRsp(oss.str().c_str(), &ret, errmsg);
    if (parse_ret) {
      UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
      return ERR_CPPSDK_CLIENT_INTERNAL;
    }
    UFILE_SET_ERROR2(ret, errmsg);
  } else {
    ret = ParseMuploadResult(oss.str(), hss.str());
    if (ret) {
      return ret;
    }
    m_uploaded_size += len;
  }
  return ret;
}

int UFileMput::MUploadCopyPart(ssize_t blk_idx, std::string src_bucket_name,
                               std::string src_object, size_t offset,
                               size_t length, std::string mimetype) {

  int64_t ret = InitGlobalConfig();
  if (ret)
    return ret;

  if (blk_idx != -1)
    m_blk_idx = blk_idx;

  //开始上传数据
  std::string signature("");
  //构建 HTTP 头部
  m_http->Reset();
  m_http->SetVerb("PUT");
  m_http->AddHeader("X-Ufile-Copy-Source",
                    std::string("/") + std::string(src_bucket_name) +
                        std::string("/") + std::string(src_object));
  m_http->AddHeader("X-Ufile-Copy-Source-Range",
                    std::string("bytes=") + std::string(SIZET2STR(offset)) +
                        std::string("-") +
                        std::string(SIZET2STR(offset + length - 1)));
  m_http->AddHeader("Content-Type", mimetype);
  m_http->AddHeader("Content-Length", SIZET2STR(0));
  m_http->AddHeader("User-Agent", USERAGENT);
  m_http->SetURL(MUploadURL());

  //使用 HTTP 信息构建签名
  UFileDigest digestor;
  ret = digestor.SignWithRequest(m_http, HEAD_FIELD_CHECK, m_bucket, m_key, "",
                                 signature);
  if (ret) {
    return ret;
  }
  m_http->AddHeader("Authorization", digestor.Token(signature));

  //设置输出
  std::ostringstream oss, hss;
  UCloudOStream data_stream(&oss);
  UCloudOStream header_stream(&hss);
  UCloudHTTPWriteParam wp = {f : NULL, os : &data_stream};
  UCloudHTTPHeaderParam hp = {f : NULL, os : &header_stream};
  ret = m_http->RoundTrip(NULL, &wp, &hp);
  if (ret) {
    UFILE_SET_ERROR2(ERR_CPPSDK_SEND_HTTP, UFILE_LAST_ERRMSG());
    return ERR_CPPSDK_SEND_HTTP;
  }

  //解析回应
  long code = 200;
  ret = m_http->ResponseCode(&code);
  if (ret) {
    UFILE_SET_ERROR(ERR_CPPSDK_CURL);
    return ERR_CPPSDK_CURL;
  }

  std::string errmsg;
  if (code != 200) {
    int parse_ret = UFileErrorRsp(oss.str().c_str(), &ret, errmsg);
    if (parse_ret) {
      UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
      return ERR_CPPSDK_CLIENT_INTERNAL;
    }
    UFILE_SET_ERROR2(ret, errmsg);
  } else {
    ret = ParseMuploadResult(oss.str(), hss.str());
    if (ret) {
      return ret;
    }
    m_uploaded_size += length;
  }
  return ret;
}

int UFileMput::MFinish(const std::map<size_t, std::string> *etags) {

  int64_t ret = InitGlobalConfig();
  if (ret)
    return ret;

  if (etags && etags->size()) {
    m_etag_list.insert(etags->begin(), etags->end());
  }
  std::string signature("");
  //构建 HTTP 头部
  m_http->Reset();
  m_http->SetVerb("POST");
  m_http->SetURL(MFinishURL());
  m_http->AddHeader("User-Agent", USERAGENT);

  //使用 HTTP 信息构建签名
  UFileDigest digestor;
  ret = digestor.SignWithRequest(m_http, HEAD_FIELD_CHECK, m_bucket, m_key, "",
                                 signature);
  if (ret) {
    return ret;
  }
  m_http->AddHeader("Authorization", digestor.Token(signature));

  //设置输入/输出
  std::stringstream sss;
  std::ostringstream oss, hss;
  ret = ETagStream(m_etag_list, sss);
  if (ret) {
    UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
    return ERR_CPPSDK_CLIENT_INTERNAL;
  }

  UCloudOStream data_stream(&oss);
  UCloudOStream header_stream(&hss);
  UCloudHTTPReadParam rp = {f : NULL, is : sss, fsize : sss.str().size()};
  UCloudHTTPWriteParam wp = {f : NULL, os : &data_stream};
  UCloudHTTPHeaderParam hp = {f : NULL, os : &header_stream};
  ret = m_http->RoundTrip(&rp, &wp, &hp);
  if (ret) {
    UFILE_SET_ERROR2(ERR_CPPSDK_SEND_HTTP, UFILE_LAST_ERRMSG());
    return ERR_CPPSDK_SEND_HTTP;
  }

  //解析回应
  long code = 200;
  ret = m_http->ResponseCode(&code);
  if (ret) {
    UFILE_SET_ERROR(ERR_CPPSDK_CURL);
    return ERR_CPPSDK_CURL;
  }

  std::string errmsg;
  if (code != 200) {
    int parseRet = UFileErrorRsp(oss.str().c_str(), &ret, errmsg);
    if (parseRet) {
      UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
      return ERR_CPPSDK_CLIENT_INTERNAL;
    }
    UFILE_SET_ERROR2(ret, errmsg);
  } else {
    ret = ParseMFinishResult(oss.str(), hss.str());
    if (ret) {
      return ret;
    }
  }
  return ret;
}

int UFileMput::ParseInitResult(const std::string &data) {

  int ret = JsonGetString(data, UPLOADID_KEY, m_uploadid);
  if (ret) {
    UFILE_SET_ERROR(ERR_CPPSDK_PARSE_JSON);
    return ERR_CPPSDK_PARSE_JSON;
  }

  ret = JsonGetSizet(data, BLKSIZE_KEY, m_blksize);
  if (ret) {
    UFILE_SET_ERROR(ERR_CPPSDK_PARSE_JSON);
    return ERR_CPPSDK_PARSE_JSON;
  }
  return 0;
}

int UFileMput::ParseMuploadResult(const std::string &body,
                                  const std::string &header) {

  size_t blk_idx;
  int ret = JsonGetSizet(body, PART_NUMBER_KEY, blk_idx);
  if (ret) {
    UFILE_SET_ERROR(ERR_CPPSDK_PARSE_JSON);
    return ERR_CPPSDK_PARSE_JSON;
  }

  std::map<std::string, std::string> headers;
  ret = ucloud::cppsdk::http::ParseHeadersFromString(header, headers);
  if (ret) {
    UFILE_SET_ERROR(ERR_CPPSDK_PARSE_JSON);
    return ERR_CPPSDK_PARSE_JSON;
  }
  std::string etag;
  if (headers.find("ETag") != headers.end()) {
    etag = headers["ETag"];
  } else {
    etag = headers["Etag"];
  }
  if (etag == "") {
    UFILE_SET_ERROR(ERR_CPPSDK_INVALID_ETAG);
    return ERR_CPPSDK_INVALID_ETAG;
  } else {
    m_etag_list[blk_idx] = etag;
  }
  headers.clear();
  return 0;
}

int UFileMput::ParseMFinishResult(const std::string &body,
                                  const std::string &header) {

  int ret = JsonGetSizet(body, FILESIZE_KEY, m_finished_fsize);
  if (ret) {
    UFILE_SET_ERROR(ERR_CPPSDK_PARSE_JSON);
    return ERR_CPPSDK_PARSE_JSON;
  }

  std::map<std::string, std::string> headers;
  ret = ucloud::cppsdk::http::ParseHeadersFromString(header, headers);
  if (ret) {
    UFILE_SET_ERROR(ERR_CPPSDK_PARSE_JSON);
    return ERR_CPPSDK_PARSE_JSON;
  }
  m_final_etag = headers["ETag"];
  if (m_final_etag == "") {
    UFILE_SET_ERROR(ERR_CPPSDK_INVALID_ETAG);
    return ERR_CPPSDK_INVALID_ETAG;
  }
  return 0;
}

int UFileMput::JudgeBlksize(const size_t blk_idx, size_t &bsize) {

  if (!m_is)
    return -1;

  std::streampos old = m_is->tellg();
  if (m_is->fail()) {
    return -1;
  }

  m_is->seekg(0, m_is->end);
  if (m_is->fail()) {
    return -1;
  }
  std::streampos end = m_is->tellg();
  if (m_is->fail()) {
    return -1;
  }

  if (end - old > std::streampos(m_blksize)) {
    bsize = m_blksize;
  } else {
    bsize = end - old;
  }

  m_is->seekg(old, m_is->beg);
  if (m_is->fail()) {
    return -1;
  }
  return 0;
}

std::string UFileMput::MUploadURL() {

  std::stringstream oss;
  oss << UFileHost(m_bucket) + EasyPathEscape(m_key) +
             "?uploadId=" + EasyQueryEscape(m_uploadid);
  oss << "&partNumber=" << m_blk_idx;
  return oss.str();
}

std::string UFileMput::MFinishURL() {

  std::stringstream oss;
  oss << UFileHost(m_bucket) + EasyPathEscape(m_key) +
             "?uploadId=" + EasyQueryEscape(m_uploadid);
  return oss.str();
}

int UFileMput::ETagStream(const std::map<size_t, std::string> &etag_list,
                          std::stringstream &sss) {

  std::map<size_t, std::string>::const_iterator it = etag_list.begin();
  for (; it != etag_list.end(); ++it) {
    sss << it->second << ",";
  }
  return 0;
}

} // namespace api
} // namespace cppsdk
} // namespace ucloud
