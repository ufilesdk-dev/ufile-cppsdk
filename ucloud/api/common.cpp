#include <json-c/json.h>
#include <string>
#include <ufile-cppsdk/common.h>
#include <ufile-cppsdk/errno.h>
#include <ufile-cppsdk/json_util.h>
#include <ufile-cppsdk/mimetype.h>

using namespace ucloud::cppsdk::utils;
using namespace ucloud::cppsdk::error;
using namespace ucloud::cppsdk::http;

namespace ucloud {
namespace cppsdk {
namespace api {

int UFileErrorRsp(const char *body, int64_t *retcode, std::string &errmsg) {

  json_object *root = json_tokener_parse(body);
  if (!root)
    return -1;

  int ret = JsonGetInt64(root, "RetCode", *retcode);
  if (ret) {
    json_object_put(root);
    return ret;
  }

  ret = JsonGetString(root, "ErrMsg", errmsg);
  if (ret) {
    json_object_put(root);
    return ret;
  }

  json_object_put(root);
  return 0;
}

int FetchContentLength(std::istream &is, std::streampos *fsize) {

  is.clear();
  std::streampos old = is.tellg();
  if (is.fail()) {
    UFILE_SET_ERROR(ERR_CPPSDK_SEEKABLE);
    return ERR_CPPSDK_SEEKABLE;
  }

  is.seekg(0, is.end);
  if (is.fail()) {
    UFILE_SET_ERROR(ERR_CPPSDK_SEEKABLE);
    return ERR_CPPSDK_SEEKABLE;
  }
  *fsize = is.tellg();
  if (*fsize < 0) {
    UFILE_SET_ERROR(ERR_CPPSDK_SEEKABLE);
    return ERR_CPPSDK_SEEKABLE;
  }
  is.clear();
  is.seekg(old, is.beg);
  if (is.fail()) {
    UFILE_SET_ERROR(ERR_CPPSDK_SEEKABLE);
    return ERR_CPPSDK_SEEKABLE;
  }
  return 0;
}

int PeekData(std::istream &is, const int n, std::string &data) {

  std::streampos old = is.tellg();
  if (is.fail())
    return -1;

  int need2peek = n;
  while (--need2peek > 0) {
    int ch = is.peek();
    if (ch == EOF)
      break;
    if (is.fail()) {
      is.seekg(old, is.beg);
      return -1;
    }
    data.append(1, ch);
    is.seekg(1, is.cur);
    if (is.fail()) {
      is.seekg(old, is.beg);
      return -1;
    }
  }
  is.seekg(old, is.beg);
  is.clear();
  return 0;
}

int MimeType(const std::string &filename, std::string &mimetype) {
  mimetype = MimeTypeInducer::Instance().InduceType(filename);
  return 0;
}

} // namespace api
} // namespace cppsdk
} // namespace ucloud
