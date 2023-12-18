#pragma once

#include <string>

namespace VolcengineTos {
class TransportConfig {
public:
    int getMaxIdleCount() const {
        return maxIdleCount_;
    }
    void setMaxIdleCount(int maxIdleCount) {
        maxIdleCount_ = maxIdleCount;
    }
    int getRequestTimeout() const {
        return requestTimeout_;
    }
    void setRequestTimeout(int requestTimeout) {
        requestTimeout_ = requestTimeout;
    }
    int getDialTimeout() const {
        return dialTimeout_;
    }
    void setDialTimeout(int dialTimeout) {
        dialTimeout_ = dialTimeout;
    }
    int getKeepAlive() const {
        return keepAlive_;
    }
    void setKeepAlive(int keepAlive) {
        keepAlive_ = keepAlive;
    }
    int getConnectTimeout() const {
        return connectTimeout_;
    }
    void setConnectTimeout(int connectTimeout) {
        connectTimeout_ = connectTimeout;
    }
    int getTlsHandshakeTimeout() const {
        return tlsHandshakeTimeout_;
    }
    void setTlsHandshakeTimeout(int tlsHandshakeTimeout) {
        tlsHandshakeTimeout_ = tlsHandshakeTimeout;
    }
    int getResponseHeaderTimeout() const {
        return responseHeaderTimeout_;
    }
    void setResponseHeaderTimeout(int responseHeaderTimeout) {
        responseHeaderTimeout_ = responseHeaderTimeout;
    }
    int getExpectContinueTimeout() const {
        return expectContinueTimeout_;
    }
    void setExpectContinueTimeout(int expectContinueTimeout) {
        expectContinueTimeout_ = expectContinueTimeout;
    }
    int getReadTimeout() const {
        return readTimeout_;
    }
    void setReadTimeout(int readTimeout) {
        readTimeout_ = readTimeout;
    }
    int getWriteTimeout() const {
        return writeTimeout_;
    }
    void setWriteTimeout(int writeTimeout) {
        writeTimeout_ = writeTimeout;
    }
    bool isEnableVerifySsl() const {
        return enableVerifySSL_;
    }
    void setEnableVerifySsl(bool enableVerifySsl) {
        enableVerifySSL_ = enableVerifySsl;
    }
    const std::string& getProxyHost() const {
        return proxyHost_;
    }
    void setProxyHost(const std::string& proxyHost) {
        proxyHost_ = proxyHost;
    }
    int getProxyPort() const {
        return proxyPort_;
    }
    void setProxyPort(int proxyPort) {
        proxyPort_ = proxyPort;
    }
    const std::string& getProxyUsername() const {
        return proxyUsername_;
    }
    void setProxyUsername(const std::string& proxyUsername) {
        proxyUsername_ = proxyUsername;
    }
    const std::string& getProxyPassword() const {
        return proxyPassword_;
    }
    void setProxyPassword(const std::string& proxyPassword) {
        proxyPassword_ = proxyPassword;
    }
    int getDnsCacheTime() const {
        return dnsCacheTime_;
    }
    void setDnsCacheTime(int dnsCacheTime) {
        dnsCacheTime_ = dnsCacheTime;
    }
    int getMaxConnections() const {
        return maxConnections_;
    }
    void setMaxConnections(int maxConnections) {
        maxConnections_ = maxConnections;
    }
    int getSocketTimeout() const {
        return socketTimeout_;
    }
    void setSocketTimeout(int socketTimeout) {
        socketTimeout_ = socketTimeout;
    }
    const std::string& getCaPath() const {
        return caPath_;
    }
    void setCaPath(const std::string& caPath) {
        caPath_ = caPath;
    }
    const std::string& getCaFile() const {
        return caFile_;
    }
    void setCaFile(const std::string& caFile) {
        caFile_ = caFile;
    }
    int getHighLatencyLogThreshold() const {
        return highLatencyLogThreshold_;
    }
    void setHighLatencyLogThreshold(int highLatencyLogThreshold) {
        highLatencyLogThreshold_ = highLatencyLogThreshold;
    }

private:
    int maxIdleCount_ = 128;
    int requestTimeout_ = 120000;
    int dialTimeout_ = 10;
    int keepAlive_ = 30;
    int connectTimeout_ = 10000;
    int tlsHandshakeTimeout_ = 10;
    int responseHeaderTimeout_ = 60;
    int expectContinueTimeout_ = 3;
    int readTimeout_ = 120;
    int writeTimeout_ = 120;
    bool enableVerifySSL_ = true;
    std::string proxyHost_;
    int proxyPort_ = -1;
    std::string proxyUsername_;
    std::string proxyPassword_;
    int dnsCacheTime_ = 0;
    int maxConnections_ = 25;
    int socketTimeout_ = 30000;
    std::string caPath_;
    std::string caFile_;
    int highLatencyLogThreshold_ = 100;
};
}  // namespace VolcengineTos
