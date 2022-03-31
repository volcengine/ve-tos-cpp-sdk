#include <iostream>
#include "TosClient.h"
#include "auth/StaticCredentials.h"
#include "model/acl/Acl.h"
#include <string>
#include <fstream>

using namespace VolcengineTos;

void creatBucket(TosClient& client, const std::string & bkt){
  CreateBucketInput input;
  input.setBucket(bkt);
  auto output = client.createBucket(input);
  if (!output.isSuccess()){
    std::cout << output.error().String() << std::endl;
    return;
  }
  std::cout << output.result().getLocation() << std::endl;
}

void headBucket(TosClient& client, const std::string & name){
  auto output = client.headBucket(name);
  if (!output.isSuccess()){
    std::cout << output.error().String() << std::endl;
    return;
  }
  std::cout << "The bucket region is: "
            << output.result().getRegion()
            << std::endl;
}

void listBucket(TosClient& client) {
  ListBucketsInput input;
  auto output = client.listBuckets(input);
  if (!output.isSuccess()){
    std::cout << output.error().String() << std::endl;
    return;
  }
  auto bkts = output.result().getBuckets();
  for (auto & bkt :bkts) {
    std::cout << bkt.getName() << std::endl;
  }
}

void deleteBucket(TosClient& client, const std::string& name){
  auto output = client.deleteBucket(name);
  if (!output.isSuccess()){
    std::cout << output.error().String() << std::endl;
    return;
  }
  std::cout << "Delete bucket success, the req id is: "
            << output.result().getRequestInfo().getRequestId()
            << std::endl;
}

void putObject(TosClient& client, const std::string & bucket, const std::string & key){
  std::string data = "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'";
  auto ss = std::make_shared<std::stringstream>(data);
  RequestOptionBuilder rob;
  rob.withContentLength(data.length());
  rob.withContentType("application/json");
  rob.withMeta("self-test", "yes");
  auto output = client.putObject(bucket, key, ss, rob);
  if (!output.isSuccess()) {
    std::cout << "put object error: "
              << output.error().String()
              << std::endl;
    return;
  }
  std::cout << "put object success, object etag is: "
            << output.result().getEtag()
            << std::endl;
}

void putObject(TosClient& client, const std::string & bucket, const std::string & key, const std::string & path){
  auto content = std::make_shared<std::fstream>(path, std::ios::in | std::ios_base::binary);
  auto output = client.putObject(bucket, key, content);
  if (!output.isSuccess()) {
    std::cout << output.error().String() << std::endl;
    return;
  }
  std::cout << "the object etag is: "
            << output.result().getEtag()
            << std::endl;
}

void getObject(TosClient & client, const std::string & bucket, const std::string & key){
  auto output = client.getObject(bucket, key);
  if (!output.isSuccess()){
    std::cout << output.error().String() << std::endl;
    return;
  }
  std::cout << output.result().getObjectMeta().getEtags() << std::endl;
}

void headObject(TosClient & client, const std::string & bucket, const std::string & key){
  auto output = client.headObject(bucket, key);
  if (!output.isSuccess()){
    std::cout << output.error().String() << std::endl;
    return;
  }
  auto iter = output.result().getObjectMeta().getMetadata().begin();
  while (iter != output.result().getObjectMeta().getMetadata().end()) {
    std::cout << iter->first << "\t" << iter->second << std::endl;
    iter++;
  }
  std::cout << output.result().getObjectMeta().getContentType() << std::endl;
}

void deleteObject(TosClient & client, const std::string & bucket, const std::string & key){
  auto output = client.deleteObject(bucket, key);
  if (!output.isSuccess()){
    std::cout << output.error().String() << std::endl;
    return;
  }
  std::cout << output.result().getRequestInfo().getRequestId() << std::endl;
}

