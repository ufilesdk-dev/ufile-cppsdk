#ifndef _UFILESDK_CPP_UCLOUD_HTTP_HTTP_
#define _UFILESDK_CPP_UCLOUD_HTTP_HTTP_

#include <curl/curl.h>
#include <istream>
#include <map>
#include <ostream>
#include <string>
#include <ufile-cppsdk/stream_util.h>

#define SCHEME ("http://")

namespace ucloud {
namespace cppsdk {
namespace http {

typedef struct {
  FILE *f;
  ucloud::cppsdk::utils::UCloudOStream *os;
} UCloudHTTPWriteParam;

typedef struct {
  FILE *f;
  std::istream &is;
  size_t fsize;
  //需要读取的总字节数
  size_t need_total_n;
  //已经读取的总字节数
  size_t read_total_n;
} UCloudHTTPReadParam;

typedef struct {
  FILE *f;
  ucloud::cppsdk::utils::UCloudOStream *os;
  //需要读取的总字节数
  size_t need_total_n;
  //已经读取的总字节数
  size_t read_total_n;
} UCloudHTTPHeaderParam;

class HTTPRequest {

public:
  explicit HTTPRequest(const char *verb);
  HTTPRequest(const char *verb, const char *url);
  HTTPRequest();
  ~HTTPRequest();

  void Reset();
  void SetURL(const std::string &url);
  void SetVerb(const std::string &verb);
  void SetQuery(const std::string &k, const std::string &v);
  void SetRange(const std::pair<ssize_t, ssize_t> &range);
  void AddHeader(const std::string &k, const std::string &v);
  void InitOption();
  int RoundTrip(const UCloudHTTPReadParam *rp, const UCloudHTTPWriteParam *wp,
                const UCloudHTTPHeaderParam *hp, bool nobody = false);
  int ResponseCode(long *code);
  int ParseURL();
  int ReqHeaders(std::map<std::string, std::string> &headers);
  std::string Query(const std::string &k);
  std::string ReqHeader(const std::string &k);
  inline std::string HTTPVerb() { return m_verb; }

private:
  CURL *m_curl;
  struct curl_slist *m_req_headers;
  std::string m_url;
  std::string m_verb;
  std::map<std::string, std::string> m_query;
};

class HTTPResponse {

public:
  HTTPResponse();
  ~HTTPResponse();
};

int ParseHeadersFromString(const std::string &header,
                           std::map<std::string, std::string> &headers);
int WriteToStream(ucloud::cppsdk::utils::UCloudOStream *os,
                  const std::string &ptr);
size_t write_cb(char *ptr, size_t size, size_t nmemb, void *userdata);
size_t read_cb(char *buffer, size_t size, size_t nitems, void *userdata);
size_t write_header_cb(void *ptr, size_t size, size_t nmemb, void *userdata);

} // namespace http
} // namespace cppsdk
} // namespace ucloud

#endif
