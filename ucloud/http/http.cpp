#include "http.h"
#include <string.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "errno/errno.h"
#include "utils/network_util.h"
#include "utils/string_util.h"

#if defined(_WIN32)
#pragma comment(lib, "curllib.lib")
#endif

using namespace ucloud::cppsdk::error;
using namespace ucloud::cppsdk::utils;

namespace ucloud {
namespace cppsdk {
namespace http   {

HTTPRequest::HTTPRequest(const char *verb) {

    m_verb = verb;
    m_req_headers = NULL;

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    m_curl = curl_easy_init();
    InitOption();
}

HTTPRequest::HTTPRequest(const char *verb, const char *url) {

    m_verb = verb;
    m_req_headers = NULL;

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    m_curl = curl_easy_init();
    SetURL(url);
    InitOption();
}

HTTPRequest::~HTTPRequest() {

    if (m_curl) curl_easy_cleanup(m_curl);
    if (m_req_headers) curl_slist_free_all(m_req_headers);
}

void HTTPRequest::Reset() {

    curl_easy_reset(m_curl);
    curl_slist_free_all(m_req_headers);
    m_req_headers = NULL;
}

void HTTPRequest::SetVerb(const std::string &verb) {

    m_verb = verb;
    if (m_verb != "") {
        curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, m_verb.c_str());
    }
}

void HTTPRequest::SetQuery(const std::string &k, const std::string &v) {

    m_query[k] = v;
}

void HTTPRequest::SetURL(const std::string &url) {

    m_url = url;
    curl_easy_setopt(m_curl, CURLOPT_URL, m_url.c_str());
    ParseURL();
}

void HTTPRequest::SetRange(const std::pair<ssize_t, ssize_t> &range) {

    std::ostringstream oss;
    oss << "bytes=";
    if (range.first != -1) oss << range.first;
    oss << "-";
    if (range.second != -1) oss << range.second;
    if (m_curl) {
        m_req_headers = curl_slist_append(m_req_headers, ("Range: " + oss.str()).c_str());
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_req_headers);
    }
}

int HTTPRequest::ParseURL() {

    std::string uri = m_url;
    size_t pos = uri.find(strlen(SCHEME), '/');
    if (pos != std::string::npos) {
        pos = uri.find(pos, '?');
        if (pos != std::string::npos) {
            std::string querystring = uri.substr(pos+1);
            uri = uri.substr(0, pos);

            size_t qpos = 0;
            size_t qprev = qpos;
            while(true) {
                qprev = qpos;
                if ((qpos = querystring.find(qpos, '&')) != std::string::npos) {
                    std::string query = querystring.substr(qprev, qpos);
                    size_t equalpos = query.find('=');
                    if (equalpos != std::string::npos) {
                        m_query[query.substr(qprev, equalpos)] = query.substr(equalpos+1);
                    } else {
                        m_query[query.substr(qprev, equalpos)] = "";
                    }
                }
            }
        }
    }
    return 0;
}

void HTTPRequest::InitOption() {

    if (m_curl) {
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
        if (m_url != "") curl_easy_setopt(m_curl, CURLOPT_URL, m_url.c_str());
        if (m_verb != "") curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, m_verb.c_str());
    }
}

void HTTPRequest::AddHeader(const std::string &k, const std::string &v) {

    m_req_headers = curl_slist_append(m_req_headers, (k + ": " + v).c_str());
    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_req_headers);
}

std::string HTTPRequest::ReqHeader(const std::string &k) {

    struct curl_slist *header = m_req_headers;
    for(; header ; header = header->next) {
        const char *data = header->data;
        if (strlen(data) < k.size()) continue;
        if (memcmp(k.c_str(), data, k.size()) == 0) {
            char *pos = (char *)strchr(data, ':');
            if (!pos) return "";
            std::string v = pos + 1;
            ucloud::cppsdk::utils::Trim(v, " ");
            return v;
        }
    }
    return "";
}

int HTTPRequest::ReqHeaders(std::map<std::string, std::string> &headers) {

    struct curl_slist *header = m_req_headers;
    for(; header ; header = header->next) {
        char *data = header->data;
        char *pos = strchr(data, ':');
        if (pos) headers.insert(std::make_pair<std::string, std::string>(std::string(data, pos), std::string(pos+1)));
    }
    return 0;
}

std::string HTTPRequest::Query(const std::string &k) {

    std::map<std::string, std::string>::iterator it = m_query.find(k);
    if (it == m_query.end()) return "";
    return it->second;
}

