cmake_minimum_required(VERSION 3.1)
project(ve-tos-cpp-sdk-example
        VERSION 0.2.1
        LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

add_executable(${PROJECT_NAME} example.cc)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/sdk/include)
target_link_libraries(${PROJECT_NAME} ve-tos-cpp-sdk-lib)
target_link_libraries(${PROJECT_NAME} ${CLIENT_SSL_LIBS})
target_link_libraries(${PROJECT_NAME} ${CLIENT_CURL_LIBS})

install(TARGETS  ${PROJECT_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )