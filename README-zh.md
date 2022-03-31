# 火山引擎 TOS C++ SDK
TOS C++ SDK为C++开发者提供了访问火山引擎对象存储服务TOS（Tinder Object Storage）的系列接口。
本文档将给出TOS桶和对象的基本操作代码，供开发者参考，具体API文档请参考[xxx]()

## 安装
### 最低依赖
- C++ 11 及以上版本
- CMake ( 3.11 及以上版本)
- Visual Studio 2013 及以上版本
- GCC 4.8 及以上版本
- Clang 3.3 及以上版本

### 通过源码编译
1. 下载源码
- 直接下载 [release](https://github.com/volcengine/ve-tos-cpp-sdk/archive/refs/tags/0.1.0.zip)
- 通过git克隆到本地
```bash
git clone https://github.com/volcengine/ve-tos-cpp-sdk
```
2. 使用 CMake 3.11 及以上版本构建
```bash
cd ve-tos-cpp-sdk
mkdir build
cd build
```

SDK 依赖 libcurl 和 openssl 两个库. 请在构建 SDK 之前确保已安装这两个库。

#### Windows
请提前安装好 libcurl 和 openssl 两个库.
在visual studio 2013 及以上环境中打开sdk，并运行以下命令进行构建
```bash
cmake ../ -DCMAKE_INSTALL_PREFIX="your path to install sdk"
msbuild ALL_BUILD.vcxproj
msbuild INSTALL.vcxproj
```

#### Linux
运行以下命令安装 libcurl 和 openssl
```bash
$ (Ubuntu/Debian)
$ sudo apt-get install libcurl4-openssl-dev libssl-dev
$ (Redhat/Fedora)
$ sudo dnf install libcurl-devel openssl-devel
```
运行以下命令进行编译及安装
```bash
cmake ../ -DCMAKE_INSTALL_PREFIX="sdk安装路径"
make
make install
```
#### Macos
在mac系统上需指定openssl库的路径。以如下命令为例
```bash
cmake .. -DOPENSSL_ROOT_DIR=/usr/local/Cellar/openssl@3/3.0.0 
         -DOPENSSL_LIBRARIES=/usr/local/Cellar/openssl@3/3.0.0/lib 
         -DOPENSSL_INCLUDE_DIRS=/usr/local/Cellar/openssl@3/3.0.0/include
         -DCMAKE_INSTALL_PREFIX="your path to install sdk"
make
make install
```

### CMake编译选项
#### BUILD_SHARED_LIB
默认关闭，cmake默认只编译静态库。打开后，cmake会同时编译静态库和动态库，同时SDK会链接到动态库。
```bash
cmake .. -DBUILD_SHARED_LIB=ON
```
#### BUILD_DEMO
默认关闭。打开后，cmake会对example文件夹中的示例进行编译，可直接运行。具体见example文件夹中示例。
```bash
cmake .. -DBUILD_DEMO=ON
```
#### BUILD_UNITTEST
默认关闭。打开后cmake会编译test文件夹中的单测用例。
```bash
cmake .. -DBUILD_UNITTEST=ON
```

## 快速入门
本节介绍，如何通过TOS C++ SDK来完成常见的操作，如创建桶，上传、下载和删除对象等。
### 初始化TOS客户端
初始化TOSClient实例之后，才可以向TOS服务发送HTTP/HTTPS请求。
初始化客户端时，需要带上accesskey，secretkey，endpoint和region。初始化代码如下：

```cpp
using namespace VolcengineTos;
std::string endpoint("your endpoint");
std::string region("your region");
std::string ak("Your Access Key");
std::string sk("Your Secret Key");

// 初始化SDK
InitializeClient();
// 创建 client 用于与 TOS 服务进行交互
TosClient client(endpoint, region, ak, sk);
/*
使用client进行操作
*/
// 关闭client
CloseClient();
```

### 创建桶
桶是TOS的全局唯一的命名空间，相当于数据的容器，用来储存对象数据。如下代码展示如何创建一个新桶：

```cpp
std::string bucket("your bucket name");
CreateBucketInput input;
input.setBucket(bktName);
auto output = client.createBucket(input);
if (!output.isSuccess()){
  std::cout << output.error().String() << std::endl;
  return;
}
std::cout << output.result().getLocation() << std::endl;
```

### 上传对象
新建桶成功后，可以往桶中上传对象，如下展示如何上传一个对象到已创建的桶中：

```cpp
// 需要上传的对象数据，以stringstream的形式上传
std::string bucket("your bucket name");
std::string data("1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'");
auto ss = std::make_shared<std::stringstream>(data);
std::string key("object-put-1");
auto output = client.putObject(bucket, key, ss);
if (!output.isSuccess()) {
  std::cout << "上传对象失败: "
            << output.error().String()
            << std::endl;
  return;
}
std::cout << "上传对象成功, 对象 etag 是: "
          << output.result().getEtag()
          << std::endl;
```

### 下载对象
如下展示如何从桶中下载一个已经存在的对象：

```cpp
std::string key("object-put-1");
std::string bucket("your bucket");
auto output = client.getObject(bucket, key);
if (!output.isSuccess()){
  std::cout << output.error().String() << std::endl;
  return;
}
std::cout << output.result().getObjectMeta().getEtags() << std::endl;
```

### 删除对象
如下展示如何从桶中删除一个已经存在的对象：

```cpp
std::string key("object-put-1");
std::string bucket("your bucket");
auto output = client.deleteObject(bucket, key);
if (!output.isSuccess()){
  std::cout << output.error().String() << std::endl;
  return;
}
std::cout << output.result().getRequestInfo().getRequestId() << std::endl;
```

## License
[Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0.html)
