#pragma once

#include <string>
#include <memory>
#include <libpq-fe.h>
#include "galaxy.h"
#include "http_server.h"

namespace space4x {

class BackendServer {
private:
    int port;
    int server_fd;
    bool running;
    
    // Database connection
    PGconn* db_connection;
    std::string db_host;
    std::string db_name;
    std::string db_user;
    std::string db_password;
    int db_port;
    
    // Game engine components
    Galaxy currentGalaxy;
    SystemConfigManager systemConfigManager;
    
    // HTTP request handling
    std::string handleRequest(const std::string& request);
    std::string handleHealthCheck();
    std::string handleGalaxyGenerate(const std::string& request);
    std::string handleGalaxyHealth();
    std::string handleSystemDetails(const std::string& request);
    std::string handleGameState();
    std::string handleGameAction(const std::string& request);
    std::string handleApiTest();
    
    // Database operations
    bool connectToDatabase();
    void disconnectFromDatabase();
    std::string testDatabaseConnection();
    
    // HTTP utilities
    std::string createJsonResponse(const std::string& json);
    std::string createErrorResponse(int status, const std::string& message);
    std::string createErrorResponse(const std::string& message);
    std::string createCorsResponse();
    std::string extractPath(const std::string& request);
    std::string extractMethod(const std::string& request);
    std::string extractBody(const std::string& request);
    std::string serializeSystemDefinition(const SystemDefinition& systemDef);
    
    // CORS support
    std::string addCorsHeaders(const std::string& response);

public:
    BackendServer(int port = 3001);
    ~BackendServer();
    
    bool start();
    void stop();
    void run();
    
    // Configuration
    void setDatabaseConfig(const std::string& host, const std::string& name, 
                          const std::string& user, const std::string& password, int port = 5432);
};

} // namespace space4x
