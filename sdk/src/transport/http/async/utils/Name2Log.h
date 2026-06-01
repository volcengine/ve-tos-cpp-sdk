#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <thread>

namespace VolcengineTos {

// 辅助函数：将指针转换为字符串（用于日志中的句柄标识）
// inline 关键字避免头文件函数多重定义问题
inline std::string ptrToString(const void* ptr) {
    std::stringstream ss;
    ss << "0x" << std::hex << std::setw(16) << std::setfill('0') << reinterpret_cast<uintptr_t>(ptr);
    return ss.str();
}

// 平台宏判断（根据编译环境自动识别）
#if defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
// Windows老版本兼容（Win7/8）
#pragma warning(disable : 4996)
#endif

/**
 * 跨平台设置线程名称
 * @param thread 要命名的std::thread对象（创建后未join/detach）
 * @param name 线程名称（Linux限制15字符，Windows无限制）
 */
inline void set_thread_name(std::thread& thread, const std::string& name) {
#if defined(__linux__) || defined(__APPLE__)
    // Linux/macOS：通过pthread设置
    if (thread.native_handle() != 0) {
        // 截断为15字符（pthread限制）
        char short_name[16] = {0};
        strncpy(short_name, name.c_str(), 15);
        pthread_setname_np(thread.native_handle(), short_name);
    }
#elif defined(_WIN32) || defined(_WIN64)
    // Windows Win10+：SetThreadDescription（推荐）
    if (thread.native_handle() != 0) {
        HANDLE hThread = static_cast<HANDLE>(thread.native_handle());
        // 转换为宽字符
        std::wstring wname(name.begin(), name.end());
        SetThreadDescription(hThread, wname.c_str());

        // 兼容Win7/8（可选）：SetThreadName（通过异常机制实现）
        typedef struct tagTHREADNAME_INFO {
            DWORD dwType;      // 必须为0x1000
            LPCSTR szName;     // 线程名（C字符串）
            DWORD dwThreadID;  // 线程ID（0=当前线程）
            DWORD dwFlags;     // 必须为0
        } THREADNAME_INFO;
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = name.c_str();
        info.dwThreadID = GetThreadId(hThread);
        info.dwFlags = 0;
        __try {
            RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (DWORD_PTR*)&info);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }
#endif
}

/**
 * 在线程函数内部设置当前线程名称（推荐，无竞态）
 * @param name 线程名称
 */
inline void set_current_thread_name(const std::string& name) {
#if defined(__linux__) || defined(__APPLE__)
    char short_name[16] = {0};
    strncpy(short_name, name.c_str(), 15);
    pthread_setname_np(pthread_self(), short_name);
#elif defined(_WIN32) || defined(_WIN64)
    // 当前线程直接用GetCurrentThread()
    HANDLE hThread = GetCurrentThread();
    std::wstring wname(name.begin(), name.end());
    SetThreadDescription(hThread, wname.c_str());

    // 兼容Win7/8
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name.c_str();
    info.dwThreadID = GetCurrentThreadId();
    info.dwFlags = 0;
    __try {
        RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (DWORD_PTR*)&info);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
#endif
}

}  // namespace VolcengineTos