本源码包含使用 C++ SDK 对 UCloud 的对象存储业务US3 (原名UFile) 进行上传/下载等文件操作,
不包含对 Bucket 的管理操作,如有任何疑问请咨询 UCloud 技术支持.  

# 目录结构
* demo 目录:  
    该目录包含如何使用本 SDK 的演示程序.  

* ucloud 目录:  
    该目录是 SDK 具体实现.  

# 安装

## 前提条件
* C++ 11及以上版本的编译器  
* cmake 3.10及以上版本

## 安装第三方依赖

### 安装libtcmalloc
```
  yum install gperftools
```

### 安装libmagic

#### For Debian-based systems:
```
  apt-get install libmagic-dev
```

#### For CentOS 6.x or similar RPM-based systems:
```
  yum install file-devel
```

#### For CentOS 5.x or similar RPM-based systems:
```
  yum install file
```

### 安装libcurl
```
  yum install libcurl-devel
```

### 安装libjson-c
```
  git clone https://github.com/json-c/json-c.git
  mkdir json-c-build
  cd json-c-build
  cmake ../json-c
  make
  make install
```

## sdk下载安装

- 通过[GitHub](https://github.com/ufilesdk-dev/ufile-cppsdk)下载
```
  git clone https://github.com/ufilesdk-dev/ufile-cppsdk.git
```

- 编译安装
```
  cd ufile-cppsdk
  mkdir build
  cd build
  cmake ..
  make
  make install
  git clone https://github.com/ufilesdk-dev/ufile-cppsdk.git
```

- 查阅演示程序
```
  cd demo
  ./demo
```

# 使用示例

使用本 SDK 时请配置相关配置文件, 配置文件的路径为 /etc/ufilesdk.conf 或者 ./ufilesdk.conf  

具体配置格式请参考 代码目录中的ucloud/include/ufile-cppsdk/config.h文件的描述  

- 配置文件示例:
```
{
  "public_key": "your public key",
  "private_key": "your private key",
  "proxy_host": ".cn-bj.ufileos.com"
}
```

> 密钥可以在控制台中 [API 产品 - API 密钥](https://console.ucloud.cn/uapi/apikey)，点击显示 API 密钥获取。将 public_key 和 private_key 分别赋值给相关变量后，SDK即可通过此密钥完成鉴权。请妥善保管好 API 密钥，避免泄露。 token（令牌）是针对指定bucket授权的一对公私钥。可通过token进行授权bucket的权限控制和管理。可以在控制台中[对象存储US3-令牌管理](https://console.ucloud.cn/ufile/token)，点击创建令牌获取。

> file_host 是文件操作请求的接收地址，可以在[控制台](https://console.ucloud.cn/ufile/ufile)的存储空间域名一列获取，例如`test-demo.cn-bj.ufileos.com`，填写`.cn-bj.ufileos.com`即可(注意要以.开头)，不需要存储空间名称。  

## 示例代码如下

```
#include <iostream>
#include <ufile-cppsdk/api.h>

#define BUCKET ("mytest")
#define KEY ("gg")
#define FILE ("/etc/ufilesdk.conf")


int main() {
  ucloud::cppsdk::api::UFilePut uploader;
  int ret = uploader.PutFile(BUCKET, KEY, FILE);
  if (ret) {
    std::cerr << "putfile error: retcode=" << UFILE_LAST_RETCODE() \
              << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
    return ret;
  }

  std::cout << "put file success" << std::endl;
  return 0;
}
```

> 上述代码实现了一个简单的上传逻辑，将文件/etc/ufilesdk.conf以"gg"作为key上传到存储空间(Bucket)mytest中

编译示例代码:  
```
  g++ mydemo.cc -o mydemo -lufilecppsdk -lcurl -lmagic -ljson-c
```
