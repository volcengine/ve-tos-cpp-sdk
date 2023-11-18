#include <TosClientV2.h>
#include <photon/photon.h>
#include <photon/thread/std-compat.h>

using namespace VolcengineTos;

int main() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    photon::init(photon::INIT_EVENT_EPOLL, photon::INIT_IO_LIBCURL);
    DEFER(photon::fini());

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 在当前线程开启多个协程，并发读取文件
    std::vector<photon_std::thread*> threads;
    for (int i = 0; i < 8; ++i) {
        auto th = new photon_std::thread([&] {
            GetObjectV2Input input(bucketName, objectName);
            const int length = 1048576;
            input.setRangeStart(i * length);
            input.setRangeEnd(i * length + length - 1);

            auto output = client.getObject(input);
            if (!output.isSuccess()) {
                std::cout << "GetObject failed." << output.error().String() << std::endl;
                CloseClient();
                return -1;
            }

            auto stream = output.result().getContent();
            char streamBuffer[length];
            while (stream->good()) {
                stream->read(streamBuffer, length);
            }
            return 0;
        });
        threads.push_back(th);
    }

    // 等待所有协程执行完毕
    for (auto th : threads) {
        th->join();
        delete th;
    }

    CloseClient();
}