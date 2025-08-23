cmake_minimum_required(VERSION 3.1)
project(ve-tos-cpp-sdk-example
        VERSION 2.6.18
        LANGUAGES CXX)

add_executable(${PROJECT_NAME} example.cc)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/sdk/include)
# 依赖的日志库，需要引入该搜索目录
#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/sdk/src/external)

# 链接静态编译的 libve-tos-cpp-sdk-lib.a 静态库
target_link_libraries(${PROJECT_NAME}${STATIC_LIB_SUFFIX} ve-tos-cpp-sdk-lib)
# 如果想要使用动态库，linux 用以下语句替换上面链接静态库的语句:
# target_link_libraries(${PROJECT_NAME} ve-tos-cpp-sdk-lib.so)

target_link_libraries(${PROJECT_NAME} ${CLIENT_SSL_LIBS})
target_link_libraries(${PROJECT_NAME} ${CLIENT_CURL_LIBS})

#target_compile_options(${PROJECT_NAME} PRIVATE "${SDK_COMPILER_FLAGS}")

install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )