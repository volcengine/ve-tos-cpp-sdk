#pragma once

#include <map>
#include <string>
#include <vector>

namespace VolcengineTos {
namespace http {
static const char* MethodGet = "GET";
static const char* MethodHead = "HEAD";
static const char* MethodPost = "POST";
static const char* MethodPut = "PUT";
static const char* MethodDelete = "DELETE";

static const char* SchemeHTTP = "HTTP";
static const char* SchemeHTTPS = "HTTPS";

static const int Success = 0;
static const int Refused = -1;
static const int otherErr = -2;

/**
 * HTTP header
 */
static const char* HEADER_USER_AGENT = "User-Agent";
static const char* HEADER_CONTENT_LENGTH = "Content-Length";
static const char* HEADER_CONTENT_TYPE = "Content-Type";
static const char* HEADER_CONTENT_MD5 = "Content-MD5";
static const char* HEADER_CONTENT_LANGUAGE = "Content-Language";
static const char* HEADER_CONTENT_ENCODING = "Content-Encoding";
static const char* HEADER_CONTENT_DISPOSITION = "Content-Disposition";
static const char* HEADER_LAST_MODIFIED = "Last-Modified";
static const char* HEADER_CACHE_CONTROL = "Cache-Control";
static const char* HEADER_EXPIRES = "Expires";
static const char* HEADER_ETAG = "ETag";
static const char* HEADER_IF_MODIFIED_SINCE = "If-Modified-Since";
static const char* HEADER_IF_UNMODIFIED_SINCE = "If-Unmodified-Since";
static const char* HEADER_IF_MATCH = "If-Match";
static const char* HEADER_IF_NONE_MATCH = "If-None-Match";
static const char* HEADER_RANGE = "Range";
static const char* HEADER_CONTENT_RANGE = "Content-Range";
static const char* HEADER_LOCATION = "Location";
static const char* HEADER_SDK_RETRY_COUNT = "X-Sdk-Retry-Count";
static const char* HEADER_Retry_After = "Retry-After";
}  // namespace http

/**
 * TOS Header
 */
static const char* HEADER_CONTENT_SHA256 = "X-Tos-Content-Sha256";
static const char* HEADER_VERSIONID = "X-Tos-Version-Id";
static const char* HEADER_DELETE_MARKER = "X-Tos-Delete-Marker";
static const char* HEADER_STORAGE_CLASS = "X-Tos-Storage-Class";
static const char* HEADER_RESTORE = "X-Tos-Restore";
static const char* HEADER_MIRROR_TAG = "X-Tos-Tag";
static const char* HEADER_SSE_CUSTOMER_ALGORITHM = "X-Tos-Server-Side-Encryption-Customer-Algorithm";
static const char* HEADER_SSE_CUSTOMER_KEY_MD5 = "X-Tos-Server-Side-Encryption-Customer-Key-MD5";
static const char* HEADER_SSE_CUSTOMER_KEY = "X-Tos-Server-Side-Encryption-Customer-Key";
static const char* HEADER_SSE = "x-tos-server-side-encryption";
static const char* HEADER_CRC64 = "x-tos-hash-crc64ecma";
static const char* HEADER_REQUEST_ID = "X-Tos-Request-Id";
static const char* HEADER_ACCOUNT_ID = "X-Tos-Account-Id";
static const char* HEADER_ID_2 = "X-Tos-Id-2";
static const char* HEADER_BUCKET_REGION = "X-Tos-Bucket-Region";
static const char* HEADER_ACL = "X-Tos-Acl";
static const char* HEADER_GRANT_FULL_CONTROL = "X-Tos-Grant-Full-Control";
static const char* HEADER_GRANT_READ = "X-Tos-Grant-Read";
static const char* HEADER_GRANT_READ_ACP = "X-Tos-Grant-Read-Acp";
static const char* HEADER_GRANT_WRITE = "X-Tos-Grant-Write";
static const char* HEADER_GRANT_WRITE_ACP = "X-Tos-Grant-Write-Acp";
static const char* HEADER_AZ_REDUNDANCY = "X-Tos-Az-Redundancy";
static const char* HEADER_NEXT_APPEND_OFFSET = "X-Tos-Next-Append-Offset";
static const char* HEADER_OBJECT_TYPE = "X-Tos-Object-Type";
static const char* HEADER_METADATA_DIRECTIVE = "X-Tos-Metadata-Directive";
static const char* HEADER_COPY_SOURCE = "X-Tos-Copy-Source";
static const char* HEADER_COPY_SOURCE_IF_MATCH = "X-Tos-Copy-Source-If-Match";
static const char* HEADER_COPY_SOURCE_IF_NONE_MATCH = "X-Tos-Copy-Source-If-None-Match";
static const char* HEADER_COPY_SOURCE_IF_MODIFIED_SINCE = "X-Tos-Copy-Source-If-Modified-Since";
static const char* HEADER_COPY_SOURCE_IF_UNMODIFIED_SINCE = "X-Tos-Copy-Source-If-Unmodified-Since";
static const char* HEADER_Trash_Path = "x-tos-trash-path";
static const char* HEADER_EC_CODE = "X-Tos-Ec";

static const char* HEADER_COPY_SOURCE_SSE_CUSTOMER_ALGORITHM =
        "X-Tos-Copy-Source-Server-Side-Encryption-Customer-Algorithm";
static const char* HEADER_COPY_SOURCE_SSE_CUSTOMER_KEY = "X-Tos-Copy-Source-Server-Side-Encryption-Customer-key";
static const char* HEADER_COPY_SOURCE_SSE_CUSTOMER_KEY_MD5 =
        "X-Tos-Copy-Source-Server-Side-Encryption-Customer-key-MD5";

static const char* HEADER_COPY_SOURCE_RANGE = "X-Tos-Copy-Source-Range";
static const char* HEADER_COPY_SOURCE_VERSION_ID = "X-Tos-Copy-Source-Version-Id";
static const char* HEADER_WEBSITE_REDIRECT_LOCATION = "X-Tos-Website-Redirect-Location";
static const char* HEADER_CS_TYPE = "X-Tos-Cs-Type";
static const char* HEADER_META_PREFIX = "X-Tos-Meta-";
static const char* HEADER_TRAFFIC_LIMIT = "X-Tos-Traffic-Limit";
static const char* HEADER_COMPLETE_ALL = "X-Tos-Complete-All";
static const char* HEADER_CALLBACK = "X-Tos-Callback";
static const char* HEADER_CALLBACK_VAR = "X-Tos-Callback-Var";
static const char* HEADER_NOTIFY_CUSTOM_PARAM = "X-Tos-Notification-Custom-Parameters";
// HNS
static const char* HEADER_DIRECTORY = "X-Tos-Directory";
static const char* HEADER_BUCKET_TYPE = "X-Tos-Bucket-Type";
static const char* HEADER_CRC32 = "x-tos-hash-crc32c";
static const char* HEADER_NEXT_MODIFY_OFFSET = "x-tos-next-modify-offset";
static const char* HEADER_FORBID_OVERWRITE = "x-tos-forbid-overwrite";
static const char* HEADER_RECURSIVE_MKDIR = "x-tos-recursive-mkdir";


/**
 * replace source object metadata when calling copyObject
 */
static const char* METADATA_DIRECTIVE_REPLACE = "REPLACE";

/**
 * copy source object metadata when calling copyObject
 */
static const char* METADATA_DIRECTIVE_COPY = "COPY";

}  // namespace VolcengineTos
