## Release Note

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