void appendObject(TosClient & client, const std::string & bucket, const std::string & key) {
  auto part0 = std::make_shared<std::stringstream>();
  for (int i = 0; i < (128 << 10); ++i) {
    *part0 << "1";
  }
  auto output = client.appendObject(bucket, key, part0, 0);
  if (!output.isSuccess()) {
    std::cout << output.error().String() << std::endl;
    return;
  }
  std::cout << "Next Append Offset is: "
            << output.result().getNextAppendOffset()
            << std::endl;
  auto part1 = std::make_shared<std::stringstream>();
  for (int i = 0; i < (256 << 10); ++i) {
    *part1 << "1";
  }
  auto output1 = client.appendObject(bucket, key, part1, output.result().getNextAppendOffset());
  if (!output1.isSuccess()) {
    std::cout << output1.error().String() << std::endl;
    return;
  }
  std::cout << "Next Append Offset is: "
            << output1.result().getNextAppendOffset()
            << std::endl;
}

void putObjectAcl(TosClient & client, const std::string & bucket, const std::string & key) {
  PutObjectAclInput input;
  input.setKey(key);
  ObjectAclGrant objectAclGrant;
  objectAclGrant.setAcl(VolcengineTos::ACL_PRIVATE);
  input.setAclGrant(objectAclGrant);
  auto putRes = client.putObjectAcl(bucket, input);
  if (!putRes.isSuccess()){
    std::cout << putRes.error().String() << std::endl;
    return;
  }
  std::cout << putRes.result().getRequestInfo().getRequestId() << std::endl;
}

void getObjectAcl(TosClient & client, const std::string & bucket, const std::string & key) {
  auto gotAcl = client.getObjectAcl(bucket, key);
  if (!gotAcl.isSuccess()){
    std::cout << gotAcl.error().String() << std::endl;
  }
  std::cout << gotAcl.result().getGrant()[0].getGrantee().getId() << std::endl;
}

void listObjects(TosClient & client, const std::string & bucket) {
  int maxKeys = 100;
  ListObjectsInput input;
  input.setMaxKeys(maxKeys);
  auto output = client.listObjects(bucket, input);
  if (!output.isSuccess()){
    std::cout << output.error().String() << std::endl;
  }
  for (const auto & i : output.result().getContents()) {
    std::cout << i.getKey() << std::endl;
  }
}

void listObjectVersion(TosClient & client, const std::string & bucket) {
  ListObjectVersionsInput input;
  input.setMaxKeys(100);
  auto output = client.listObjectVersions(bucket, input);
  if (!output.isSuccess()){
    std::cout << output.error().String() << std::endl;
  }
  for (const auto & i : output.result().getVersions()) {
    std::cout << i.getKey() << "\t" << i.getVersionId() << std::endl;
  }
}

void deleteObjectVersion(TosClient & client, const std::string & bucket) {
  std::string key("test0325-version");
  client.putObject(bucket, key, std::make_shared<std::stringstream>("put your data"));
  client.putObject(bucket, key, std::make_shared<std::stringstream>("put your new data"));
  ListObjectVersionsInput input;
  input.setMaxKeys(100);
  input.setPrefix("test0325");
  auto output = client.listObjectVersions(bucket, input);
  for (const auto & i : output.result().getVersions()) {
    std::cout << i.getKey() << "\t" << i.getVersionId() << std::endl;
    RequestOptionBuilder rob;
    rob.withVersionID(i.getVersionId());
    auto res = client.deleteObject(bucket, key, rob);
    std::cout << res.result().getVersionId() << std::endl;
  }
}

void getObjectRange(TosClient & client, const std::string & bucket, const std::string & key){
  RequestOptionBuilder rbo;
  rbo.withRange(0, 7);
  auto output = client.getObject(bucket, key, rbo);
  if (!output.isSuccess()){
    std::cout << output.error().String() << std::endl;
  }
  std::cout << output.result().getContent()->rdbuf() << std::endl;
}

