#include "InitClient.h"

using namespace VolcengineTos;

void InitClient::InitClientWithDefaultConfig() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);
    // 释放网络等资源
    CloseClient();
    return;
}

void InitClient::InitClientWithSTS() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    std::string securityToken = "Your Security Token";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey, securityToken);

    // STS 场景下使用自定义参数可以参考如下代码
    // ClientConfig conf;
    // conf.xxx = xxx
    // TosClientV2 client(region, accessKey,secretKey, securityToken, conf);

    // 释放网络等资源
    CloseClient();
}
void InitClient::InitClientWithTimeout() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";

    // ClientConfig 是 TOSClient 的配置类，可配置 endpoint、超时时间、是否进行 CRC 数据校验等参数。
    ClientConfig conf;
    // 设置建立连接超时时间，单位：毫秒，默认 10000 毫秒。
    conf.connectionTimeout = 10000;
    // 设置一次 http 请求超时时间，单位：毫秒，默认 120000 毫秒。
    conf.requestTimeout = 120000;

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey, conf);
    // 释放网络等资源
    CloseClient();
}
void InitClient::InitClientWithRetry() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";

    // ClientConfig 是 TOSClient 的配置类，可配置 endpoint、超时时间、是否进行 CRC 数据校验等参数。
    ClientConfig conf;
    // 请求失败最大重试次数。默认 3 次。
    conf.maxRetryCount = 3;

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey, conf);
    // 释放网络等资源
    CloseClient();
}
void InitClient::InitClientWithCrc() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";

    // ClientConfig 是 TOSClient 的配置类，可配置 endpoint、超时时间、是否进行 CRC 数据校验等参数。
    ClientConfig conf;
    // 是否开启 CRC 数据校验，默认开启。
    conf.enableCRC = true;

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey, conf);
    // 释放网络等资源
    CloseClient();
}
void InitClient::InitClientWithDnsCache() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";

    // ClientConfig 是 TOSClient 的配置类，可配置 endpoint、超时时间、是否进行 CRC 数据校验等参数。
    ClientConfig conf;
    // DNS 缓存的有效期，单位：分钟，小于等于 0 时代表关闭 DNS 缓存，默认为 0。
    conf.dnsCacheTime = 3;

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey, conf);
    // 释放网络等资源
    CloseClient();
}
void InitClient::InitClientWithProxy() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";

    // ClientConfig 是 TOSClient 的配置类，可配置 endpoint、超时时间、是否进行 CRC 数据校验等参数。
    ClientConfig conf;
    // 代理服务器的主机地址，当前只支持 HTTP 协议。
    conf.proxyHost = http::SchemeHTTP;
    // 代理服务器的端口号。
    conf.proxyPort = 80;
    // 连接代理服务器时使用的用户名。
    conf.proxyUsername = "Your Proxy UserName";
    // 连接代理服务器时使用的用户密码
    conf.proxyPassword = "your Proxy Password";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey, conf);
    // 释放网络等资源
    CloseClient();
}
void InitClient::InitClientWithAllConfig() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";

    // ClientConfig 是 TOSClient 的配置类，可配置 endpoint、超时时间、是否进行 CRC 数据校验等参数。
    ClientConfig conf;
    // TOS 服务端域名，默认为空。
    conf.endPoint = "tos-cn-beijing.volces.com";
    // 自动识别 MIME 类型，默认开启。
    conf.autoRecognizeContentType = true;
    // 是否开启 CRC 数据校验，默认开启。
    conf.enableCRC = true;
    // 请求失败最大重试次数。默认 3 次。
    conf.maxRetryCount = 3;
    // 设置建立连接超时时间，单位：毫秒，默认 10000 毫秒。
    conf.connectionTimeout = 10000;
    // 设置一次 http 请求超时时间，单位：毫秒，默认 120000 毫秒。
    conf.requestTimeout = 120000;
    // 代理服务器的主机地址，当前只支持 HTTP 协议。
    conf.proxyHost = http::SchemeHTTP;
    // 代理服务器的端口号。
    conf.proxyPort = 80;
    // 连接代理服务器时使用的用户名。
    conf.proxyUsername = "Your Proxy UserName";
    // 连接代理服务器时使用的用户密码
    conf.proxyPassword = "your Proxy Password";
    conf.enableVerifySSL = true;
    // DNS 缓存的有效期，单位：分钟，小于等于 0 时代表关闭 DNS 缓存，默认为 0。
    conf.dnsCacheTime = 3;

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey, conf);
    // 释放网络等资源
    CloseClient();
}