#include <iostream>
#include <sstream>
#include <string>
#include <ufile-cppsdk/common.h>
#include <ufile-cppsdk/config.h>
#include <ufile-cppsdk/digest.h>
#include <ufile-cppsdk/errno.h>
#include <ufile-cppsdk/json_util.h>
#include <ufile-cppsdk/string_util.h>
#include <ufile-cppsdk/tagging.h>
#include <ufile-cppsdk/urlcodec.h>

using namespace ucloud::cppsdk::config;
using namespace ucloud::cppsdk::digest;
using namespace ucloud::cppsdk::error;
using namespace ucloud::cppsdk::http;
using namespace ucloud::cppsdk::utils;

namespace ucloud {
namespace cppsdk {
namespace api {

UFileTagging::UFileTagging() : APIBase() {}

UFileTagging::~UFileTagging() {}

int UFileTagging::PutTagging(const std::string &bucket, const std::string &key,
                             std::map<std::string, std::string> &tags) {
  int64_t ret = InitGlobalConfig();
  if (ret)
    return ret;

  std::string tag_str;
  if (TaggingToJsonString(tags, &tag_str) != 0) {
    UFILE_SET_ERROR(ERR_CPPSDK_PARSE_JSON);
    return ERR_CPPSDK_PARSE_JSON;
  }

  std::istringstream is(tag_str);

  std::string signature("");
  //构建 HTTP 头部
  m_http->Reset();
  m_http->SetVerb("PUT");
  m_http->AddHeader("Content-Length", SIZET2STR(tag_str.size()));
  m_http->AddHeader("User-Agent", USERAGENT);
  m_http->SetURL(UFileHost(bucket) + EasyPathEscape(key) + "?tagging");
  //使用 HTTP 信息构建签名
  UFileDigest digestor;
  ret = digestor.SignWithRequest(m_http, HEAD_FIELD_CHECK, bucket, key, "",
                                 signature);
  if (ret) {
    return ret;
  }
  m_http->AddHeader("Authorization", digestor.Token(signature));

  //设置数据源
  std::ostringstream oss;
  UCloudOStream data_stream(&oss);
  UCloudHTTPReadParam rp = {
    f : NULL,
    is : is,
    fsize : tag_str.size(),
    need_total_n : tag_str.size()
  };
  UCloudHTTPWriteParam wp = {f : NULL, os : &data_stream};
  ret = m_http->RoundTrip(&rp, &wp, NULL);
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
  }
  return ret;
}

int UFileTagging::GetTagging(const std::string &bucket, const std::string &key,
                             std::map<std::string, std::string> *tags) {
  int64_t ret = InitGlobalConfig();
  if (ret)
    return ret;

  std::string signature("");
  //构建 HTTP 头部
  m_http->Reset();
  m_http->AddHeader("User-Agent", USERAGENT);
  m_http->AddHeader("Content-Length", std::to_string(0));
  m_http->SetURL(UFileHost(bucket) + EasyPathEscape(key) + "?tagging");

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
  if (code != 200) {
    int parse_ret = UFileErrorRsp(oss.str().c_str(), &ret, errmsg);
    if (parse_ret) {
      UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
      return ERR_CPPSDK_CLIENT_INTERNAL;
    }
    UFILE_SET_ERROR2(ret, errmsg);
  } else {
    int parse_ret = ParseTaggingFromJsonString(oss.str().c_str(), tags);
    if (parse_ret) {
      UFILE_SET_ERROR(ERR_CPPSDK_PARSE_JSON);
      return ERR_CPPSDK_PARSE_JSON;
    }
  }
  return ret;
}

int UFileTagging::DeleteTagging(const std::string &bucket,
                                const std::string &key) {
  int64_t ret = InitGlobalConfig();
  if (ret)
    return ret;

  std::string signature("");
  //构建 HTTP 头部
  m_http->Reset();
  m_http->SetVerb("DELETE");
  m_http->AddHeader("Content-Length", std::to_string(0));
  m_http->AddHeader("User-Agent", USERAGENT);
  m_http->SetURL(UFileHost(bucket) + EasyPathEscape(key) + "?tagging");
  //使用 HTTP 信息构建签名
  UFileDigest digestor;
  ret = digestor.SignWithRequest(m_http, HEAD_FIELD_CHECK, bucket, key, "",
                                 signature);
  if (ret) {
    return ret;
  }
  m_http->AddHeader("Authorization", digestor.Token(signature));

  //设置数据源
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
  if (code != 200 && code != 204) {
    int parse_ret = UFileErrorRsp(oss.str().c_str(), &ret, errmsg);
    if (parse_ret) {
      UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
      return ERR_CPPSDK_CLIENT_INTERNAL;
    }
    UFILE_SET_ERROR2(ret, errmsg);
  }
  return ret;
}

int UFileTagging::TaggingToJsonString(std::map<std::string, std::string> &tags,
                                      std::string *json_str) {
  /*
  example:
  {
      "Tags": [
          {
              "Key": "key1",
              "Value": "val1"
          },
          {
              "Key": "key2",
              "Value": "val2"
          }
      ]
  }
  */
  json_object *root = json_object_new_object();
  if (!root) {
    return -1;
  }

  json_object *json_array = json_object_new_array();
  if (!json_array) {
    json_object_put(root);
    return -1;
  }
  if (json_object_object_add(root, "Tags", json_array) != 0) {
    json_object_put(root);
    json_object_put(json_array);
    return -1;
  }

  for (auto it = tags.begin(); it != tags.end(); ++it) {
    json_object *tag = json_object_new_object();
    if (!tag) {
      json_object_put(root);
      return -1;
    }
    if (json_object_object_add(
            tag, "Key", json_object_new_string(it->first.c_str())) != 0 ||
        json_object_object_add(
            tag, "Value", json_object_new_string(it->second.c_str())) != 0 ||
        json_object_array_add(json_array, tag) != 0) {
      json_object_put(root);
      json_object_put(tag);
      return -1;
    }
  }

  *json_str = json_object_to_json_string(root);
  json_object_put(root);
  return 0;
}

int UFileTagging::ParseTaggingFromJsonString(
    const std::string &json_str, std::map<std::string, std::string> *tags) {
  json_object *root = json_tokener_parse(json_str.c_str());
  if (!root) {
    return -1;
  }

  json_object *json_array = json_object_object_get(root, "Tags");
  if (!json_array) {
    json_object_put(root);
    return -1;
  }
  if (!json_object_is_type(json_array, json_type_array)) {
    json_object_put(root);
    return -1;
  }

  size_t array_len = json_object_array_length(json_array);
  for (size_t i = 0; i < array_len; i++) {
    json_object *tag = json_object_array_get_idx(json_array, i);
    if (!tag) {
      json_object_put(root);
      return -1;
    }

    std::string key, value;
    if (JsonGetString(tag, "Key", key) != 0 ||
        JsonGetString(tag, "Value", value) != 0) {
      json_object_put(root);
      return -1;
    }
    tags->insert(std::make_pair(key, value));
  }
  json_object_put(root);
  return 0;
}

} // namespace api
} // namespace cppsdk
} // namespace ucloud
