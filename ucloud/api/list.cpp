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
#include <ufile-cppsdk/list.h>
#include <ufile-cppsdk/urlcodec.h>

#define LIST_MAX_COUNT (1000)

using namespace ucloud::cppsdk::http;
using namespace ucloud::cppsdk::utils;
using namespace ucloud::cppsdk::error;
using namespace ucloud::cppsdk::digest;
using namespace ucloud::cppsdk::config;

namespace ucloud {
namespace cppsdk {
namespace api {

UFileList::UFileList() : delimiter_("") {}

UFileList::~UFileList() {}

int UFileList::List(const std::string &bucket, const std::string &prefix,
                    uint32_t count, ListResult *result, bool *is_truncated,
                    std::string *next_marker, const std::string &marker) {
  int64_t ret = InitGlobalConfig();
  if (ret) {
    return ret;
  }

  result->clear();
  SetResource(bucket, prefix, count, marker);

  std::string signature("");
  // 构建HTTP头部
  m_http->Reset();
  m_http->AddHeader("User-Agent", USERAGENT);
  m_http->AddHeader("Content-Length", std::to_string(0));
  std::map<std::string, std::string> params;
  params["prefix"] = prefix;
  params["marker"] = marker;
  params["max-keys"] = std::to_string(count);
  params["delimiter"] = delimiter_;
  SetURL(params);

  //使用 HTTP 信息构建签名
  UFileDigest digestor;
  ret = digestor.SignWithRequestV2(m_http, HEAD_FIELD_CHECK, bucket, "", "",
                                   signature, "listobjects", params);
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

int UFileList::ListDir(const std::string &bucket, const std::string &prefix,
                       uint32_t count, std::list<std::string> *result,
                       bool *is_truncated, std::string *next_marker,
                       const std::string &marker) {
  int64_t ret = InitGlobalConfig();
  if (ret) {
    return ret;
  }

  result->clear();
  SetResource(bucket, prefix, count, marker, "/");

  std::string signature("");
  // 构建HTTP头部
  m_http->Reset();
  m_http->AddHeader("User-Agent", USERAGENT);
  m_http->AddHeader("Content-Length", std::to_string(0));
  std::map<std::string, std::string> params;
  params["prefix"] = prefix;
  params["marker"] = marker;
  params["max-keys"] = std::to_string(count);
  params["delimiter"] = delimiter_;
  SetURL(params);

  //使用 HTTP 信息构建签名
  UFileDigest digestor;
  ret = digestor.SignWithRequestV2(m_http, HEAD_FIELD_CHECK, bucket, "", "",
                                   signature, "listobjects", params);
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
        ParseRsp(oss.str().c_str(), nullptr, is_truncated, next_marker, result);
    if (parse_ret) {
      UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
      return ERR_CPPSDK_CLIENT_INTERNAL;
    }
  }
  return ret;
}

void UFileList::SetResource(const std::string &bucket,
                            const std::string &prefix, uint32_t count,
                            const std::string &marker, std::string delimiter) {
  bucket_ = bucket;
  prefix_ = prefix;
  max_key_ = count > LIST_MAX_COUNT ? LIST_MAX_COUNT : count;
  marker_ = marker;
  delimiter_ = delimiter;
}

void UFileList::SetURL(std::map<std::string, std::string> params) {
  std::string url = UFileHost(bucket_);
  url = url + "?listobjects";
  for (auto it = params.begin(); it != params.end(); ++it) {
    url = url + "&" + it->first + "=" + it->second;
  }
  m_http->SetURL(url);
}

int UFileList::ParseRsp(const char *body, ListResult *result,
                        bool *is_truncated, std::string *next_marker,
                        std::list<std::string> *prefixes) {
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
    ret = JsonGetString(root, "NextMarker", *next_marker);
    if (ret) {
      json_object_put(root);
      return ret;
    }
  }

  if (result != nullptr || prefixes != nullptr) {
    json_object *contents;
    ret = JsonGetArray(root, "Contents", contents);
    if (ret) {
      json_object_put(root);
      return ret;
    }
    int num_key = json_object_array_length(contents);
    for (int i = 0; i < num_key; i++) {
      json_object *content = json_object_array_get_idx(contents, i);
      ListResultEntry entry;
      ret = JsonGetString(content, "Key", entry.filename);
      if (ret) {
        json_object_put(root);
        return ret;
      }

      ret = JsonGetString(content, "MimeType", entry.mime_type);
      if (ret) {
        json_object_put(root);
        return ret;
      }

      ret = JsonGetString(content, "ETag", entry.etag);
      if (ret) {
        ret = JsonGetString(content, "Etag", entry.etag);
        if (ret) {
          json_object_put(root);
          return ret;
        }
      }

      std::string size_str;
      ret = JsonGetString(content, "Size", size_str);
      if (ret) {
        json_object_put(root);
        return ret;
      }
      entry.size = std::stol(size_str);

      ret = JsonGetString(content, "StorageClass", entry.storage_class);
      if (ret) {
        json_object_put(root);
        return ret;
      }

      int64_t value;
      ret = JsonGetInt64(content, "LastModified", value);
      if (ret) {
        json_object_put(root);
        return ret;
      }
      entry.last_modified = value;

      ret = JsonGetInt64(content, "CreateTime", value);
      if (ret) {
        json_object_put(root);
        return ret;
      }
      entry.create_time = value;

      if (result) {
        result->push_back(entry);
      }
      if (prefixes) {
        prefixes->push_back(entry.filename);
      }
    }
  }

  // for ListDir
  if (prefixes != nullptr) {
    json_object *common_prefixes;
    ret = JsonGetArray(root, "CommonPrefixes", common_prefixes);
    int num_key = json_object_array_length(common_prefixes);
    for (int i = 0; i < num_key; i++) {
      json_object *common_prefix =
          json_object_array_get_idx(common_prefixes, i);
      std::string prefix;
      ret = JsonGetString(common_prefix, "Prefix", prefix);
      if (ret) {
        json_object_put(root);
        return ret;
      }
      prefixes->push_back(prefix);
    }
  }

  json_object_put(root);
  return 0;
}

} // namespace api
} // namespace cppsdk
} // namespace ucloud