int HTTPRequest::RoundTrip(const UCloudHTTPReadParam *rp
        , const UCloudHTTPWriteParam *wp
        , const UCloudHTTPHeaderParam *hp) {

    CURLcode code = CURLE_OK;
    if (wp) {
        code = curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_cb);
        if (code != CURLE_OK) {
            UFILE_SET_ERROR2(ERR_CPPSDK_SET_CURLOPT, curl_easy_strerror(code));
            return ERR_CPPSDK_SET_CURLOPT;
        }
        code = curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, wp);
        if (code != CURLE_OK) {
            UFILE_SET_ERROR2(ERR_CPPSDK_SET_CURLOPT, curl_easy_strerror(code));
            return ERR_CPPSDK_SET_CURLOPT;
        }
    }

    if (rp) {
        code = curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, read_cb);
        if (code != CURLE_OK) {
            UFILE_SET_ERROR2(ERR_CPPSDK_SET_CURLOPT, curl_easy_strerror(code));
            return ERR_CPPSDK_SET_CURLOPT;
        }
        code = curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 1L);
        if (code != CURLE_OK) {
            UFILE_SET_ERROR2(ERR_CPPSDK_SET_CURLOPT, curl_easy_strerror(code));
            return ERR_CPPSDK_SET_CURLOPT;
        }
        code = curl_easy_setopt(m_curl, CURLOPT_READDATA, rp);
        if (code != CURLE_OK) {
            UFILE_SET_ERROR2(ERR_CPPSDK_SET_CURLOPT, curl_easy_strerror(code));
            return ERR_CPPSDK_SET_CURLOPT;
        }
        //必须设置,否则读取字节数会出问题
        code = curl_easy_setopt(m_curl, CURLOPT_INFILESIZE_LARGE, rp->fsize);
        if (code != CURLE_OK) {
            UFILE_SET_ERROR2(ERR_CPPSDK_SET_CURLOPT, curl_easy_strerror(code));
            return ERR_CPPSDK_SET_CURLOPT;
        }
    }

    if (hp) {
        code = curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, write_header_cb);
        if (code != CURLE_OK) {
            UFILE_SET_ERROR2(ERR_CPPSDK_SET_CURLOPT, curl_easy_strerror(code));
            return ERR_CPPSDK_SET_CURLOPT;
        }
        code = curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, hp);
        if (code != CURLE_OK) {
            UFILE_SET_ERROR2(ERR_CPPSDK_SET_CURLOPT, curl_easy_strerror(code));
            return ERR_CPPSDK_SET_CURLOPT;
        }
    }

    code = curl_easy_perform(m_curl);
    if (code != CURLE_OK) {
        UFILE_SET_ERROR2(ERR_CPPSDK_CURL_PERFORM, curl_easy_strerror(code));
        return ERR_CPPSDK_CURL_PERFORM;
    }
    return 0;
}

int HTTPRequest::ResponseCode(long *code) {

    if (!code) return -1;

    if (curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, code) != CURLE_OK) return -1;
    return 0;
}

//====================================================================================================
//

size_t read_cb(char *buffer, size_t size, size_t nitems, void *userdata) {

    UCloudHTTPReadParam *param = (UCloudHTTPReadParam *)userdata;
    if (!param) {
        //indicating no data
        return 0;
    }

    int     fd = -1;
    ssize_t nr = 0;
    size_t  total = size*nitems;
    //如果只是读取某个文件的一部分的话不会得到 EOF,CURL 就不会停止发送请求.
    if (param->need_total_n != 0 && total > param->need_total_n - param->read_total_n)
        total = param->need_total_n - param->read_total_n;

    size_t  need = total;
    while(need > 0) {
        //优先使用文件对象
        if (param->f) {
            if (fd == -1) fd = fileno(param->f);
            nr = read(fd, buffer+total-need, need);
        } else {
            try {
                nr = (ssize_t)param->is.readsome(buffer+total-need, need);
            } catch(...) {
                nr = -1;
            }
        }

        if (nr <= 0) break;
        need -= nr;
        param->read_total_n += nr;
    }
    return total-need;
}

size_t writen(FILE *f, UCloudOStream *os, const char *ptr, const size_t total) {

    int    fd = -1;
    int    ret = 0;
    size_t left = total;
    ssize_t nw = 0;
    while(left > 0) {
        //优先使用文件对象
        if (f) {
            if (fd == -1) fd = fileno(f);
            nw = write(fd, ptr+total-left, left);
        } else if (os){
            try {
                //only write once
                std::string data(ptr, total);
                ret = WriteToStream(os, data);
                if (ret) break;
                nw = total;
            } catch(...) {
                nw = -1;
            }
        }

        if (nw <= 0) return total-left;
        left -= nw;
    }
    return total-left;
}

size_t write_header_cb(void *ptr, size_t size, size_t nmemb, void *userdata) {

    UCloudHTTPHeaderParam *param = (UCloudHTTPHeaderParam *)userdata;
    if (!param) return 0;
    return writen(param->f, param->os, (char *)ptr, size*nmemb);
}

size_t write_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {

    UCloudHTTPWriteParam *param = (UCloudHTTPWriteParam *)userdata;
    if (!param) {
        return 0;
    }
    return writen(param->f, param->os, (char *)ptr, size*nmemb);
}

//====================================================================================================
int ParseHeadersFromString(const std::string &header, std::map<std::string, std::string> &headers) {

    size_t crlf_pos = -2;
    for(size_t old_pos = crlf_pos;; old_pos = crlf_pos) {
        crlf_pos = header.find("\r\n", old_pos+2);
        if (crlf_pos == std::string::npos) break;

        std::string h = header.substr(old_pos+2, crlf_pos-old_pos-2);
        if (h == "") continue;

        size_t pos = h.find(":");
        if (pos == std::string::npos) continue;
        std::string k = h.substr(0, pos);
        std::string v = h.substr(pos+1);
        ucloud::cppsdk::utils::Trim(k, " ");
        ucloud::cppsdk::utils::Trim(v, " ");
        headers[k] = v;
        old_pos = crlf_pos;
    }
    return 0;
}

int WriteToStream(UCloudOStream *os, const std::string &ptr) {

    if (!os) return -1;

    if (os->Type() == STREAM_TYPE_MULTI) {
        ((MultiWriteStream *)os)->operator<<(&ptr);
    } else if (os->Type() == STREAM_TYPE_LIMIT) {
        ((LimitWriteStream *)(os->Stream()))->operator<<(&ptr);
    } else {
        *(os->Stream()) << ptr;
    }
    return 0;
}

}
}
}
