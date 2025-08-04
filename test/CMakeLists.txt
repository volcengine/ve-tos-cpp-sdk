cmake_minimum_required(VERSION 3.1)
project(ve-tos-cpp-sdk-test
        VERSION 2.6.17
        LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 2fe3bd994b3189899d93f1d5a881e725e046fdc2 # release-1.8.1
)

FetchContent_GetProperties(googletest)
if (NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif ()

include_directories(${CMAKE_SOURCE_DIR}/sdk/src/external)
file(GLOB test_bucket_src "src/Bucket/*")
file(GLOB test_object_src "src/Object/*")
file(GLOB test_others_src "src/Others/*")
file(GLOB test_valueadded_src "src/ValueAdded/*")
add_executable(${PROJECT_NAME} src/Utils.cc ${test_bucket_src} ${test_others_src} ${test_object_src} ${test_valueadded_src})

#target_include_directories(${PROJECT_NAME} PRIVATE gtest/include)
target_include_directories(${PROJECT_NAME}
        PRIVATE ${CMAKE_SOURCE_DIR}/sdk/include
        PRIVATE ${CMAKE_SOURCE_DIR}/sdk/src
        PRIVATE ${CMAKE_SOURCE_DIR}/sdk/src/external/spdlog
        PRIVATE ${CLIENT_SSL_INCLUDE_DIRS}
        PRIVATE ${CLIENT_CURL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}
        PRIVATE gtest_main
        PRIVATE ve-tos-cpp-sdk-lib
        PRIVATE ${CLIENT_SSL_LIBS}
        PRIVATE ${CLIENT_CURL_LIBS})

#target_compile_options(${PROJECT_NAME} PRIVATE "${SDK_COMPILER_FLAGS}")

install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
