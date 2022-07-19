#ifndef _UFILESDK_CPP_UCLOUD_API_COMMON_
#define _UFILESDK_CPP_UCLOUD_API_COMMON_

#include <json-c/json.h>
#include <string>

namespace ucloud {
namespace cppsdk {
namespace utils {

inline int JsonGetString(const json_object *root, const std::string &key,
                         std::string &value) {
  if (!root)
    return -1;
  json_object *node = NULL;
  if (json_object_object_get_ex((json_object *)root, key.c_str(), &node)) {
    value = json_object_get_string(node);
    return 0;
  }
  //不存在
  return 1;
}

inline int JsonGetInt64(const json_object *root, const std::string &key,
                        int64_t &value) {

  if (!root)
    return -1;
  json_object *node = NULL;
  if (json_object_object_get_ex((json_object *)root, key.c_str(), &node)) {
    value = json_object_get_int64(node);
    return 0;
  }
  //不存在
  return 1;
}

inline int JsonGetSizet(const json_object *root, const std::string &key,
                        size_t &value) {

  int64_t v;
  int ret = JsonGetInt64(root, key, v);
  if (ret == 0) {
    value = v;
  }
  return ret;
}

inline int JsonGetBool(const json_object *root, const std::string &key,
                       bool &value) {

  if (!root)
    return -1;
  json_object *node = NULL;
  if (json_object_object_get_ex((json_object *)root, key.c_str(), &node)) {
    value = json_object_get_boolean(node);
    return 0;
  }
  //不存在
  return 1;
}

inline int JsonGetArray(const json_object *root, const std::string &key,
                        json_object *&value) {
  if (!root) {
    return -1;
  }
  json_object *jarray;
  if (json_object_object_get_ex((json_object *)root, key.c_str(), &jarray)) {
    if (json_object_get_type(jarray) == json_type_array) {
      value = jarray;
      return 0;
    }
  }
  //不存在
  return 1;
}

inline int JsonGetString(const std::string &data, const std::string &key,
                         std::string &value) {

  json_object *root = json_tokener_parse(data.c_str());
  if (!root)
    return -1;

  int ret = JsonGetString(const_cast<const json_object *>(root), key, value);
  json_object_put(root);
  return ret;
}

inline int JsonGetInt64(const std::string &data, const std::string &key,
                        int64_t &value) {

  json_object *root = json_tokener_parse(data.c_str());
  if (!root)
    return -1;

  int ret = JsonGetInt64(const_cast<const json_object *>(root), key, value);
  json_object_put(root);
  return ret;
}

inline int JsonGetSizet(const std::string &data, const std::string &key,
                        size_t &value) {

  json_object *root = json_tokener_parse(data.c_str());
  if (!root)
    return -1;

  int ret = JsonGetSizet(const_cast<const json_object *>(root), key, value);
  json_object_put(root);
  return ret;
}

} // namespace utils
} // namespace cppsdk
} // namespace ucloud

#endif
