#ifndef _UFILESDK_CPP_UCLOUD_CONFIG_CONFIG_
#define _UFILESDK_CPP_UCLOUD_CONFIG_CONFIG_

#include <string>

namespace ucloud {
namespace cppsdk {
namespace config {

//请在/etc/ufilesdk.conf 或者运行程序的目录下./ufilesdk.conf 配置下列参数.
//参数配置使用 json 格式,模板如下
/*
{
    "public_key": "请把您账户的 API 公钥粘贴于此",
    "private_key": "请把您账户的 API 私钥粘贴于此",
    //非北京地域的配置请参考官网文档列出的各个区域的域名
    "proxy_host": ".ufile.ucloud.cn"
}
*/

//用户公钥
extern std::string UCLOUD_PUBLIC_KEY;
//用户私钥
extern std::string UCLOUD_PRIVATE_KEY;

//上传域名的后缀, 外网上传默认为 ".ufile.ucloud.cn" ,不需要改动
//内网上传需要使用各 region 对应的域名.
extern std::string UCLOUD_HOST_SUFFIX;

int InitGlobalConfig();

#define USERAGENT ("UFile C++SDK/1.1.2")

/*
 * 动态修改配置
 * public_key: 您账户的 API 公钥
 * private_key: 您账户的 API 私钥
 * proxy_host: 区域的域名
 */
void UFileSetConfig(std::string public_key, std::string private_key,
                    std::string proxy_host);

} // namespace config
} // namespace cppsdk
} // namespace ucloud

#endif