void copyObject(TosClient& client, const std::string & bucket) {
  std::string dstBkt("bucket-copy");

  std::cout << "copy from one object to another object in one bucket." << std::endl;
  auto cp = client.copyObject(bucket, "object-put-1", "object-cp-1-1234");
  if (!cp.isSuccess()) {
    std::cout << "copy obj error: " << cp.error().String() << std::endl;
    return;
  }
  std::cout << "copy object success, the copied object etag is: "
            << cp.result().getEtag() << std::endl;
  auto head = client.headObject(bucket, "object-cp-1-1234");
  if (!head.isSuccess()) {
    std::cout << "head obj error: " << head.error().String() << std::endl;
    return;
  }
  std::cout << "head copied object success, the object etag is: "
            << head.result().getObjectMeta().getEtags() << std::endl;


  std::cout << "copy one object from source bucket to the destination bucket." << std::endl;
  auto cpTo = client.copyObjectTo(bucket, dstBkt, "object-cp-2-1234", "object-put-1");
  if (!cpTo.isSuccess()) {
    std::cout << "copy to obj error: " << cpTo.error().String() << std::endl;
    return;
  }
  std::cout << "copy object to destination success, the copied object etag is: "
            << cpTo.result().getEtag() << std::endl;
  auto headTo = client.headObject(dstBkt, "object-cp-2-1234");
  if (!headTo.isSuccess()){
    std::cout << "head obj error: " << headTo.error().String() << std::endl;
    return;
  }
  std::cout << "head destination object success, the object etag is: "
            << headTo.result().getObjectMeta().getEtags() << std::endl;


  std::cout << "copy one object to a source bucket from a destination bucket." << std::endl;
  auto cpFrom = client.copyObjectFrom(bucket, dstBkt, "object-cp-2-1234", "object-put-2");
  if (!cpFrom.isSuccess()) {
    std::cout << "copy from obj error: " << cp.error().String() << std::endl;
    return;
  }
  std::cout << "copy object from destination success, the copied object etag is: "
            << cpFrom.result().getEtag() << std::endl;
  auto headFrom = client.headObject(bucket, "object-put-2");
  if (!headFrom.isSuccess()){
    std::cout << "head obj error: " << headFrom.error().String() << std::endl;
    return;
  }
  std::cout << "head destination object success, the object etag is: "
            << headFrom.result().getObjectMeta().getEtags() << std::endl;
}

void uploadPartCopy(TosClient& client, const std::string & bucket, const std::string &fileName){
  auto fs = std::make_shared<std::fstream>(fileName, std::ios::in | std::ios_base::binary);
  std::string key("object-upload-part-copy-"+ std::to_string(std::time(nullptr)));
  std::string dstKey("objectUploadPartCopy.data");
  auto srcPut = client.putObject(bucket, key, fs);
  if (!srcPut.isSuccess()) {
    std::cout << "put obj error: " << srcPut.error().String() << std::endl;
    return;
  }
  auto upload = client.createMultipartUpload(bucket, dstKey);
  if (!upload.isSuccess()) {
    std::cout << "createMultipartUpload error: " << upload.error().String() << std::endl;
    return;
  }
  // 获取要copy的对象大小
  auto head = client.headObject(bucket, key);
  if (!head.isSuccess()) {
    std::cout << "head object error: " << head.error().String() << std::endl;
    return;
  }
  int64_t size = head.result().getObjectMeta().getContentLength();
  std::cout << "object size is " << size << std::endl;
  // 分片大小5MB为例
  int partSize = 5 * 1024 * 1024; // 10MB
  int partCount = (int) (size / partSize);
  UploadPartCopyOutput copyParts[partCount];
  for (int i = 0; i < partCount; ++i) {
    int64_t partLen = partSize;
    if (partCount == i + 1 && (size % partLen) > 0) {
      partLen += size % (int64_t) partSize;
    }
    int64_t startOffset = (int64_t) i * partSize;
    UploadPartCopyInput input;
    input.setUploadId(upload.result().getUploadId());
    input.setDestinationKey(dstKey);
    input.setSourceBucket(bucket);
    input.setStartOffset(startOffset);
    input.setPartSize(partLen);
    input.setPartNumber(i+1);
    input.setSourceKey(key);
    auto res = client.uploadPartCopy(bucket, input);
    std::cout << "upload part " << i+1 << "'s etag is " << res.result().getEtag() << std::endl;
    if (!res.isSuccess()){
      std::cout << "upload part " << i << "failed, error is: "
                << res.error().String() << std::endl;
      return;
    }
    copyParts[i] = res.result();
  }
  std::vector<UploadPartCopyOutput> parts(copyParts, copyParts + sizeof(copyParts) / sizeof(UploadPartCopyOutput));
  CompleteMultipartCopyUploadInput input(dstKey, upload.result().getUploadId(), parts);
  auto complete = client.completeMultipartUpload(bucket, input);
  if (!complete.isSuccess()) {
    std::cout << "completeMultipartUpload error: " << complete.error().String() << std::endl;
    return;
  }
  std::cout << "completeMultipartUpload success, the request id is: "
            << complete.result().getRequestInfo().getRequestId() << std::endl;
}

