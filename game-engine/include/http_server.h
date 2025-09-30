#pragma once

#include <string>

namespace space4x {

class SimpleHttpServer {
private:
    int port;
    int server_fd;
    bool running;

public:
    SimpleHttpServer(int p);
    ~SimpleHttpServer();
    
    bool start();
    void stop();
    void run();
    
private:
    void handleRequest(int socket);
    std::string handleGalaxyGeneration(const std::string& request);
    std::string handleSystemDetails(const std::string& request);
    std::string handleHealthCheck();
    std::string handleNotFound();
    std::string createJsonResponse(const std::string& json);
    std::string createErrorResponse(const std::string& message);
    std::string createErrorResponse(int code, const std::string& message);
    
    // Simple JSON parsing helpers
    int extractIntValue(const std::string& json, const std::string& key, int defaultValue);
    double extractDoubleValue(const std::string& json, const std::string& key, double defaultValue);
    bool extractBoolValue(const std::string& json, const std::string& key, bool defaultValue);
    std::string extractJsonSection(const std::string& json, const std::string& sectionName);
    std::string galaxyToSimpleJson(const struct Galaxy& galaxy);
    struct GalaxyConfig parseSimpleGalaxyConfig(const std::string& json);
};

} // namespace space4x
