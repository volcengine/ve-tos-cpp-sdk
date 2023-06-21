cmake_minimum_required(VERSION 3.1)
project(ve-tos-cpp-sdk-lib
        VERSION 2.5.7
        LANGUAGES CXX)
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
        include/model/bucket/CreateBucketV2Input.h
        include/model/bucket/CreateBucketInput.h
        include/model/bucket/DeleteBucketOutput.h
        include/model/bucket/HeadBucketV2Output.h
        include/model/bucket/ListBucketsInput.h
        include/model/bucket/ListBucketsOutput.h
        include/model/bucket/ListedBucket.h
        include/model/bucket/CreateBucketV2Output.h
        include/model/bucket/CreateBucketOutput.h
        include/model/bucket/ListedOwner.h
        include/model/bucket/DeleteBucketPolicyOutput.h
        include/model/bucket/GetBucketPolicyOutput.h
        include/model/bucket/PutBucketPolicyOutput.h
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
        include/model/object/ListedDeleteMarker.h
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
        include/model/object/UploadFileOutput.h
        include/model/object/UploadFileInput.h
        include/model/object/UploadFileInfo.h
        include/model/object/UploadFileCheckpoint.h
        include/model/object/UploadFilePartInfo.h
        include/model/RequestInfo.h
        include/utils/BaseUtils.h
        include/utils/crc64.h
        include/ClientConfig.h
        include/TosResponse.h
        include/TosRequest.h
        include/Outcome.h
        include/TosError.h
        include/TosClient.h
        include/TosClientV2.h
        include/Config.h
        include/RequestBuilder.h
        include/RequestOptionBuilder.h
        include/model/object/DeleteObjectInput.h
        include/model/object/CopyObjectV2Output.h
        include/model/object/CopyObjectV2Input.h
        include/model/acl/GetObjectAclV2Input.h
        include/model/acl/GetObjectAclV2Output.h
        include/model/bucket/CreateBucketV2Output.h
        include/model/bucket/HeadBucketV2Output.h
        include/model/bucket/HeadBucketV2Input.h
        include/model/bucket/DeleteBucketInput.h
        include/model/bucket/CreateBucketV2Input.h
        include/model/object/AppendObjectV2Output.h
        include/model/object/AppendObjectV2Input.h
        include/model/object/CompleteMultipartUploadV2Output.h
        include/model/object/CompleteMultipartUploadV2Input.h
        include/model/object/CreateMultipartUploadInput.h
        include/model/object/GetObjectBasicOutput.h
        include/model/object/GetObjectV2Output.h
        include/model/object/GetObjectV2Input.h
        include/model/object/GetObjectToFileInput.h
        include/model/object/GetObjectToFileOutput.h
        include/model/object/HeadObjectV2Output.h
        include/model/object/HeadObjectV2Input.h
        include/model/object/ListObjectsV2Output.h
        include/model/object/ListMultipartUploadsV2Output.h
        include/model/object/ListMultipartUploadsV2Input.h
        include/model/object/ListObjectsV2Input.h
        include/model/object/ListedObjectV2.h
        include/model/object/ListObjectVersionsV2Input.h
        include/model/object/ListObjectVersionsV2Output.h
        include/model/object/ListPartsOutput.h
        include/model/object/ListPartsInput.h
        include/model/object/PutObjectV2Input.h
        include/model/object/PutObjectV2Output.h
        include/model/object/PutObjectFromFileOutput.h
        include/model/object/PutObjectFromFileIntput.h
        include/model/object/UploadPartCopyV2Input.h
        include/model/object/UploadPartCopyV2Output.h
        include/model/object/UploadPartV2Output.h
        include/model/object/UploadPartV2Input.h
        include/model/object/UploadPartFromFileOutput.h
        include/model/object/UploadPartFromFileInput.h
        include/model/acl/PutObjectAclV2Input.h
        include/model/acl/PutObjectAclV2Output.h
        include/model/object/SetObjectMetaInput.h
        include/Type.h
        include/model/object/UploadFileV2Input.h
        include/model/object/UploadFileV2Output.h
        include/model/object/UploadFileCheckpointV2.h
        include/model/object/DownloadFileObjectInfo.h
        include/model/object/DownloadFilePartInfo.h
        include/model/object/DownloadFileCheckpoint.h
        include/model/object/DownloadFileFileInfo.h
        include/model/object/DownloadFileInput.h
        include/model/object/DownloadFileOutput.h
        include/model/object/UploadFilePartInfoV2.h
        include/model/object/UploadFileInfoV2.h
        include/model/object/PreSignedURLInput.h
        include/model/object/PreSignedURLOutput.h
        include/model/bucket/PutBucketCORSInput.h
        include/model/bucket/PutBucketCORSOutput.h
        include/model/bucket/GetBucketCORSInput.h
        include/model/bucket/GetBucketCORSOutput.h
        include/model/bucket/DeleteBucketCORSInput.h
        include/model/bucket/DeleteBucketCORSOutput.h
        include/model/bucket/CORSRule.h
        include/model/object/ListObjectsType2Input.h
        include/model/object/ListObjectsType2Output.h
        include/model/bucket/PutBucketStorageClassOutput.h
        include/model/bucket/PutBucketStorageClassInput.h
        include/model/bucket/GetBucketLocationOutput.h
        include/model/bucket/GetBucketLocationInput.h
        include/model/bucket/PutBucketLifecycleInput.h
        include/model/bucket/PutBucketLifecycleOutput.h
        include/model/bucket/GetBucketLifecycleInput.h
        include/model/bucket/GetBucketLifecycleOutput.h
        include/model/bucket/DeleteBucketLifecycleInput.h
        include/model/bucket/DeleteBucketLifecycleOutput.h
        include/model/bucket/PutBucketMirrorBackInput.h
        include/model/bucket/PutBucketMirrorBackOutput.h
        include/model/bucket/GetBucketMirrorBackInput.h
        include/model/bucket/GetBucketMirrorBackOutput.h
        include/model/bucket/DeleteBucketMirrorBackInput.h
        include/model/bucket/DeleteBucketMirrorBackOutput.h
        include/model/object/PutObjectTaggingInput.h
        include/model/object/PutObjectTaggingOutput.h
        include/model/object/GetObjectTaggingInput.h
        include/model/object/GetObjectTaggingOutput.h
        include/model/object/DeleteObjectTaggingInput.h
        include/model/object/DeleteObjectTaggingOutput.h
        include/model/acl/PutBucketAclInput.h
        include/model/acl/PutBucketAclOutput.h
        include/model/acl/GetBucketAclInput.h
        include/model/acl/GetBucketAclOutput.h
        include/model/object/FetchObjectInput.h
        include/model/object/FetchObjectOutput.h
        include/model/object/PutFetchTaskInput.h
        include/model/object/PutFetchTaskOutput.h
        include/model/acl/PreSignedPostSignatureInput.h
        include/model/acl/PreSignedPostSignatureOutput.h
        include/model/bucket/PutBucketReplicationInput.h
        include/model/bucket/PutBucketReplicationOutput.h
        include/model/bucket/GetBucketReplicationInput.h
        include/model/bucket/GetBucketReplicationOutput.h
        include/model/bucket/DeleteBucketReplicationInput.h
        include/model/bucket/DeleteBucketReplicationOutput.h
        include/model/bucket/PutBucketVersioningInput.h
        include/model/bucket/PutBucketVersioningOutput.h
        include/model/bucket/GetBucketVersioningInput.h
        include/model/bucket/GetBucketVersioningOutput.h
        include/model/bucket/PutBucketWebsiteInput.h
        include/model/bucket/PutBucketWebsiteOutput.h
        include/model/bucket/GetBucketWebsiteInput.h
        include/model/bucket/GetBucketWebsiteOutput.h
        include/model/bucket/DeleteBucketWebsiteInput.h
        include/model/bucket/DeleteBucketWebsiteOutput.h
        include/model/bucket/PutBucketNotificationInput.h
        include/model/bucket/PutBucketNotificationOutput.h
        include/model/bucket/GetBucketNotificationInput.h
        include/model/bucket/GetBucketNotificationOutput.h
        include/model/bucket/PutBucketCustomDomainInput.h
        include/model/bucket/PutBucketCustomDomainOutput.h
        include/model/bucket/ListBucketCustomDomainInput.h
        include/model/bucket/ListBucketCustomDomainOutput.h
        include/model/bucket/DeleteBucketCustomDomainInput.h
        include/model/bucket/DeleteBucketCustomDomainOutput.h
        include/model/bucket/PutBucketRealTimeLogInput.h
        include/model/bucket/PutBucketRealTimeLogOutput.h
        include/model/bucket/GetBucketRealTimeLogInput.h
        include/model/bucket/GetBucketRealTimeLogOutput.h
        include/model/bucket/DeleteBucketRealTimeLogInput.h
        include/model/bucket/DeleteBucketRealTimeLogOutput.h
        include/model/acl/PreSignedPolicyURLInput.h
        include/model/acl/PreSignedPolicyURLOutput.h
        include/model/object/ResumableCopyObjectInput.h
        include/model/object/ResumableCopyObjectOutput.h
        )