void uploadPart(TosClient& client, const std::string & bucket, const std::string & key) {
  auto upload = client.createMultipartUpload(bucket, key);
  if (!upload.isSuccess()) {
    std::cout << "createMultipartUpload error: " << upload.error().String() << std::endl;
    return;
  }
  std::cout << "createMultipartUpload succeed, begin to upload part" << std::endl;
  // generate some data..
  auto ss1 = std::make_shared<std::stringstream>();
  for (int i = 0; i < (5 << 20); ++i) {
    *ss1 << 1;
  }
  UploadPartInput input1(key, upload.result().getUploadId(), ss1->tellg(), 1,ss1);
  auto part1 = client.uploadPart(bucket, input1);
  if (!part1.isSuccess()) {
    std::cout << "uploadPart error: " << part1.error().String() << std::endl;
    return;
  }

  std::cout << "uploadPart 1 succeed, begin to upload part 2" << std::endl;
  // generate some data..
  auto ss2 = std::make_shared<std::stringstream>();
  for (int i = 0; i < (4 << 20); ++i) {
    *ss2 << 2;
  }
  UploadPartInput input2(key, upload.result().getUploadId(), ss2->tellg(), 2,ss2);
  auto part2 = client.uploadPart(bucket, input2);
  if (!part2.isSuccess()) {
    std::cout << "uploadPart error: " << part2.error().String() << std::endl;
    return;
  }

  std::cout << "uploadPart 2 succeed, begin to completeMultipartUpload" << std::endl;
  CompleteMultipartUploadInput input(key, upload.result().getUploadId(), {part1.result(), part2.result()});
  auto com = client.completeMultipartUpload(bucket, input);
  if (!com.isSuccess()){
    std::cout << "completeMultipartUpload error: " << com.error().String() << std::endl;
    return;
  }
  std::cout << "completeMultipartUpload success: " << com.result().getRequestInfo().getRequestId() << std::endl;
}

void uploadPartAbort(TosClient& client, const std::string & bucket, const std::string & key) {
  auto upload = client.createMultipartUpload(bucket, key);
  if (!upload.isSuccess()) {
    std::cout << "createMultipartUpload error: " << upload.error().String() << std::endl;
    return;
  }
  AbortMultipartUploadInput input(key, upload.result().getUploadId());
  auto abort = client.abortMultipartUpload(bucket, input);
  if (!abort.isSuccess()) {
    std::cout << abort.error().String() << std::endl;
    return;
  }
  std::cout << abort.result().getRequestInfo().getRequestId() << std::endl;
}

void listMultipart(TosClient& client, const std::string & bucket){
  ListMultipartUploadsInput input;
  input.setMaxUploads(100);
  auto res = client.listMultipartUploads(bucket, input);
  if (!res.isSuccess()) {
    std::cout << res.error().String() << std::endl;
    return;
  }
  for (const auto & i : res.result().getUpload()) {
    std::cout << "multipart key is: " << i.getKey() << std::endl;
  }
}

