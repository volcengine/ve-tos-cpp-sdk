cmake_minimum_required(VERSION 3.11)

project(ve-tos-cpp-sdk
        VERSION 0.1.0
        DESCRIPTION "Volcengine TOS cpp SDK"
        LANGUAGES CXX)

# project settings
set(CMAKE_CXX_STANDARD 11)

# options
option(BUILD_UNITTEST "Build unittest" OFF)
option(BUILD_DEMO "Build demo" OFF)
option(BUILD_SHARED_LIB "Build shared library" OFF)

# check OS platform
if (UNIX)
    set(OS_PLATFORM "LINUX")
    set(LINUX_PLATFORM 1)
elseif (APPLE)
    set(OS_PLATFORM "APPLE")
    set(APPLE_PLATFORM 1)
elseif (WIN32)
    set(OS_PLATFORM "WINDOWS")
    set(WINDOWS_PLATFORM 1)
else ()
    message(FATAL_ERROR "unkonwn os platform")
endif ()
message(STATUS "OS platform: ${OS_PLATFORM}")

message(STATUS "Looking for OpenSSL")
find_package(OpenSSL REQUIRED)
if(NOT OPENSSL_FOUND)
    message(FATAL_ERROR "Could not find openSSL")
endif()
message(STATUS "OpenSSL include: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL lib: ${OPENSSL_LIBRARIES}")
set(CLIENT_SSL_LIBS ${OPENSSL_LIBRARIES})
set(CLIENT_SSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR})

message(STATUS "Looking for CURL")
find_package(CURL)
if(NOT CURL_FOUND)
    message(FATAL_ERROR "Could not find curl")
endif()
message(STATUS "CURL include: ${CURL_INCLUDE_DIRS}")
message(STATUS "CURL lib: ${CURL_LIBRARIES}")
set(CLIENT_CURL_LIBS ${CURL_LIBRARIES})
set(CLIENT_CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIRS})

add_subdirectory(sdk)

if (BUILD_DEMO)
    add_subdirectory(example)
endif()

if (BUILD_UNITTEST)
    add_subdirectory(test)
endif ()
