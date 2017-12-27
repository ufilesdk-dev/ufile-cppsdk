#ifndef _UFILE_COMMON_URL_URLCODEC_
#define _UFILE_COMMON_URL_URLCODEC_

namespace ucloud {
namespace cppsdk {
namespace utils {

int QueryUnescape(const std::string &src, std::string *dst);

int QueryEscape(const std::string &src, std::string *dst);

int PathUnescape(const std::string &src, std::string *dst);

int PathEscape(const std::string &src, std::string *dst);

std::string EasyQueryEscape(const std::string &src);
std::string EasyQueryUnescape(const std::string &src);
std::string EasyPathEscape(const std::string &src);
std::string EasyPathUnescape(const std::string &src);

}
}
}

#endif
