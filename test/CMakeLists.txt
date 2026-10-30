cmake_minimum_required(VERSION 3.1...3.10)
project(ve-tos-cpp-sdk-test
        VERSION 2.6.31
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
    # GoogleTest 1.8.1 still requests policies removed by CMake 4.
    if (NOT CMAKE_VERSION VERSION_LESS "4.0" AND
            (NOT DEFINED CMAKE_POLICY_VERSION_MINIMUM OR CMAKE_POLICY_VERSION_MINIMUM VERSION_LESS "3.5"))
        set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    endif ()
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif ()

include_directories(${CMAKE_SOURCE_DIR}/sdk/src/external)
file(GLOB test_bucket_src "src/Bucket/*")
file(GLOB test_object_src "src/Object/*")
file(GLOB test_others_src "src/Others/*")
file(GLOB test_valueadded_src "src/ValueAdded/*")
add_executable(${PROJECT_NAME} src/Utils.cc ${test_bucket_src} ${test_others_src} ${test_object_src} ${test_valueadded_src})
if (BUILD_ASYNC_SDK)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VOLCENGINE_TOS_ASYNC_TEST=1)
endif ()

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

add_executable(sync-error-propagation-test SyncErrorPropagationTest.cc)
target_include_directories(sync-error-propagation-test PRIVATE
        ${CMAKE_SOURCE_DIR}/sdk/include ${CMAKE_SOURCE_DIR}/sdk/src
        ${CMAKE_SOURCE_DIR}/sdk/src/external/spdlog
        ${CLIENT_SSL_INCLUDE_DIRS} ${CLIENT_CURL_INCLUDE_DIRS})
target_link_libraries(sync-error-propagation-test PRIVATE gtest_main
        ve-tos-cpp-sdk-lib ${CLIENT_SSL_LIBS} ${CLIENT_CURL_LIBS})
add_test(NAME sync-error-propagation COMMAND sync-error-propagation-test)
set_tests_properties(sync-error-propagation PROPERTIES TIMEOUT 90 LABELS "sync;regression")

add_executable(tos-response-header-test TosResponseHeaderTest.cc)
target_include_directories(tos-response-header-test PRIVATE ${CMAKE_SOURCE_DIR}/sdk/include)
add_test(NAME tos-response-header COMMAND tos-response-header-test)
set_tests_properties(tos-response-header PROPERTIES TIMEOUT 10 LABELS "sync;regression;headers")

# This target includes HttpClient.cc to test its private callback. The static
# archive supplies other SDK objects without linking HttpClient.cc a second time.
add_executable(upload-read-callback-test UploadReadCallbackTest.cc)
target_include_directories(upload-read-callback-test PRIVATE
        ${CMAKE_SOURCE_DIR}/sdk/include ${CMAKE_SOURCE_DIR}/sdk/src
        ${CMAKE_SOURCE_DIR}/sdk/src/external/spdlog
        ${CLIENT_SSL_INCLUDE_DIRS} ${CLIENT_CURL_INCLUDE_DIRS})
target_link_libraries(upload-read-callback-test PRIVATE gtest_main
        ve-tos-cpp-sdk-lib ${CLIENT_SSL_LIBS} ${CLIENT_CURL_LIBS})

install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
