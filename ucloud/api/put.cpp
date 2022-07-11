//first level including
#include <ufile-cppsdk/put.h>
#include <ufile-cppsdk/common.h>
//C/C++including
#include <fstream>
#include <sstream>
//other local project including
#include <ufile-cppsdk/config.h>
#include <ufile-cppsdk/digest.h>
#include <ufile-cppsdk/errno.h>
#include <ufile-cppsdk/mimetype.h>
#include <ufile-cppsdk/urlcodec.h>
#include <ufile-cppsdk/string_util.h>
#include <ufile-cppsdk/stream_util.h>

using namespace ucloud::cppsdk::config;
using namespace ucloud::cppsdk::digest;
using namespace ucloud::cppsdk::error;
using namespace ucloud::cppsdk::http;
using namespace ucloud::cppsdk::utils;

namespace ucloud {
namespace cppsdk {
namespace api    {

UFilePut::UFilePut():APIBase() {} 

UFilePut::~UFilePut() {}

int UFilePut::Put(const std::string &bucket
    , const std::string &key
    , std::istream &is) {

    int64_t ret = InitGlobalConfig();
    if (ret) return ret;

    std::string mimetype;
    ret = MimeType(is, "", mimetype);
    if (ret) return ret;

    //获取数据流大小
    std::streampos fsize;
    ret = FetchContentLength(is, &fsize);
    if (ret < 0) {
        return ret;
    }

    std::string signature("");
    //构建 HTTP 头部
    m_http->Reset();
    m_http->SetVerb("PUT");
    m_http->AddHeader("Content-Type", mimetype); 
    //m_http->AddHeader("Content-Length", SIZET2STR(fsize));
    m_http->AddHeader("Expect", "");
    m_http->AddHeader("User-Agent", USERAGENT);
    m_http->SetURL(UFileHost(bucket) + EasyPathEscape(key));
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

    //设置数据源
    std::ostringstream oss;
    UCloudOStream data_stream(&oss);
    UCloudHTTPReadParam rp = {f: NULL, is: is, fsize:fsize, need_total_n: fsize};
    UCloudHTTPWriteParam wp = {f: NULL, os: &data_stream};
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

int UFilePut::Put(const std::string &bucket
    , const std::string &key
    , const char *ptr
    , const size_t size) {

    if (!ptr || size == 0) {
        UFILE_SET_ERROR(ERR_CPPSDK_INVALID_PARAM);
        return ERR_CPPSDK_INVALID_PARAM;
    }

    int64_t ret = InitGlobalConfig();
    if (ret) return ret;

    if (UCLOUD_PUBLIC_KEY == "" || UCLOUD_PRIVATE_KEY == "") {
        UFILE_SET_ERROR(ERR_CPPSDK_MISS_AKSK);
        return ERR_CPPSDK_MISS_AKSK;
    }

    std::string data(ptr, size);
    std::istringstream iss(data, std::istringstream::in);
    if (!iss) {
        UFILE_SET_ERROR(ERR_CPPSDK_FILE_READ);
        return ERR_CPPSDK_FILE_READ;
    }

    ret = this->Put(bucket, key, iss);
    return ret;
}

int UFilePut::PutFile(const std::string &bucket
    , const std::string &key
    , const std::string &filepath) {
 
    int64_t ret = InitGlobalConfig();
    if (ret) return ret;

    if (UCLOUD_PUBLIC_KEY == "" || UCLOUD_PRIVATE_KEY == "") {
        UFILE_SET_ERROR(ERR_CPPSDK_MISS_AKSK);
        return ERR_CPPSDK_MISS_AKSK;
    }

    std::ifstream ifs(filepath.c_str(), std::ifstream::in|std::ifstream::binary);
    if (!ifs) {
        UFILE_SET_ERROR(ERR_CPPSDK_FILE_READ);
        return ERR_CPPSDK_FILE_READ;
    }

    ret = this->Put(bucket, key, ifs);
    ifs.close();
    return ret;
}

}
}
}

