cmake_minimum_required(VERSION 3.11)
project(ve-tos-cpp-sdk-lib VERSION 0.1.0)
set(SDK_HEADER
        include/auth/Credential.h
        include/auth/Credentials.h
        include/auth/Signer.h
        include/auth/SignKeyInfo.h
        include/auth/StaticCredentials.h
        include/auth/FederationCredentials.h
        include/auth/FederationToken.h
        include/auth/FederationTokenProvider.h
        include/common/Common.h
        include/transport/http/HttpClient.h
        include/transport/http/HttpRequest.h
        include/transport/http/HttpResponse.h
        include/transport/http/Url.h
        include/transport/Transport.h
        include/transport/TransportConfig.h
        include/model/acl/Acl.h
        include/model/acl/GetObjectAclOutput.h
        include/model/acl/Grant.h
        include/model/acl/ObjectAclGrant.h
        include/model/acl/ObjectAclRules.h
        include/model/acl/PutObjectAclInput.h
        include/model/acl/PutObjectAclOutput.h
        include/model/acl/Grantee.h
        include/model/acl/Owner.h
        include/model/bucket/CreateBucketInput.h
        include/model/bucket/DeleteBucketOutput.h
        include/model/bucket/HeadBucketOutput.h
        include/model/bucket/ListBucketsInput.h
        include/model/bucket/ListBucketsOutput.h
        include/model/bucket/ListedBucket.h
        include/model/bucket/CreateBucketOutput.h
        include/model/bucket/ListedOwner.h
        include/model/object/AbortMultipartUploadInput.h
        include/model/object/AbortMultipartUploadOutput.h
        include/model/object/AppendObjectOutput.h
        include/model/object/CompleteMultipartUploadInput.h
        include/model/object/InnerUploadedPart.h
        include/model/object/CompleteMultipartUploadOutput.h
        include/model/object/CopyObjectOutput.h
        include/model/object/CreateMultipartUploadOutput.h
        include/model/object/Deleted.h
        include/model/object/DeleteError.h
        include/model/object/DeleteMultiObjectsInput.h
        include/model/object/ObjectTobeDeleted.h
        include/model/object/DeleteMultiObjectsOutput.h
        include/model/object/DeleteObjectOutput.h
        include/model/object/GetObjectOutput.h
        include/model/object/ObjectMeta.h
        include/model/object/HeadObjectOutput.h
        include/model/object/ListedCommonPrefix.h
        include/model/object/ListedDeleteMarkerEntry.h
        include/model/object/ListedObject.h
        include/model/object/ListedObjectVersion.h
        include/model/object/ListMultipartUploadsInput.h
        include/model/object/ListMultipartUploadsOutput.h
        include/model/object/UploadInfo.h
        include/model/object/UploadCommonPrefix.h
        include/model/object/ListObjectsInput.h
        include/model/object/ListObjectsOutput.h
        include/model/object/ListObjectVersionsInput.h
        include/model/object/ListObjectVersionsOutput.h
        include/model/object/ListUploadedPartsInput.h
        include/model/object/ListUploadedPartsOutput.h
        include/model/object/UploadedPart.h
        include/model/object/MultipartUpload.h
        include/model/object/PutObjectOutput.h
        include/model/object/SetObjectMetaOutput.h
        include/model/object/UploadPartCopyInput.h
        include/model/object/UploadPartCopyOutput.h
        include/model/object/UploadPartInput.h
        include/model/object/UploadPartOutput.h
        include/model/RequestInfo.h
        include/utils/BaseUtils.h
        include/TosResponse.h
        include/TosRequest.h
        include/Outcome.h
        include/TosError.h
        include/TosClient.h
        include/Config.h
        include/RequestBuilder.h
        include/RequestOptionBuilder.h
        src/TosClientImpl.h )
set(SDK_LIB
        src/transport/http/HttpClient.cc
        src/transport/http/HttpRequest.cc
        src/transport/http/HttpResponse.cc
        src/transport/http/Url.cc
        src/transport/Transport.cc
        src/transport/TransportConfig.cc
        src/transport/DefaultTransport.h
        src/transport/DefaultTransport.cc
        src/utils/BaseUtils.cc
        src/auth/SignV4.h
        src/auth/SignV4.cc
        src/auth/Signer.cc
        src/auth/FederationCredentials.cc
        src/external/json/json.hpp
        src/model/acl/ObjectAclRules.cc
        src/model/acl/GetObjectAclOutput.cc
        src/model/bucket/ListBucketsOutput.cc
        src/model/object/GetObjectOutput.cc
        src/model/object/ObjectMeta.cc
        src/model/object/ListObjectsOutput.cc
        src/model/object/ListObjectVersionsOutput.cc
        src/model/object/CopyObjectOutput.cc
        src/model/object/UploadPartCopyOutput.cc
        src/model/object/MultipartUpload.cc
        src/model/object/CompleteMultipartUploadInput.cc
        src/model/object/ListUploadedPartsOutput.cc
        src/model/object/ListMultipartUploadsOutput.cc
        src/model/object/DeleteMultiObjectsInput.cc
        src/model/object/DeleteMultiObjectsOutput.cc
        src/TosRequest.cc
        src/RequestBuilder.cc
        src/TosClient.cc
        src/TosError.cc
        src/TosResponse.cc
        src/TosClientImpl.cc
        src/utils/MimeType.h )

if (BUILD_SHARED_LIB)
    set(STATIC_LIB_SUFFIX "-static")
else()
    set(STATIC_LIB_SUFFIX "")
endif()

add_library(${PROJECT_NAME}${STATIC_LIB_SUFFIX} STATIC ${SDK_LIB} ${SDK_HEADER})
set_target_properties(${PROJECT_NAME}${STATIC_LIB_SUFFIX} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        OUTPUT_NAME ${PROJECT_NAME}${STATIC_LIB_SUFFIX}
        )
target_include_directories(${PROJECT_NAME}${STATIC_LIB_SUFFIX} PRIVATE include)
target_include_directories(${PROJECT_NAME}${STATIC_LIB_SUFFIX} PRIVATE ${CLIENT_SSL_INCLUDE_DIRS} PRIVATE ${CLIENT_CURL_INCLUDE_DIRS})

if (BUILD_SHARED_LIB)
    add_library(${PROJECT_NAME} SHARED ${SDK_LIB} ${SDK_HEADER})
    set_target_properties(${PROJECT_NAME} PROPERTIES
            CXX_STANDARD 11
            CXX_STANDARD_REQUIRED YES
            CXX_EXTENSIONS NO
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
            OUTPUT_NAME ${PROJECT_NAME}
            )
    target_include_directories(${PROJECT_NAME} PUBLIC include)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CLIENT_SSL_INCLUDE_DIRS} PRIVATE ${CLIENT_CURL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${CLIENT_SSL_LIBS})
    target_link_libraries(${PROJECT_NAME} ${CLIENT_CURL_LIBS})
endif ()

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}${STATIC_LIB_SUFFIX}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if (BUILD_SHARED_LIB)
    install(TARGETS ${PROJECT_NAME}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()