cmake_minimum_required(VERSION 3.1)
project(ve-tos-cpp-sdk-test
        VERSION 0.2.1
        LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        e2239ee6043f73722e7aa812a459f54a28552929 # release-1.11.0
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()
add_executable(${PROJECT_NAME} src/SignV4Test.cc src/TosRequestTest.cc)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11)
#target_include_directories(${PROJECT_NAME} PRIVATE gtest/include)
target_include_directories(${PROJECT_NAME}
        PRIVATE ${CMAKE_SOURCE_DIR}/sdk/include
        PRIVATE ${CMAKE_SOURCE_DIR}/sdk/src
        PRIVATE ${CLIENT_SSL_INCLUDE_DIRS}
        PRIVATE ${CLIENT_CURL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}
        PRIVATE gtest_main
        PRIVATE ve-tos-cpp-sdk-lib
        PRIVATE ${CLIENT_SSL_LIBS}
        PRIVATE ${CLIENT_CURL_LIBS})

install(TARGETS  ${PROJECT_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
