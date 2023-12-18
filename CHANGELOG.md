## Release Note

### 2023.11.28 Version 2.6.5

- 新增：初始化的 Region 新增柔佛 ap-southeast-1
- 新增：支持配置打印高延迟日志
- 新增：异常信息中新增 EC 详细错误码
- 改变：重试策略适配服务端 Retry-After 机制
- 修复：预签名 URL 使用 AlternativeEndpoint 参数和预期行为不一致的问题

### 2023.11.21 Version 2.6.4

- 修复：windows 指定 unicode 时编码问题

### 2023.10.30 Version 2.6.3

- 修复：query 未进行 uri 编码的问题

### 2023.9.19 Version 2.6.2

- 新增：配置中新增 caPath 和 caFile 用于传递证书路径，方便 Windows 系统通过路径传递证书
- 新增：错误处理中新增 CurlErrCode 透传 libcurl 错误码
- 修复：少量 Windows 下编译报错

### 2023.8.9 Version 2.6.1

- 修复：Windows 编译动态库时的编译脚本错误

### 2023.7.10 Version 2.6.0

- 新增：支持单连接限速
- 新增：StorageClass 支持智能分层类型、冷归档
- 新增：CompleteMultipartUpload 接口支持 completeAll
- 新增：GetObject 支持设置图片转码参数
- 新增：支持使用自定义域名，初始化参数新增 isCustomDomain
- 新增：支持上传回调参数
- 新增：支持镜像回源参数增强
- 新增：支持重命名单个对象
- 新增：支持取回冷归档对象
- 新增：事件通知增加 MQ 类型

### 2023.7.4 Version 2.5.9

- 修复：增加针对 LibCurl 几种错误码的重试

### 2023.6.30 Version 2.5.8

- 修复：HTTP 客户端多线程中析构多次调用

### 2023.6.17 Version 2.5.7

- 新增：Windows 系统兼容性
- 修复：自定义元数据中文编码问题
- 修复：网络等错误时，客户端报错描述

### 2023.3.12 Version 2.5.6

- 修改：放开参数校验限制

### 2023.3.10 Version 2.5.5

- 修复：删除多余的文件

### 2023.3.5 Version 2.5.4

- 新增：连接池实现
- 修复：重试机制

### 2023.3.2 Version 2.5.3

- 修复：优化可移植性

### 2023.2.23 Version 2.5.2

- 修复：当前对对象本身没有 crc64 的场景，Crc64 会校验不通过

### 2023.2.23 Version 2.5.1

- 修复：ListObjectsType2 将会默认返回owner信息
- 修复：修复 ListMultipartUploads,ListParts,deleteMultiObjects,listUploadedParts 解 Json 的强校验
- 修复：对 Header 中参数进行格式转换时，会判断是否有该 Header，而不是现有的直接转换逻辑
- 修复：uploadFile 和 resumableCopyObject 在调用 cancel isAbortFunc 时，没有正确清理 checkpoint 文件

### 2022.11.16 Version 2.5.0

- 新增：桶跨区域复制相关接口
- 新增：桶多版本相关接口
- 新增：桶配置静态网站相关接口
- 新增：桶事件通知相关接口
- 新增：自定义域名相关接口
- 新增：断点续传复制接口
- 新增：目录分享签名接口
- 修复：uploadFile不支持空文件

### 2022.11.16 Version 2.4.2

- 修复: ClientV2 增加对 ClientV1 接口兼容性

### 2022.11.10 Version 2.4.1

- 修复: GCC 4.8.5 兼容性问题

### 2022.10.31 Version 2.4.0

- 新增：客户端 DNS 缓存功能
- 新增：Proxy 功能
- 新增：ListObjectsType2 接口
- 新增：桶生命周期相关接口
- 新增：桶策略相关接口
- 新增：桶存储类型相关接口
- 新增：桶 CORS 相关接口
- 新增：桶镜像回源相关接口
- 新增：桶 ACL 相关接口
- 新增：对象标签相关接口
- 新增：fetch 相关接口
- 修改：统一错误定义
- 修复：下载文件缺陷

### 2022.10.7 Version 2.2.0

- 新增：ClientV2
- 新增：断点续传 DownloadFile 接口
- 新增：上传/下载可支持进度条
- 新增：客户端断流校验功能
- 新增：上传/下载客户端限速功能
- 新增：客户端 CRC 校验功能
- 新增：引入请求重试

### 2022.5.30 Version 0.2.1

- 支持 federation token

### 2022.5.9 Version 0.2.0

- 修复预签名 url 中的符号函数

### 2022.4.9 Version 0.1.1

- 修复 signer 中的并发错误
- 删除头中的 json 依赖项

### 2022.4.1 Version 0.1.0

- 第一版 C++ SDK