cmake_minimum_required(VERSION 3.1)
project(ve-tos-cpp-sdk-example
        VERSION 2.6.26
        LANGUAGES CXX)

function(ve_tos_link_example target)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/sdk/include)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/sdk/src/external)
    target_link_libraries(${target} ve-tos-cpp-sdk-lib)
    target_link_libraries(${target} ${CLIENT_SSL_LIBS})
    target_link_libraries(${target} ${CLIENT_CURL_LIBS})
    if (ASYNC_IO_URING_LIBS)
        target_link_libraries(${target} ${ASYNC_IO_URING_LIBS})
    endif ()
    if (UNIX AND NOT APPLE)
        target_link_libraries(${target} ${CMAKE_DL_LIBS})
    endif ()
endfunction()

add_executable(${PROJECT_NAME} example.cc)
ve_tos_link_example(${PROJECT_NAME})

if (BUILD_ASYNC_SDK)
    add_executable(ve-tos-cpp-sdk-async-file-transfer-benchmark async_file_transfer_benchmark.cc)
    ve_tos_link_example(ve-tos-cpp-sdk-async-file-transfer-benchmark)

    add_executable(ve-tos-cpp-sdk-async-example async_example.cc)
    ve_tos_link_example(ve-tos-cpp-sdk-async-example)

    add_executable(ve-tos-cpp-sdk-async-api-smoke async_example.cc)
    ve_tos_link_example(ve-tos-cpp-sdk-async-api-smoke)
endif ()

#target_compile_options(${PROJECT_NAME} PRIVATE "${SDK_COMPILER_FLAGS}")

install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
