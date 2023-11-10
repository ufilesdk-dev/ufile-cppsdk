#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <ufile-cppsdk/common.h>
#include <ufile-cppsdk/config.h>
#include <ufile-cppsdk/digest.h>
#include <ufile-cppsdk/errno.h>
#include <ufile-cppsdk/http.h>
#include <ufile-cppsdk/json_util.h>
#include <ufile-cppsdk/listparts.h>
#include <ufile-cppsdk/urlcodec.h>

using namespace ucloud::cppsdk::http;
using namespace ucloud::cppsdk::utils;
using namespace ucloud::cppsdk::error;
using namespace ucloud::cppsdk::digest;
using namespace ucloud::cppsdk::config;

namespace ucloud {
namespace cppsdk {
namespace api {

UFileListParts::UFileListParts() {}

UFileListParts::~UFileListParts() {}

int UFileListParts::ListParts(
    const std::string &bucket, const std::string &uploadid,
    uint32_t count, ListPartsResult *result, bool *is_truncated,
    int32_t *next_marker, int32_t marker) {
  int64_t ret = InitGlobalConfig();
  if (ret) {
    return ret;
  }

  result->clear();

  std::string signature("");
  // 构建HTTP头部
  m_http->Reset();
  m_http->AddHeader("User-Agent", USERAGENT);
  m_http->AddHeader("Content-Length", std::to_string(0));
  std::map<std::string, std::string> params;
  params["uploadId"] = uploadid;
  params["max-parts"] = std::to_string(count);
  params["part-number-marker"] = std::to_string(marker);
  SetURL(bucket, params);

  //使用 HTTP 信息构建签名
  UFileDigest digestor;
  ret = digestor.SignWithRequest(m_http, HEAD_FIELD_CHECK, bucket, "", "",
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
    int parse_ret =
        ParseRsp(oss.str().c_str(), result, is_truncated, next_marker);
    if (parse_ret) {
      UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
      return ERR_CPPSDK_CLIENT_INTERNAL;
    }
  }
  return ret;
}

void UFileListParts::SetURL(const std::string &bucket,
      std::map<std::string, std::string> params) {
  std::string url = UFileHost(bucket);
  url = url + "?muploadpart";
  for (auto it = params.begin(); it != params.end(); ++it) {
    url = url + "&" + it->first + "=" + it->second;
  }
  m_http->SetURL(url);
}

int UFileListParts::ParseRsp(const char *body,
    ListPartsResult *result, bool *is_truncated, int32_t *next_marker) {
  json_object *root = json_tokener_parse(body);
  if (!root) {
    return -1;
  }

  int ret;
  if (is_truncated != nullptr) {
    ret = JsonGetBool(root, "IsTruncated", *is_truncated);
    if (ret) {
      json_object_put(root);
      return ret;
    }
  }

  if (next_marker != nullptr) {
    ret = JsonGetInt32(root, "NextPartNumberMarker", *next_marker);
    if (ret) {
      json_object_put(root);
      return ret;
    }
  }

  if (result != nullptr) {
    json_object *parts;
    ret = JsonGetArray(root, "Parts", parts);
    if (ret) {
      json_object_put(root);
      return ret;
    }
    int num_parts = json_object_array_length(parts);
    for (int i = 0; i < num_parts; i++) {
      json_object *part = json_object_array_get_idx(parts, i);
      ListPartsResultEntry entry;
      ret = JsonGetInt32(part, "PartNum", entry.part_number);
      if (ret) {
        json_object_put(root);
        return ret;
      }

      ret = JsonGetString(part, "Etag", entry.etag);
      if (ret) {
        json_object_put(root);
        return ret;
      }

      ret = JsonGetUInt64(part, "LastModified", entry.last_modified);
      if (ret) {
        json_object_put(root);
        return ret;
      }

      ret = JsonGetUInt64(part, "Size", entry.size);
      if (ret) {
        json_object_put(root);
        return ret;
      }

      if (result) {
        result->push_back(entry);
      }
    }
  }

  json_object_put(root);
  return 0;
}

} // namespace api
} // namespace cppsdk
} // namespace ucloud
