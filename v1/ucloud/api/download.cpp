#include "api/download.h"
#include "api/common.h"
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include "errno/errno.h"
#include "http/http.h"
#include "utils/urlcodec.h"
#include "utils/stream_util.h"
#include "digest/digest.h"
#include "config/config.h"

using namespace ucloud::cppsdk::http;
using namespace ucloud::cppsdk::error;
using namespace ucloud::cppsdk::utils;
using namespace ucloud::cppsdk::digest;

namespace ucloud {
namespace cppsdk {
namespace api    {

UFileDownload::UFileDownload():
    m_bucket("")
    , m_key("")
    , m_os(NULL) {
}

UFileDownload::~UFileDownload() {}

int UFileDownload::Download(const std::string &bucket
    , const std::string &key
    , std::ostream *os
    , const std::pair<ssize_t, ssize_t> *range) {

    int64_t ret = InitGlobalConfig();
    if (ret) return ret;

    if (!os) {
        UFILE_SET_ERROR2(ERR_CPPSDK_INVALID_PARAM, "output stream is null");
        return ERR_CPPSDK_INVALID_PARAM;
    } else {
        m_os = os;
    }

    SetResource(bucket, key);

    std::string signature("");
    //构建 HTTP 头部
    m_http->Reset();
    m_http->AddHeader("User-Agent", USERAGENT);
    m_http->SetURL(UFileHost(bucket) + EasyPathEscape(key));
    if (range) {
        m_http->SetRange(*range);
    }

    //使用 HTTP 信息构建签名
    UFileDigest digestor;
    ret = digestor.SignWithRequest(m_http
                    , HEAD_FIELD_CHECK
                    , bucket
                    , key
                    , ""
                    , signature);
    if (ret) {
        return ret;
    }
    m_http->AddHeader("Authorization", digestor.Token(signature));

    //设置数据输出
    std::stringstream error_stream;
    ucloud::cppsdk::utils::UCloudOStream data_stream(m_os);
    ucloud::cppsdk::utils::LimitWriteStream lws(&error_stream, 1024);
    ucloud::cppsdk::utils::MultiWriteStream mws;
    mws.AddStream(&lws).AddStream(&data_stream);
    UCloudHTTPWriteParam wp = {f: NULL, os: &mws};
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
    if (code != 200 && code != 206) {
        int parse_ret = UFileErrorRsp(error_stream.str().c_str(), &ret, errmsg);
        if (parse_ret) {
            UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
            return ERR_CPPSDK_CLIENT_INTERNAL;
        }
        UFILE_SET_ERROR2(ret, errmsg);
    }
    return ret;
}

int UFileDownload::DownloadAsFile(const std::string &bucket
    , const std::string &key
    , const std::string &filepath
    , const std::pair<ssize_t, ssize_t> *range) {

    int64_t ret = InitGlobalConfig();
    if (ret) return ret;

    std::ofstream ofs(filepath.c_str(), std::ofstream::out|std::ofstream::app);
    if (!ofs) {
        UFILE_SET_ERROR(ERR_CPPSDK_CLIENT_INTERNAL);
        return ERR_CPPSDK_CLIENT_INTERNAL;
    }

    ret = this->Download(bucket, key, &ofs, range);
    ofs.close();
    return ret;
}

std::string UFileDownload::DownloadURL(const std::string &bucket, const std::string &key, const size_t expires) {

    int64_t ret = InitGlobalConfig();
    if (ret) return "";

    m_http->Reset();
    m_http->AddHeader("User-Agent", USERAGENT);

    std::string signature;
    UFileDigest digestor;

    char expires_str[32];
    size_t n = snprintf(expires_str, sizeof(expires_str)-1, "%zd", expires + time(NULL));
    if (expires != 0) {
        m_http->SetQuery("Expires", std::string(expires_str, n));
    }

    ret = digestor.SignWithRequest(m_http
                    , QUERY_STRING_CHECK
                    , bucket
                    , key
                    , ""
                    , signature);
    if (ret) {
        return "";
    }

    std::string token = UFileHost(bucket) + EasyPathEscape(key) \
                + "?UCloudPublicKey=" + EasyQueryEscape(UCLOUD_PUBLIC_KEY) \
                + "&Signature=" + EasyQueryEscape(signature);
    if (expires) token += "&Expires=" + EasyQueryEscape(std::string(expires_str));
    return token;
}

void UFileDownload::SetResource(const std::string &bucket, const std::string &key) {

    m_bucket = bucket;
    m_key    = key;
}

}
}
}
