#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <ufile-cppsdk/common.h>
#include <ufile-cppsdk/config.h>
#include <ufile-cppsdk/digest.h>
#include <ufile-cppsdk/errno.h>
#include <ufile-cppsdk/headfile.h>
#include <ufile-cppsdk/http.h>
#include <ufile-cppsdk/stream_util.h>
#include <ufile-cppsdk/urlcodec.h>
#include <unistd.h>

using namespace ucloud::cppsdk::http;
using namespace ucloud::cppsdk::error;
using namespace ucloud::cppsdk::utils;
using namespace ucloud::cppsdk::digest;

namespace ucloud {
namespace cppsdk {
namespace api {

int UFileHeadfile::GetFileInfo(const std::string &bucket,
                               const std::string &key, FileInfo *result) {
  int64_t ret = InitGlobalConfig();
  if (ret) {
    return ret;
  }

  SetResource(bucket, key);

  std::string signature("");
  // 构建HTTP头部
  m_http->Reset();
  m_http->SetVerb("HEAD");
  m_http->SetURL(UFileHost(bucket) + EasyPathEscape(key));
  m_http->AddHeader("User-Agent", USERAGENT);

  //使用 HTTP 信息构建签名
  UFileDigest digestor;
  ret = digestor.SignWithRequest(m_http, HEAD_FIELD_CHECK, bucket, key, "",
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
  ret = m_http->RoundTrip(NULL, &wp, &hp, true);
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
    return code;
  } else {
    ret = ParseRsp(hss.str().c_str(), result);
  }
  return ret;
}

void UFileHeadfile::SetResource(const std::string &bucket,
                                const std::string &key) {
  bucket_ = bucket;
  key_ = key;
}

int UFileHeadfile::ParseRsp(const char *header, FileInfo *result) {
  std::map<std::string, std::string> headers;
  int ret = ucloud::cppsdk::http::ParseHeadersFromString(header, headers);
  if (ret) {
    UFILE_SET_ERROR(ERR_CPPSDK_PARSE_JSON);
    return ERR_CPPSDK_PARSE_JSON;
  }
  if (headers.find("Content-Type") != headers.end()) {
    result->type = headers["Content-Type"];
  }
  if (headers.find("Content-Length") != headers.end()) {
    result->size = std::stoul(headers["Content-Length"]);
  }
  if (headers.find("ETag") != headers.end()) {
    result->etag = headers["ETag"];
  } else if (headers.find("Etag") != headers.end()) {
    result->etag = headers["Etag"];
  }
  if (headers.find("Last-Modified") != headers.end()) {
    result->last_modified = headers["Last-Modified"];
  }
  if (headers.find("X-Ufile-Create-Time") != headers.end()) {
    result->create_time = headers["X-Ufile-Create-Time"];
  }
  if (headers.find("X-Ufile-Storage-Class") != headers.end()) {
    result->storage_class = headers["X-Ufile-Storage-Class"];
  }
  return 0;
}

} // namespace api
} // namespace cppsdk
} // namespace ucloud
