cmake_minimum_required(VERSION 3.1)

project(ve-tos-cpp-sdk
        VERSION 2.6.26
        LANGUAGES CXX)

# project settings
if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif ()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# options
option(BUILD_UNITTEST "Build unittest" OFF)
option(BUILD_DEMO "Build demo" OFF)
option(BUILD_SHARED_LIB "Build shared library" OFF)
option(BUILD_ASYNC_SDK "Build async SDK sources and public async APIs" OFF)
set(BUILD_ASYNC_SDK_IO_URING "AUTO" CACHE STRING
        "Build async SDK io_uring file transfer support: AUTO, ON, OFF")
set_property(CACHE BUILD_ASYNC_SDK_IO_URING PROPERTY STRINGS AUTO ON OFF)
# close warning
add_definitions(-w)
# check OS platform
if (APPLE)
    set(OS_PLATFORM "APPLE")
    set(APPLE_PLATFORM 1)
elseif (UNIX)
    set(OS_PLATFORM "LINUX")
    set(LINUX_PLATFORM 1)
elseif (WIN32)
    set(OS_PLATFORM "WINDOWS")
    set(WINDOWS_PLATFORM 1)
else ()
    message(FATAL_ERROR "unkonwn os platform")
endif ()

message(STATUS "OS_PLATFORM: ${OS_PLATFORM}")
message(STATUS "BUILD_ASYNC_SDK: ${BUILD_ASYNC_SDK}")
message(STATUS "BUILD_ASYNC_SDK_IO_URING: ${BUILD_ASYNC_SDK_IO_URING}")
if (BUILD_ASYNC_SDK AND NOT "${OS_PLATFORM}" STREQUAL "LINUX")
    message(FATAL_ERROR "BUILD_ASYNC_SDK is currently supported only on Linux")
endif ()
add_definitions(-DPLATFORM_${OS_PLATFORM})
if (${OS_PLATFORM} STREQUAL "WINDOWS")
    set(WLIB_TARGET "Win32")
    if (CMAKE_CL_64)
        set(WLIB_TARGET "x64")
    endif()
    set(CLIENT_SSL_LIBS
            ${CMAKE_SOURCE_DIR}/third_party/lib/${WLIB_TARGET}/ssleay32.lib
            ${CMAKE_SOURCE_DIR}/third_party/lib/${WLIB_TARGET}/libeay32.lib)
    set(CLIENT_SSL_INCLUDE_DIRS
            ${CMAKE_SOURCE_DIR}/third_party/include)

    set(CLIENT_CURL_LIBS
            ${CMAKE_SOURCE_DIR}/third_party/lib/${WLIB_TARGET}/libcurl.lib)
    set(CLIENT_CURL_INCLUDE_DIRS
            ${CMAKE_SOURCE_DIR}/third_party/include)

else ()
    message(STATUS "Looking for OpenSSL")
    find_package(OpenSSL REQUIRED)
    if (NOT OPENSSL_FOUND)
        message(FATAL_ERROR "Could not find openSSL")
    endif ()
    message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL include: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL lib: ${OPENSSL_LIBRARIES}")
    set(CLIENT_SSL_LIBS ${OPENSSL_LIBRARIES})
    set(CLIENT_SSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR})

    message(STATUS "Looking for CURL")
    find_package(CURL)
    if (NOT CURL_FOUND)
        message(FATAL_ERROR "Could not find curl")
    endif ()
    message(STATUS "CURL version: ${CURL_VERSION_STRING}")
    message(STATUS "CURL include: ${CURL_INCLUDE_DIRS}")
    message(STATUS "CURL lib: ${CURL_LIBRARIES}")
    set(CLIENT_CURL_LIBS ${CURL_LIBRARIES})
    set(CLIENT_CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIRS})
endif ()

set(VOLCENGINE_TOS_ASYNC_HAS_IO_URING 0)
set(ASYNC_IO_URING_LIBS "")
if (BUILD_ASYNC_SDK)
    string(TOUPPER "${BUILD_ASYNC_SDK_IO_URING}" ASYNC_IO_URING_MODE)
    if (NOT ASYNC_IO_URING_MODE STREQUAL "AUTO" AND
            NOT ASYNC_IO_URING_MODE STREQUAL "ON" AND
            NOT ASYNC_IO_URING_MODE STREQUAL "OFF")
        message(FATAL_ERROR "BUILD_ASYNC_SDK_IO_URING must be AUTO, ON, or OFF")
    endif ()

    if (ASYNC_IO_URING_MODE STREQUAL "OFF")
        message(STATUS "Async SDK io_uring support: OFF")
    else ()
        include(CheckIncludeFileCXX)
        check_include_file_cxx("liburing.h" HAVE_LIBURING_H)
        find_library(LIBURING_LIBRARY NAMES uring)
        if (HAVE_LIBURING_H AND LIBURING_LIBRARY)
            set(VOLCENGINE_TOS_ASYNC_HAS_IO_URING 1)
            set(ASYNC_IO_URING_LIBS ${LIBURING_LIBRARY})
            message(STATUS "Async SDK io_uring support: ON (${LIBURING_LIBRARY})")
        elseif (ASYNC_IO_URING_MODE STREQUAL "ON")
            message(FATAL_ERROR "BUILD_ASYNC_SDK_IO_URING=ON requires liburing.h and liburing")
        else ()
            message(STATUS "Async SDK io_uring support: OFF (liburing not found)")
        endif ()
    endif ()
endif ()

list(APPEND SDK_COMPILER_FLAGS "-std=c++11")
if (WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif ()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    list(APPEND SDK_COMPILER_FLAGS "/MP" "/bigobj")
    if (NOT ENABLE_RTTI)
        list(APPEND SDK_COMPILER_FLAGS "/GR-")
    endif ()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND SDK_COMPILER_FLAGS "-fno-exceptions" "-fPIC" "-fno-rtti")
    if (NOT ENABLE_RTTI)
        list(APPEND SDK_COMPILER_FLAGS "-fno-rtti")
    endif ()

    list(APPEND SDK_COMPILER_FLAGS "-Wall" "-Werror" "-pedantic" "-Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread")
else ()
    list(APPEND SDK_COMPILER_FLAGS "-fno-exceptions" "-fPIC")
    if (NOT ENABLE_RTTI)
        list(APPEND SDK_COMPILER_FLAGS "-fno-rtti")
    endif ()

    list(APPEND SDK_COMPILER_FLAGS "-Wall" "-Werror" "-pedantic" "-Wextra")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread")
endif ()

MESSAGE(${CLIENT_CURL_LIBS})
MESSAGE(${CLIENT_CURL_INCLUDE_DIRS})
MESSAGE(${CLIENT_SSL_LIBS})
MESSAGE(${CLIENT_SSL_INCLUDE_DIRS})


add_subdirectory(sdk)

if (BUILD_DEMO)
    add_subdirectory(example)
endif ()

if (BUILD_UNITTEST)
    add_subdirectory(test)
endif ()
