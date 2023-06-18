#include "TosClientV2.h"
#include <string>
#include <fstream>
#include <memory>
#include <cstring>
namespace VolcengineTos {
class TestUtils {
public:
    static std::string GetBucketName(const std::string& prefix);

    static std::string GetObjectKey(const std::string& prefix);

    static void CreateBucket(const std::shared_ptr<TosClientV2>& client, const std::string& name);

    static void PutObject(const std::shared_ptr<TosClientV2>& client, const std::string& bkt_name,
                          const std::string& obj_name, const std::string& data);

    static void PutObjectWithMeta(const std::shared_ptr<TosClientV2>& client, const std::string& bkt_name,
                                  const std::string& obj_name, const std::string& data,
                                  const std::map<std::string, std::string>& meta);

    static std::string GetObjectContentByStream(const std::shared_ptr<TosClientV2>& client, const std::string& bkt_name,
                                                const std::string& obj_name);

    static std::map<std::string, std::string> GetObjectMeta(const std::shared_ptr<TosClientV2>& client,
                                                            const std::string& bkt_name, const std::string& obj_name);

    static void CleanBucket(const std::shared_ptr<TosClientV2>& client, const std::string& bkt_name);

    static void CleanAllBucket(const std::shared_ptr<TosClientV2>& client);

    static std::string GetRandomString(int length);
    static void GetRandomCharArray(int length, unsigned char* array);
    static void WriteRandomDatatoFile(const std::string& file, int length);
    static time_t GetTimeWithDelay(int64_t delay);
};
}  // namespace VolcengineTos
