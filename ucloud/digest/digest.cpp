//first level including
#include <ufile-cppsdk/digest.h>
//C/C++ including
#include <string.h>
#include <algorithm>
//other local project including
#include <ufile-cppsdk/config.h>
#include <ufile-cppsdk/errno.h>
#include <ufile-cppsdk/base64.h>
#include <ufile-cppsdk/hmac_sha1.h>
#include <ufile-cppsdk/http.h>
#include <ufile-cppsdk/string_util.h>

using namespace ucloud::cppsdk::error;
using namespace ucloud::cppsdk::base64;
using namespace ucloud::cppsdk::hmac;
using namespace ucloud::cppsdk::http;
using namespace ucloud::cppsdk::utils;

namespace ucloud {
namespace cppsdk {
namespace digest {

UFileDigest::UFileDigest():
    m_ak(ucloud::cppsdk::config::UCLOUD_PUBLIC_KEY)
    , m_sk(ucloud::cppsdk::config::UCLOUD_PRIVATE_KEY) {
}

UFileDigest::UFileDigest(const std::string &publicKey, const std::string &privateKey):
    m_ak(publicKey)
    , m_sk(privateKey) {
}

UFileDigest::~UFileDigest() {
}

void UFileDigest::SetKey(const std::string &publicKey, const std::string &privateKey) {

    m_ak = publicKey;
    m_sk = privateKey;
} 

//return: signature
int UFileDigest::SignWithData(const std::string &utf8String2Sign, std::string &signature) {

    if (m_ak == "" || m_sk == "") {
        UFILE_SET_ERROR(ERR_CPPSDK_MISS_AKSK);
        return ERR_CPPSDK_MISS_AKSK;
    }

    unsigned char digest[21];
    HMAC_SHA1(digest
              , (unsigned char *)m_sk.c_str()
              , m_sk.size()
              , (unsigned char *)utf8String2Sign.c_str()
              , utf8String2Sign.size()
    );
    signature = base64_encode(digest, 20, BASE64_STD);
    return 0;
}

int UFileDigest::SignWithRequest(HTTPRequest *http
    , const int type
    , const std::string &bucket
    , const std::string &key
    //如果是 multipart 上传的话需要把文件内容的真实 mimetype 传递进来
    , const std::string multipartMimetype
    , std::string &signature) {

    if (m_ak == "" || m_sk == "") {
        UFILE_SET_ERROR(ERR_CPPSDK_MISS_AKSK);
        return ERR_CPPSDK_MISS_AKSK;
    }

    std::string mimetype = http->ReqHeader("Content-Type");
    if (http->ReqHeader("multipart/form-data") != "") {
        mimetype = multipartMimetype;
    }

    std::string utf8String2Sign = http->HTTPVerb() + "\n" + \
                             http->ReqHeader("Content-MD5") + "\n" + \
                             mimetype + "\n";
    if (type == HEAD_FIELD_CHECK)
        utf8String2Sign += http->ReqHeader("Date") + "\n";
    else
        utf8String2Sign += http->Query("Expires") + "\n";

    utf8String2Sign += CanonicalizedUCloudHeaders(http);
    utf8String2Sign += CanonicalizedResource(bucket, key);
    return SignWithData(utf8String2Sign, signature);
}

std::string UFileDigest::Token(const std::string &signature) {

    return "UCloud " + m_ak + ":" + signature;
}

std::string UFileDigest::CanonicalizedResource(const std::string &bucket, const std::string &key) {

    return "/" + bucket + "/" + key;
}

std::string UFileDigest::CanonicalizedUCloudHeaders(HTTPRequest *http) {

    if (!http) return "";

    std::map<std::string, std::string> ucloudHeaders;
    std::map<std::string, std::string> headers;
    http->ReqHeaders(headers);

    std::multimap<std::string, std::string>::const_iterator it = headers.begin();
    for(; it != headers.end(); ++it) {
        std::string k = it->first;
        std::string v = it->second;
        transform(k.begin(), k.end(), k.begin(), ::tolower);
        if (memcmp(k.c_str(), "x-ucloud-", 9) != 0) continue;
        Trim(v, std::string(" ").c_str());
        ucloudHeaders[k] += v;
    }

    std::string s;
    std::map<std::string, std::string>::const_iterator it2 = ucloudHeaders.begin();
    for(; it2 != ucloudHeaders.end(); ++it2) {
        s += it2->first + ":" + it2->second + "\n";
    }
    return s;
}

}
}
}
