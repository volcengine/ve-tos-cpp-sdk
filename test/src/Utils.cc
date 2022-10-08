#include <string>
#include <fstream>
#include "Utils.h"

using namespace VolcengineTos;
std::string TestUtils::GetBucketName(const std::string& prefix) {
    std::stringstream ss;
    auto time_point = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());

    ss << prefix << "-bucket-" << time_point.time_since_epoch().count();
    std::string res = ss.str();
    return res;
}

std::string TestUtils::GetObjectKey(const std::string& prefix) {
    std::stringstream ss;
    auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    ss << prefix << "-object-" << tp.time_since_epoch().count();
    return ss.str();
}

void TestUtils::CreateBucket(const std::shared_ptr<TosClientV2>& client, const std::string& name) {
    CreateBucketV2Input input_v2;
    input_v2.setBucket(name);
    auto output = client->createBucket(input_v2);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
}

void TestUtils::PutObject(const std::shared_ptr<TosClientV2>& client, const std::string& bkt_name,
                          const std::string& obj_name, const std::string& data) {
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put(bkt_name, obj_name, ss);
    auto output = client->putObject(input_obj_put);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
}

void TestUtils::PutObjectWithMeta(const std::shared_ptr<TosClientV2>& client, const std::string& bkt_name,
                                  const std::string& obj_name, const std::string& data,
                                  const std::map<std::string, std::string>& meta) {
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input_obj_put(bkt_name, obj_name, ss);
    auto basic_input = input_obj_put.getPutObjectBasicInput();
    basic_input.setMeta(meta);
    input_obj_put.setPutObjectBasicInput(basic_input);
    auto output = client->putObject(input_obj_put);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
}

std::string TestUtils::GetObjectContent(const std::shared_ptr<TosClientV2>& client, const std::string& bkt_name,
                                        const std::string& obj_name) {
    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    input_obj_get.setKey(obj_name);
    auto output_obj_get = client->getObject(input_obj_get);
    auto basic_output = output_obj_get.result().getGetObjectBasicOutput();
    auto content_output = output_obj_get.result().getContent();
    std::string ss_;
    auto stream = content_output.get();
    auto header = output_obj_get.result().getGetObjectBasicOutput().getRequestInfo().getHeaders();
    auto content_length = std::stoi(header["Content-Length"]);
    char streamBuffer[content_length + 1];
    memset(streamBuffer, 0, content_length + 1);
    while (stream->good()) {
        stream->read(streamBuffer, content_length + 1);
    }
    std::string tmp_string(streamBuffer);
    return tmp_string;
}
std::string TestUtils::GetObjectContentByStream(const std::shared_ptr<TosClientV2>& client, const std::string& bkt_name,
                                                const std::string& obj_name) {
    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    input_obj_get.setKey(obj_name);
    auto output_obj_get = client->getObject(input_obj_get);
    std::ostringstream ss;
    ss << output_obj_get.result().getContent()->rdbuf();
    std::string tmp_string = ss.str();
    return tmp_string;
}
std::map<std::string, std::string> TestUtils::GetObjectMeta(const std::shared_ptr<TosClientV2>& client,
                                                            const std::string& bkt_name, const std::string& obj_name) {
    GetObjectV2Input input_obj_get;
    input_obj_get.setBucket(bkt_name);
    input_obj_get.setKey(obj_name);
    auto output_obj_get = client->getObject(input_obj_get);
    return output_obj_get.result().getGetObjectBasicOutput().getMeta();
}

void TestUtils::CleanBucket(const std::shared_ptr<TosClientV2>& client, const std::string& bkt_name) {
    ListMultipartUploadsV2Input input_multipart_list(bkt_name);
    auto output_multipart_list = client->listMultipartUploads(input_multipart_list);
    auto uploads = output_multipart_list.result().getUploads();
    AbortMultipartUploadInput input_multipart_abort;
    input_multipart_abort.setBucket(bkt_name);
    for (const auto& content : uploads) {
        input_multipart_abort.setKey(content.getKey());
        input_multipart_abort.setUploadId(content.getUploadId());
        auto output_abort = client->abortMultipartUpload(input_multipart_abort);
    }
    // list and delete
    ListObjectsV2Input input_obj_list;
    input_obj_list.setBucket(bkt_name);
    auto output_obj_list = client->listObjects(input_obj_list);
    auto content_ = output_obj_list.result().getContents();
    std::vector<ListedObjectV2> testObject;
    DeleteObjectInput input_obj_delete;
    input_obj_delete.setBucket(bkt_name);
    for (const auto& obj_ : content_) {
        std::string obj_name = obj_.getKey();
        input_obj_delete.setKey(obj_name);
        auto output_obj_delete = client->deleteObject(input_obj_delete);
        if (!output_obj_delete.isSuccess()) {
            std::cout << output_obj_delete.error().String() << std::endl;
            return;
        }
    }

    DeleteBucketInput input_bkt_delete;
    input_bkt_delete.setBucket(bkt_name);
    auto output_bkt_delete = client->deleteBucket(input_bkt_delete);
    if (!output_bkt_delete.isSuccess()) {
        std::cout << output_bkt_delete.error().String() << std::endl;
        return;
    }
}

void TestUtils::CleanAllBucket(const std::shared_ptr<TosClientV2>& client) {
    ListBucketsInput input_list;
    auto output_list_all = client->listBuckets(input_list);
    auto buckets = output_list_all.result().getBuckets();
    for (const auto& bkt_ : buckets) {
        std::string bkt_name = bkt_.getName();
        auto idx = bkt_name.find("sdktest-");
        if (idx != std::string::npos) {
            std::cout << "Delete bucket name:" << bkt_name << std::endl;
            TestUtils::CleanBucket(client, bkt_name);
        }
    }
}
//  generate a random string include only [a-b]|[1-9]
std::string TestUtils::GetRandomString(int length) {
    std::string letterBytes = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::stringstream ss;
    for (int i = 0; i < length; i++) {
        int j = rand() % 35;
        ss << letterBytes[j];
    }
    return ss.str();
}
void TestUtils::GetRandomCharArray(int length, unsigned char* array) {
    std::string letterBytes = "abcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < length; i++) {
        int j = rand() % 35;
        array[i] = letterBytes[j];
    }
}
void TestUtils::WriteRandomDatatoFile(const std::string& file, int length) {
    std::fstream output_file(file, std::ios::out | std::ios::binary | std::ios::trunc);
    output_file << GetRandomString(length);
    output_file.close();
}

time_t TestUtils::GetTimeWithDelay(int64_t delay) {
    std::time_t t = std::time(nullptr);
    t += delay;
    return t;
}
