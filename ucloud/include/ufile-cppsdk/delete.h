#ifndef _H_UCLOUDSDK_CPP_UCLOUD_API_DELETE_
#define _H_UCLOUDSDK_CPP_UCLOUD_API_DELETE_

#include <ufile-cppsdk/apibase.h>
#include <string>

namespace ucloud {
namespace cppsdk {
namespace api    {

class UFileDelete: public APIBase {

public:
    UFileDelete();
    ~UFileDelete();

    /*
     * @brief: 删除文件
     * @bucket: 目标Bucket名称
     * @key: 要删除的文件对象名
     * @return: 0=成功，非0=失败
     */
    int Delete(const std::string &bucket, const std::string &key);
};

}
}
}

#endif
