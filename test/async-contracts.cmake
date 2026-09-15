# Independent of BUILD_UNITTEST: offline contracts need no gtest download or credentials.
# Real-bucket smoke is a separate opt-in executable and is never registered with CTest.
find_package(Threads REQUIRED)
if (BUILD_SHARED_LIB)
    set(ASYNC_CONTRACT_SDK_TARGET ve-tos-cpp-sdk-lib-static)
else ()
    set(ASYNC_CONTRACT_SDK_TARGET ve-tos-cpp-sdk-lib)
endif ()
if (BUILD_ASYNC_CONTRACT_TESTS)
foreach(contract AsyncPipelineOnce AsyncLegacyReadHttp AsyncPipelineSetup AsyncJsonLimits AsyncReceiveException
        AsyncQueueLogger AsyncTransportSafety AsyncRetryPolicy AsyncScheduling)
    add_executable(${contract}Test ${CMAKE_CURRENT_LIST_DIR}/${contract}Test.cc)
    set_target_properties(${contract}Test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
    target_include_directories(${contract}Test SYSTEM PRIVATE
            ${CMAKE_SOURCE_DIR}/sdk/include ${CMAKE_SOURCE_DIR}/sdk/src
            ${CMAKE_SOURCE_DIR}/sdk/src/external ${CLIENT_SSL_INCLUDE_DIRS} ${CLIENT_CURL_INCLUDE_DIRS})
    target_link_libraries(${contract}Test PRIVATE ${ASYNC_CONTRACT_SDK_TARGET}
            ${CLIENT_CURL_LIBS} ${CLIENT_SSL_LIBS} Threads::Threads)
    target_compile_options(${contract}Test PRIVATE -Wall -Wextra -Werror)
    add_test(NAME ${contract} COMMAND ${contract}Test)
    set_tests_properties(${contract} PROPERTIES TIMEOUT 60 LABELS "async-contract;offline")
endforeach()
add_executable(AsyncSharedEngineTest ${CMAKE_CURRENT_LIST_DIR}/AsyncSharedEngineTest.cc)
add_executable(AsyncCachedQueueTest ${CMAKE_CURRENT_LIST_DIR}/AsyncCachedQueueTest.cc)
target_include_directories(AsyncCachedQueueTest PRIVATE ${CMAKE_SOURCE_DIR}/sdk/include)
target_compile_options(AsyncCachedQueueTest PRIVATE -Wall -Wextra -Werror)
add_test(NAME AsyncCachedQueue COMMAND AsyncCachedQueueTest)
set_tests_properties(AsyncCachedQueue PROPERTIES TIMEOUT 10 LABELS "async-contract;offline;shared-engine")
set_target_properties(AsyncSharedEngineTest PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_include_directories(AsyncSharedEngineTest SYSTEM PRIVATE
        ${CMAKE_SOURCE_DIR}/sdk/include ${CMAKE_SOURCE_DIR}/sdk/src ${CMAKE_SOURCE_DIR}/sdk/src/external
        ${CLIENT_SSL_INCLUDE_DIRS} ${CLIENT_CURL_INCLUDE_DIRS})
target_link_libraries(AsyncSharedEngineTest PRIVATE ${ASYNC_CONTRACT_SDK_TARGET}
        ${CLIENT_CURL_LIBS} ${CLIENT_SSL_LIBS} Threads::Threads)
target_compile_options(AsyncSharedEngineTest PRIVATE -Wall -Wextra -Werror)
set_property(TARGET AsyncSharedEngineTest APPEND_STRING PROPERTY LINK_FLAGS
        " -Wl,--wrap=curl_easy_init,--wrap=curl_multi_init,--wrap=curl_multi_add_handle,--wrap=eventfd,--wrap=epoll_ctl,--wrap=curl_global_init,--wrap=curl_global_cleanup")
foreach(scenario reuse profiles deadline close admission reclaim capacity global_lifetime global_lifetime_failures setup public density large tls12 tls13)
    add_test(NAME AsyncShared_${scenario} COMMAND AsyncSharedEngineTest ${scenario})
    set_tests_properties(AsyncShared_${scenario} PROPERTIES TIMEOUT 90 LABELS "async-contract;offline;shared-engine"
        ENVIRONMENT "http_proxy=;https_proxy=;all_proxy=;HTTP_PROXY=;HTTPS_PROXY=;ALL_PROXY=;NO_PROXY=127.0.0.1;no_proxy=127.0.0.1")
endforeach()
add_executable(AsyncSchedulingStressTest ${CMAKE_CURRENT_LIST_DIR}/AsyncSchedulingStressTest.cc)
set_target_properties(AsyncSchedulingStressTest PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_include_directories(AsyncSchedulingStressTest SYSTEM PRIVATE
        ${CMAKE_SOURCE_DIR}/sdk/include ${CMAKE_SOURCE_DIR}/sdk/src ${CMAKE_SOURCE_DIR}/sdk/src/external
        ${CLIENT_SSL_INCLUDE_DIRS} ${CLIENT_CURL_INCLUDE_DIRS})
target_link_libraries(AsyncSchedulingStressTest PRIVATE ${ASYNC_CONTRACT_SDK_TARGET}
        ${CLIENT_CURL_LIBS} ${CLIENT_SSL_LIBS} Threads::Threads)
target_compile_options(AsyncSchedulingStressTest PRIVATE -Wall -Wextra -Werror)
set_property(TARGET AsyncSchedulingStressTest APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--wrap=curl_easy_pause")
foreach(scenario mixed large upload_qos close_race reentrant timeout_mixed cache_throw cache_fail cache_overconsume cache_zero
        cache_throw_int cache_fail_full cache_unpause cache_rearm cache_skip_fail error_reentrant cache_resume_rearm
        cache_resume_thread live_resume_get live_resume_put pattern_length pattern_chunked pattern_close pattern_short
        failure_timeout_race mixed_wide mixed_info resource_cycles)
    add_test(NAME AsyncStress_${scenario} COMMAND AsyncSchedulingStressTest ${scenario})
    set_tests_properties(AsyncStress_${scenario} PROPERTIES TIMEOUT 90 LABELS "async-contract;offline;stress"
        ENVIRONMENT "http_proxy=;https_proxy=;all_proxy=;HTTP_PROXY=;HTTPS_PROXY=;ALL_PROXY=;NO_PROXY=127.0.0.1;no_proxy=127.0.0.1")
endforeach()
set_property(TARGET AsyncSchedulingTest APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--wrap=eventfd")
foreach(scenario upload_qos close_race reentrant timeout_mixed cache_throw cache_fail cache_overconsume cache_zero
        cache_throw_int cache_fail_full cache_unpause cache_rearm cache_skip_fail error_reentrant cache_resume_rearm
        cache_resume_thread live_resume_get live_resume_put pattern_length pattern_chunked pattern_close pattern_short
        failure_timeout_race)
    add_test(NAME AsyncSharedStress_${scenario} COMMAND AsyncSchedulingStressTest shared_${scenario})
    set_tests_properties(AsyncSharedStress_${scenario} PROPERTIES TIMEOUT 90 LABELS "async-contract;offline;shared-engine"
        ENVIRONMENT "http_proxy=;https_proxy=;all_proxy=;HTTP_PROXY=;HTTPS_PROXY=;ALL_PROXY=;NO_PROXY=127.0.0.1;no_proxy=127.0.0.1")
endforeach()
# Ignore inherited proxy routing for the loopback-only test. These tests supply
# empty account credentials; they never inspect credential variables.
set_tests_properties(AsyncLegacyReadHttp AsyncReceiveException AsyncTransportSafety AsyncRetryPolicy AsyncScheduling PROPERTIES ENVIRONMENT
        "http_proxy=;https_proxy=;all_proxy=;HTTP_PROXY=;HTTPS_PROXY=;ALL_PROXY=;NO_PROXY=127.0.0.1;no_proxy=127.0.0.1")
endif()
if (BUILD_ASYNC_BUCKET_SMOKE)
    foreach(bucket_test AsyncBucketSmokeTest AsyncSharedBucketE2ETest)
        add_executable(${bucket_test} ${CMAKE_CURRENT_LIST_DIR}/${bucket_test}.cc)
        set_target_properties(${bucket_test} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
        target_include_directories(${bucket_test} SYSTEM PRIVATE
                ${CMAKE_SOURCE_DIR}/sdk/include ${CMAKE_SOURCE_DIR}/sdk/src
                ${CMAKE_SOURCE_DIR}/sdk/src/external ${CLIENT_SSL_INCLUDE_DIRS} ${CLIENT_CURL_INCLUDE_DIRS})
        target_link_libraries(${bucket_test} PRIVATE ${ASYNC_CONTRACT_SDK_TARGET}
                ${CLIENT_CURL_LIBS} ${CLIENT_SSL_LIBS} Threads::Threads)
    endforeach()
endif()
