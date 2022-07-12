#include <sstream>
#include <ufile-cppsdk/common.h>
#include <ufile-cppsdk/config.h>
#include <ufile-cppsdk/delete.h>
#include <ufile-cppsdk/digest.h>
#include <ufile-cppsdk/errno.h>
#include <ufile-cppsdk/http.h>
#include <ufile-cppsdk/stream_util.h>
#include <ufile-cppsdk/urlcodec.h>

using namespace ucloud::cppsdk::http;
using namespace ucloud::cppsdk::utils;
using namespace ucloud::cppsdk::error;
using namespace ucloud::cppsdk::digest;
using namespace ucloud::cppsdk::config;

namespace ucloud {
namespace cppsdk {
namespace api {

UFileDelete::UFileDelete() {}

UFileDelete::~UFileDelete() {}

int UFileDelete::Delete(const std::string &bucket, const std::string &key) {

  int64_t ret = InitGlobalConfig();
  if (ret)
    return ret;

  std::string signature("");
  //构建 HTTP 头部
  m_http->Reset();
  m_http->SetVerb("DELETE");
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
  if (code != 204) {
    int parse_ret = UFileErrorRsp(oss.str().c_str(), &ret, errmsg);
    if (parse_ret) {
      UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
      return ERR_CPPSDK_CLIENT_INTERNAL;
    }
    UFILE_SET_ERROR2(ret, errmsg);
  }
  return ret;
}

} // namespace api
} // namespace cppsdk
} // namespace ucloud