set(SDK_LIB
        src/transport/http/HttpClient.cc
        src/transport/http/HttpRequest.cc
        src/transport/http/HttpResponse.cc
        src/transport/http/Url.cc
        src/transport/Transport.cc
        src/transport/DefaultTransport.h
        src/transport/DefaultTransport.cc
        src/utils/BaseUtils.cc
        src/utils/crc64.cc
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
        src/model/object/UploadFileCheckpoint.cc
        src/model/object/UploadPartOutput.cc
        src/model/object/UploadFilePartInfo.cc
        src/model/object/UploadFileInfo.cc
        src/TosRequest.cc
        src/RequestBuilder.cc
        src/TosClient.cc
        src/TosClientV2.cc
        src/TosError.cc
        src/TosResponse.cc
        src/TosClientImpl.h
        src/TosClientImpl.cc
        src/utils/MimeType.h
        src/model/object/GetObjectBasicOutput.cc
        src/model/object/CopyObjectV2Output.cc
        src/model/object/ListObjectsV2Output.cc
        src/model/object/ListObjectVersionsV2Output.cc
        src/model/acl/GetObjectAclV2Output.cc
        src/model/object/HeadObjectV2Output.cc
        src/utils/LogUtils.h
        src/model/object/ResumableCopyCheckpoint.cc
        src/model/bucket/GetBucketWebsiteOutput.cc
        src/model/object/ResumableCopyCopySourceObjectInfo.cc
        src/model/acl/GetBucketAclOutput.cc
        src/model/acl/PolicyURLInner.cc
        src/model/acl/PutBucketAclInput.cc
        src/model/acl/PutObjectAclV2Input.cc
        src/model/bucket/GetBucketCORSOutput.cc
        src/model/bucket/GetBucketLocationOutput.cc
        src/model/bucket/GetBucketNotificationOutput.cc
        src/model/bucket/GetBucketRealTimeLogOutput.cc
        src/model/bucket/GetBucketReplicationOutput.cc
        src/model/bucket/GetBucketWebsiteOutput.cc
        src/model/bucket/ListBucketCustomDomainOutput.cc
        src/model/bucket/PutBucketCORSInput.cc
        src/model/bucket/PutBucketCustomDomainInput.cc
        src/model/bucket/PutBucketNotificationInput.cc
        src/model/bucket/PutBucketRealTimeLogInput.cc
        src/model/bucket/PutBucketMirrorBackInput.cc
        src/model/bucket/GetBucketMirrorBackOutput.cc
        src/model/bucket/PutBucketReplicationInput.cc
        src/model/bucket/PutBucketWebsiteInput.cc
        src/model/bucket/PutBucketLifecycleInput.cc
        src/model/bucket/GetBucketLifecycleOutput.cc
        src/model/object/CompleteMultipartUploadV2Input.cc
        src/model/object/CompleteMultipartUploadV2Output.cc
        src/model/object/CopyObjectInner.cc
        src/model/object/DownloadFileCheckpoint.cc
        src/model/object/ListedObjectType2Output.cc
        src/model/object/ListMultipartUploadsV2Output.cc
        src/model/object/ListPartsOutput.cc
        src/model/object/PostPolicyInner.cc
        src/model/object/ResumableCopyCheckpoint.cc
        src/model/object/ResumableCopyCopySourceObjectInfo.cc
        src/model/object/ResumableCopyPartInfo.cc
        src/model/object/UploadPartCopyInner.cc
        src/model/object/UploadPartCopyV2Output.cc
        src/model/acl/PreSignedPolicyURLOutput.cc
        src/model/object/DownloadFileFileInfo.cc
        src/model/object/PutObjectTaggingInput.cc
        src/model/object/GetObjectTaggingOutput.cc
        src/model/object/FetchObjectInput.cc
        src/model/object/DownloadFileObjectInfo.cc
        src/model/object/FetchObjectOutput.cc
        src/model/object/PutFetchTaskInput.cc
        src/model/object/PutFetchTaskOutput.cc
        src/model/bucket/PutBucketVersioningInput.cc
        src/model/object/DownloadFilePartInfo.cc
        src/model/bucket/GetBucketVersioningOutput.cc
        src/model/object/UploadFileInfoV2.cc
        src/model/object/UploadFilePartInfoV2.cc
        src/model/object/UploadFileCheckpointV2.cc
        )

# add spdlog
include_directories(./src/external)
file(GLOB spdlogh "src/external/spdlog/*.h")
file(GLOB spdlogcfg "src/external/spdlog/cfg/*.h")
file(GLOB spdlogdetail "src/external/spdlog/details/*.h")
file(GLOB spdlogfmt "src/external/spdlog/fmt/*.h")
file(GLOB spdlogsinks "src/external/spdlog/sinks/*.h")
file(GLOB spdlog ${spdlogh} ${spdlogcfg} ${spdlogdetail} ${spdlogfmt} ${spdlogsinks})

if (BUILD_SHARED_LIB)
    set(STATIC_LIB_SUFFIX "-static")
else ()
    set(STATIC_LIB_SUFFIX "")
endif ()

add_library(${PROJECT_NAME}${STATIC_LIB_SUFFIX} STATIC ${SDK_LIB} ${SDK_HEADER} ${spdlog})
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
    add_library(${PROJECT_NAME} SHARED ${SDK_LIB} ${SDK_HEADER} ${spdlog})
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

target_compile_options(${PROJECT_NAME} PRIVATE "${SDK_COMPILER_FLAGS}")

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