void listUploadedParts(TosClient& client, const std::string & bucket, const std::string & key) {
  auto upload = client.createMultipartUpload(bucket, key);
  if (!upload.isSuccess()) {
    std::cout << "createMultipartUpload error: " << upload.error().String() << std::endl;
    return;
  }
  // generate some data..
  auto ss1 = std::make_shared<std::stringstream>();
  for (int i = 0; i < (1 << 20); ++i) {
    *ss1 << 1;
  }
  UploadPartInput input1(key, upload.result().getUploadId(), ss1->tellg(), 1,ss1);
  auto part1 = client.uploadPart(bucket, input1);
  if (!part1.isSuccess()) {
    std::cout << "uploadPart error: " << part1.error().String() << std::endl;
    return;
  }

  // generate some data..
  auto ss2 = std::make_shared<std::stringstream>();
  for (int i = 0; i < (1 << 20); ++i) {
    *ss2 << 2;
  }
  UploadPartInput input2(key, upload.result().getUploadId(), ss2->tellg(), 2,ss2);
  auto part2 = client.uploadPart(bucket, input2);
  if (!part2.isSuccess()) {
    std::cout << "uploadPart error: " << part2.error().String() << std::endl;
    return;
  }
  ListUploadedPartsInput input;
  input.setKey(key);
  input.setUploadId(upload.result().getUploadId());
  input.setMaxParts(100);
  auto res = client.listUploadedParts(bucket, input);
  if (!res.isSuccess()) {
    std::cout << res.error().String() << std::endl;
    return;
  }
  for (const auto & i : res.result().getUploadedParts()) {
    std::cout << "uploaded part number is: " << i.getPartNumber() << std::endl;
    std::cout << "uploaded part size is: " << i.getSize() << std::endl;
    std::cout << "uploaded part etag is: " << i.getEtag() << std::endl;
  }
  AbortMultipartUploadInput abortInput(key, upload.result().getUploadId());
  auto abort = client.abortMultipartUpload(bucket, abortInput);
  if (!abort.isSuccess()) {
    std::cout << abort.error().String() << std::endl;
    return;
  }
  std::cout << abort.result().getRequestInfo().getRequestId() << std::endl;
}

void deleteMultiObjects(TosClient & client, const std::string & bucket){
  std::vector<ObjectTobeDeleted> otds(2);
  ObjectTobeDeleted otd1;
  otd1.setKey("中文测试1644921863583");
  otds[0] = otd1;
  ObjectTobeDeleted otd2;
  otd2.setKey("中文测试1644921884327");
  otds[1] = otd2;
  DeleteMultiObjectsInput input;
  input.setQuiet(true);
  input.setObjectTobeDeleteds(otds);
  auto output = client.deleteMultiObjects(bucket, input);
  if (!output.isSuccess()){
    std::cout << output.error().String() << std::endl;
    return;
  }
  std::cout << "deleteMultiObjects success, the req id is: "
            << output.result().getRequestInfo().getRequestId() << std::endl;
}

void preSignedUrl(TosClient& client, const std::string & bucket) {
  std::string data = "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'";
  auto ss = std::make_shared<std::stringstream>(data);
  std::string key("object-put-1");
  auto output = client.putObject(bucket, key, ss);
  if (!output.isSuccess()) {
    std::cout << output.error().String() << std::endl;
    return;
  }
  std::chrono::duration<int, std::ratio<100>> hs(3);
  auto res = client.preSignedURL("Get", bucket, key, hs);
  if (!res.isSuccess()) {
    std::cout << "preSignedUrl error: " << output.error().String() << std::endl;
    return;
  }
  std::cout << "the preSigned url is: " << res.result() << std::endl;
}

int main(){
  std::string endpoint("your endpoint");
  std::string region("your region");
  std::string ak("Your Access Key");
  std::string sk("Your Secret Key");
  std::string bucket("your bucket");
  std::string key("your object key");
  InitializeClient();
  TosClient client(endpoint, region, ak, sk);
  putObject(client, bucket, key);
  getObject(client, bucket, key);
  CloseClient();
}
