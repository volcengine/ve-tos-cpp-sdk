#include "executor/TosClientTemplate.h"


#include "model/async/object/output/GetObjectAsyncOutput.h"
#include "model/object/GetObjectV2Output.h"
#include <cstring>
#include <sys/mman.h>

using namespace VolcengineTos;
namespace VolcengineTos {
bool findInCanRetryCurlErr(int curlErrCode) {
    switch (curlErrCode) {
        case (7):   // CURLE_COULDNT_CONNECT
        case (18):  // CURLE_PARTIAL_FILE
        case (23):  // CURLE_WRITE_ERROR
        case (28):  // CURLE_OPERATION_TIMEDOUT
        case (52):  // CURLE_GOT_NOTHING
        case (55):  // CURLE_SEND_ERROR
        case (56):  // CURLE_RECV_ERROR
        case (65):  // CURLE_SEND_FAIL_REWIND
            return true;
        default:
            return false;
    }
}

std::set<std::string> CanRetryMethods = {"createBucketAsync",
                                         "deleteBucketAsync",
                                         "createMultipartUploadAsync",
                                         "completeMultipartUploadAsync",
                                         "abortMultipartUploadAsync",
                                         "setObjectMetaAsync",
                                         "putObjectAclAsync",
                                         "deleteObjectAsync",
                                         "putObjectAsync",
                                         "putObjectFromFileAsync",
                                         "getObjectAsync",
                                         "getObjectToFileAsync",
                                         "uploadPartAsync"};

bool findInCanRetryMethods(const std::string& method) {
    auto pos = CanRetryMethods.find(method);
    if (pos != CanRetryMethods.end()) {
        return true;
    }
    return false;
}

bool checkShouldRetry(const std::string& funcName, const int resCode, const int curlErrCode, const int flowBytes) {
    const bool curlErrShouldRetry = (curlErrCode != 0) && findInCanRetryCurlErr(curlErrCode);

    if (resCode != 429 && resCode < 500 && !curlErrShouldRetry) {
        // 三项不满足任何，不重试
        return false;
    }

    // head操作直接可以重试
    if (StringUtils::startsWithIgnoreCase(funcName, "head")) {
        return true;
    }

    // 裸下载返回了数据不重试
    if (funcName == "getObjectAsync" && flowBytes > 0) {
        return false;
    }

    // 其他情况下看重试列表是否能重试
    if (findInCanRetryMethods(funcName)) {
        return true;
    }

    return false;
}
static size_t get_system_page_size() {
    static size_t page_size = sysconf(_SC_PAGESIZE);
    // 异常情况：sysconf获取失败时，默认用4KB兜底
    if (page_size == static_cast<size_t>(-1)) {
        Logger::getInstance().warn("get_system_page_size: sysconf failed, use default 4096");
        page_size = 4096;
    }
    return page_size;
}

size_t align_to_page(const size_t size) {
    const size_t page_size = get_system_page_size();
    return (size + page_size - 1) & ~(page_size - 1);
}

// 扩展mmap映射（确保能容纳新增数据）
int extendMmap(const int file_fd, void*& mmap_ptr, size_t& mmap_size,  // 引用传递！
               const size_t file_offset, const size_t need_add_size) {
    // 检查文件描述符有效性
    if (file_fd == -1) {
        Logger::getInstance().error("extendMmap: invalid file fd");
        return -1;
    }

    // 1. 计算需要的总大小和对齐后的文件大小
    const size_t new_needed_size = file_offset + need_add_size;
    const size_t page_size = get_system_page_size();
    // 对齐到系统页大小，且确保至少为1个页（避免new_file_size=0）
    size_t new_file_size = ((new_needed_size + page_size - 1) & ~(page_size - 1));
    if (new_file_size == 0) {
        new_file_size = page_size;
    }

    // 若当前映射已足够，无需扩展
    if (new_file_size <= mmap_size) {
        Logger::getInstance().debug("extendMmap: current size ", mmap_size, " is enough, no need to extend");
        return 0;
    }

    Logger::getInstance().debug("extendMmap: need new size ", new_needed_size, ", aligned to ", new_file_size,
                                " (page size: ", page_size, ")");

    // 2. 扩展文件大小（修正off_t类型，避免截断）
    if (ftruncate(file_fd, static_cast<off_t>(new_file_size)) == -1) {
        Logger::getInstance().error("extendMmap: ftruncate failed, errno: ", errno, ", err: ", strerror(errno));
        return -1;
    }

    // 3. 解除旧映射（若存在）
    if (mmap_ptr != MAP_FAILED && mmap_ptr != nullptr) {
        if (munmap(mmap_ptr, mmap_size) == -1) {
            Logger::getInstance().error("extendMmap: munmap old failed, errno: ", errno, ", err: ", strerror(errno));
            return -1;
        }
        mmap_ptr = nullptr;  // 解除后置空，避免野指针
    }

    // 4. 重新映射新大小（MAP_SHARED确保写入同步到文件）
    mmap_ptr = mmap(nullptr,        // 内核自动分配映射地址
                    new_file_size,  // 对齐后的新映射长度
                    PROT_WRITE,     // 可写权限
                    MAP_SHARED,     // 共享映射（关键：写入同步到文件）
                    file_fd,        // 文件描述符
                    0               // 从文件起始位置映射（必须对齐页大小）
    );

    if (mmap_ptr == MAP_FAILED) {
        Logger::getInstance().error("extendMmap: mmap new failed, errno: ", errno, ", err: ", strerror(errno));
        return -1;
    }

    // 5. 同步更新外部变量（引用传递生效）
    mmap_size = new_file_size;
    Logger::getInstance().debug("extendMmap: extend success, new mmap size: ", mmap_size);
    return 0;
}

std::string getRequestID(const std::map<std::string, std::string>& headers) {
    return MapUtils::findValueByKeyIgnoreCase(headers, HEADER_REQUEST_ID);
}

std::string getEcCode(const std::map<std::string, std::string>& headers) {
    return MapUtils::findValueByKeyIgnoreCase(headers, HEADER_EC_CODE);
}

std::string getRequestId2(const std::map<std::string, std::string>& headers) {
    return MapUtils::findValueByKeyIgnoreCase(headers, HEADER_ID_2);
}

}  // namespace VolcengineTos