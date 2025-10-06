#include "backend_server.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <regex>
#include <cstdlib>

namespace space4x {

BackendServer::BackendServer(int port) 
    : port(port), server_fd(-1), running(false), db_connection(nullptr),
      db_host("localhost"), db_name("space4x_game"), db_user("space4x_user"), 
      db_password(""), db_port(5432) {
}

BackendServer::~BackendServer() {
    stop();
    disconnectFromDatabase();
}

bool BackendServer::start() {
    // Connect to database first (optional when SPACE4X_SKIP_DB is set)
    const char* skipDbEnv = std::getenv("SPACE4X_SKIP_DB");
    bool skipDb = (skipDbEnv != nullptr && std::string(skipDbEnv) == "1");

    if (!connectToDatabase()) {
        if (skipDb) {
            std::cerr << "⚠️  Database connection failed; continuing because SPACE4X_SKIP_DB=1" << std::endl;
        } else {
            std::cerr << "❌ Failed to connect to database" << std::endl;
            return false;
        }
    }
    
    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "❌ Failed to create socket" << std::endl;
        return false;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "❌ Failed to set socket options" << std::endl;
        close(server_fd);
        return false;
    }
    
    // Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "❌ Bind failed on port " << port << std::endl;
        close(server_fd);
        return false;
    }
    
    // Listen for connections
    if (listen(server_fd, 10) < 0) {
        std::cerr << "❌ Failed to listen on socket" << std::endl;
        close(server_fd);
        return false;
    }
    
    running = true;
    std::cout << "🚀 Space 4X Backend server running on port " << port << std::endl;
    std::cout << "📊 Health check available at http://localhost:" << port << "/health" << std::endl;
    std::cout << "🌌 Galaxy API available at http://localhost:" << port << "/api/galaxy/generate" << std::endl;
    
    return true;
}

void BackendServer::stop() {
    running = false;
    if (server_fd >= 0) {
        close(server_fd);
        server_fd = -1;
    }
    std::cout << "🛑 Backend server stopped" << std::endl;
}

void BackendServer::run() {
    while (running) {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_address, &client_len);
        if (client_fd < 0) {
            if (running) {
                std::cerr << "❌ Failed to accept connection" << std::endl;
            }
            continue;
        }
        
        // Read request
        char buffer[4096];
        std::string request;
        int bytes_read;
        
        while ((bytes_read = read(client_fd, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            request += buffer;
            
            // Check if we have a complete HTTP request
            if (request.find("\r\n\r\n") != std::string::npos) {
                break;
            }
        }
        
        if (bytes_read < 0) {
            std::cerr << "❌ Failed to read request" << std::endl;
            close(client_fd);
            continue;
        }
        
        // Process request
        std::string response = handleRequest(request);
        
        // Send response
        if (write(client_fd, response.c_str(), response.length()) < 0) {
            std::cerr << "❌ Failed to send response" << std::endl;
        }
        
        close(client_fd);
    }
}

std::string BackendServer::handleRequest(const std::string& request) {
    std::string method = extractMethod(request);
    std::string path = extractPath(request);
    std::string body = extractBody(request);
    
    std::cout << "📨 " << method << " " << path << std::endl;
    
    // Handle CORS preflight requests
    if (method == "OPTIONS") {
        return createCorsResponse();
    }
    
    // Route requests
    if (path == "/health") {
        return handleHealthCheck();
    } else if (path == "/api/test") {
        return handleApiTest();
    } else if (path == "/api/user/current" && method == "GET") {
        return handleGetCurrentUser();
    } else if (path == "/api/galaxy/generate" && method == "POST") {
        return handleGalaxyGenerate(body);
    } else if (path == "/api/galaxy/health") {
        return handleGalaxyHealth();
    } else if (path.find("/api/system/") == 0 && method == "GET") {
        return handleSystemDetails(request);
    } else if (path == "/api/game/state") {
        return handleGameState();
    } else if (path == "/api/game/action" && method == "POST") {
        return handleGameAction(body);
    } else if (path == "/api/saves" && method == "GET") {
        return handleGetSaves();
    } else if (path == "/api/saves" && method == "POST") {
        return handleSaveGame(body);
    } else if (path.find("/api/saves/") == 0 && method == "GET") {
        return handleLoadGame(request);
    } else {
        return createErrorResponse(404, "Route not found");
    }
}

std::string BackendServer::handleHealthCheck() {
    std::string dbStatus = testDatabaseConnection();
    
    std::ostringstream json;
    json << "{";
    json << "\"status\":\"healthy\",";
    json << "\"timestamp\":\"" << std::time(nullptr) << "\",";
    json << "\"database\":\"" << (dbStatus.empty() ? "connected" : "disconnected") << "\"";
    if (!dbStatus.empty()) {
        json << ",\"error\":\"" << dbStatus << "\"";
    }
    json << "}";
    
    return createJsonResponse(json.str());
}

std::string BackendServer::handleApiTest() {
    std::ostringstream json;
    json << "{\"message\":\"Space 4X Backend API is running!\"}";
    return createJsonResponse(json.str());
}

std::string BackendServer::handleGetCurrentUser() {
    if (!db_connection) {
        return createErrorResponse(500, "Database connection not available");
    }
    
    // For now, return the default "keith" user
    // In a real implementation, this would validate session/auth tokens
    const char* query = "SELECT id, username, email, membership FROM users WHERE username = 'keith' LIMIT 1";
    PGresult* result = PQexec(db_connection, query);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::string error = PQresultErrorMessage(result);
        PQclear(result);
        return createErrorResponse(500, "Database query failed: " + error);
    }
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return createErrorResponse(404, "User not found");
    }
    
    // Extract user data
    std::string id = PQgetvalue(result, 0, 0);
    std::string username = PQgetvalue(result, 0, 1);
    std::string email = PQgetvalue(result, 0, 2);
    std::string membership = PQgetvalue(result, 0, 3);
    
    PQclear(result);
    
    // Build JSON response
    std::ostringstream json;
    json << "{";
    json << "\"id\":\"" << id << "\",";
    json << "\"username\":\"" << username << "\",";
    json << "\"email\":\"" << email << "\",";
    json << "\"membership\":" << (membership.empty() ? "null" : "\"" + membership + "\"");
    json << "}";
    
    return createJsonResponse(json.str());
}

std::string BackendServer::handleGalaxyGenerate(const std::string& request) {
    try {
        std::cout << "🌌 Received galaxy generation request" << std::endl;
        
        // Parse request body for parameters
        int radius = 500;
        int systems = 400;
        int anomalies = 25;
        long seed = 1111111111;
        int saveSlot = 1;
        bool useSavedFlag = false;
        bool radiusProvided = false, systemsProvided = false, anomaliesProvided = false, seedProvided = false;
        
        // Simple JSON parsing for parameters
        std::regex radiusRegex(R"("radius":\s*(\d+))");
        std::regex systemsRegex(R"("systems":\s*(\d+))");
        std::regex anomaliesRegex(R"("anomalies":\s*(\d+))");
        std::regex seedRegex(R"("seed":\s*(\d+))");
        std::regex slotRegex(R"("save_slot":\s*(\d+))");
        std::regex useSavedRegex(R"("use_saved":\s*(true|false))");
        
        std::smatch match;
        if (std::regex_search(request, match, radiusRegex)) {
            radius = std::stoi(match[1].str());
            radiusProvided = true;
        }
        if (std::regex_search(request, match, systemsRegex)) {
            systems = std::stoi(match[1].str());
            systemsProvided = true;
        }
        if (std::regex_search(request, match, anomaliesRegex)) {
            anomalies = std::stoi(match[1].str());
            anomaliesProvided = true;
        }
        if (std::regex_search(request, match, seedRegex)) {
            seed = std::stol(match[1].str());
            seedProvided = true;
        }
        if (std::regex_search(request, match, slotRegex)) {
            saveSlot = std::stoi(match[1].str());
        }
        if (std::regex_search(request, match, useSavedRegex)) {
            useSavedFlag = (match[1].str() == "true");
        }
        bool anyParamsProvided = radiusProvided || systemsProvided || anomaliesProvided || seedProvided;

        // Decide behavior: if use_saved is true, try load; otherwise if params provided, generate; else try load then generate
        if (useSavedFlag) {
            bool found = false;
            std::string savedJson = loadSavedStateForUser("keith", saveSlot, found);
            if (found && !savedJson.empty()) {
                std::cout << "💾 Loaded existing saved galaxy for user keith (slot " << saveSlot << ")" << std::endl;
                return createJsonResponse(savedJson);
            }
            // If requested use_saved but none found, fall through to generation
        } else if (!useSavedFlag && !anyParamsProvided) {
            bool found = false;
            std::string savedJson = loadSavedStateForUser("keith", saveSlot, found);
            if (found && !savedJson.empty()) {
                std::cout << "💾 Loaded existing saved galaxy for user keith (slot " << saveSlot << ")" << std::endl;
                return createJsonResponse(savedJson);
            }
        }
        
        // Generate galaxy using existing game engine
        GalaxyConfig config;
        config.seed = seed;
        config.radius = radius;
        config.starSystemCount = systems;
        config.anomalyCount = anomalies;
        config.minDistance = 2.0;
        config.connectivity.minConnections = 1;
        config.connectivity.maxConnections = 8;
        config.connectivity.maxDistance = 10.0;
        config.connectivity.distanceDecayFactor = 0.8;
        config.connectivity.useVoronoiConnectivity = true;
        config.visualization.width = 2000;
        config.visualization.height = 2000;
        config.visualization.scale = 6.0;
        
        // Add fixed systems
        config.fixedSystems = {
            {"sol", "Sol System", 0.0, 0.0, "origin", true},
            {"alpha-centauri", "Alpha Centauri", 4.37, 0.0, "core", true},
            {"tau-ceti", "Tau Ceti", -7.8, 9.1, "core", true},
            {"barnards-star", "Barnard's Star", 2.1, -5.6, "core", true},
            {"bellatrix", "Bellatrix", 180.0, 165.0, "rim", true},
            {"lumiere", "Lumière", 0.0, 0.0, "rim", false, 250.0, 20.0},
            {"aspida", "Aspida", 0.0, 0.0, "rim", false, 350.0, 20.0}
        };
        
        GalaxyGenerator generator(config);
        currentGalaxy = generator.generateGalaxy();
        
        // Convert to JSON using existing method
        std::ostringstream json;
        json << "{";
        json << "\"config\":{";
        json << "\"radius\":" << currentGalaxy.config.radius << ",";
        json << "\"systems\":" << currentGalaxy.config.starSystemCount << ",";
        json << "\"anomalies\":" << currentGalaxy.config.anomalyCount << ",";
        json << "\"seed\":" << currentGalaxy.config.seed;
        json << "},";
        json << "\"visualization\":{";
        json << "\"width\":" << currentGalaxy.config.visualization.width << ",";
        json << "\"height\":" << currentGalaxy.config.visualization.height << ",";
        json << "\"scale\":" << currentGalaxy.config.visualization.scale;
        json << "},";
        json << "\"systems\":[";
        
        for (size_t i = 0; i < currentGalaxy.systems.size(); i++) {
            if (i > 0) json << ",";
            const auto& system = currentGalaxy.systems[i];
            json << "{";
            json << "\"id\":\"" << system.id << "\",";
            json << "\"name\":\"" << system.name << "\",";
            json << "\"x\":" << system.x << ",";
            json << "\"y\":" << system.y << ",";
            json << "\"type\":\"" << system.type << "\",";
            json << "\"isFixed\":" << (system.isFixed ? "true" : "false") << ",";
            json << "\"explored\":true,";
            
            // Build connections array for this system
            json << "\"connections\":[";
            bool firstConnection = true;
            for (const auto& lane : currentGalaxy.warpLanes) {
                if (lane.from == system.id || lane.to == system.id) {
                    if (!firstConnection) json << ",";
                    std::string connectedId = (lane.from == system.id) ? lane.to : lane.from;
                    json << "\"" << connectedId << "\"";
                    firstConnection = false;
                }
            }
            json << "],";
            
            // Add system info
            json << "\"systemInfo\":{";
            json << "\"starType\":\"" << system.systemInfo.starType << "\",";
            json << "\"planetCount\":" << system.systemInfo.planetCount << ",";
            json << "\"moonCount\":" << system.systemInfo.moonCount << ",";
            json << "\"asteroidCount\":" << system.systemInfo.asteroidCount;
            json << "},";
            
            // Only mark systems as having detailed data if a predefined definition exists
            bool hasDetailed = (system.detailedSystem != nullptr);
            json << "\"hasDetailedData\":" << (hasDetailed ? "true" : "false");
            json << "}";
        }
        
        json << "],\"anomalies\":[";
        for (size_t i = 0; i < currentGalaxy.anomalies.size(); i++) {
            if (i > 0) json << ",";
            const auto& anomaly = currentGalaxy.anomalies[i];
            json << "{";
            json << "\"id\":\"" << anomaly.id << "\",";
            json << "\"name\":\"" << anomaly.name << "\",";
            json << "\"x\":" << anomaly.x << ",";
            json << "\"y\":" << anomaly.y << ",";
            json << "\"type\":\"" << anomaly.type << "\"";
            json << "}";
        }
        
        json << "],\"warpLanes\":[";
        for (size_t i = 0; i < currentGalaxy.warpLanes.size(); i++) {
            if (i > 0) json << ",";
            const auto& lane = currentGalaxy.warpLanes[i];
            json << "{";
            json << "\"from\":\"" << lane.from << "\",";
            json << "\"to\":\"" << lane.to << "\",";
            json << "\"distance\":" << lane.distance;
            json << "}";
        }
        json << "]";
        json << "}";
        
        std::cout << "✅ Galaxy generated successfully" << std::endl;
        
        // Persist generated state
        std::string error;
        if (!upsertSavedStateForUser("keith", saveSlot, json.str(), error)) {
            std::cerr << "⚠️  Failed to persist save: " << error << std::endl;
        } else {
            std::cout << "💾 Saved galaxy to DB for user keith (slot " << saveSlot << ")" << std::endl;
        }
        
        return createJsonResponse(json.str());
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Galaxy generation failed: " << e.what() << std::endl;
        return createErrorResponse(500, std::string("Galaxy generation failed: ") + e.what());
    }
}

std::string BackendServer::handleGalaxyHealth() {
    std::ostringstream json;
    json << "{";
    json << "\"status\":\"healthy\",";
    json << "\"engine\":\"operational\",";
    json << "\"proxy\":\"operational\"";
    json << "}";
    return createJsonResponse(json.str());
}

std::string BackendServer::handleSystemDetails(const std::string& request) {
    // Extract system ID from path
    std::regex pathRegex(R"(/api/system/([^/\s]+))");
    std::smatch match;
    
    if (!std::regex_search(request, match, pathRegex)) {
        return createErrorResponse(400, "Invalid system ID");
    }
    
    std::string systemId = match[1].str();
    
    // First, try to get predefined system definition
    const SystemDefinition* systemDef = systemConfigManager.getSystemDefinition(systemId);
    
    if (systemDef) {
        // Found predefined system - serialize it
        return serializeSystemDefinition(*systemDef);
    }
    
    // Not a predefined system - look up in current galaxy
    if (currentGalaxy.systems.empty()) {
        return createErrorResponse("No galaxy data available. Generate a galaxy first.");
    }
    
    // Find the system in the current galaxy
    const StarSystem* galaxySystem = nullptr;
    for (const auto& sys : currentGalaxy.systems) {
        if (sys.id == systemId) {
            galaxySystem = &sys;
            break;
        }
    }
    
    if (!galaxySystem) {
        return createErrorResponse("System not found in current galaxy");
    }
    
    // Generate detailed system data for this procedural system
    SystemDefinition generatedSystem = systemConfigManager.generateRandomSystem(systemId, galaxySystem->name);
    
    // Override with galaxy system info
    generatedSystem.systemId = galaxySystem->id;
    generatedSystem.systemName = galaxySystem->name;
    generatedSystem.starType = galaxySystem->systemInfo.starType;
    
    // Serialize the generated system
    return serializeSystemDefinition(generatedSystem);
}

std::string BackendServer::handleGameState() {
    bool found = false;
    std::string savedJson = loadSavedStateForUser("keith", 1, found);
    if (found && !savedJson.empty()) {
        return createJsonResponse(savedJson);
    }
    return createErrorResponse(404, "No saved game state for user");
}

std::string BackendServer::handleGameAction(const std::string& request) {
    std::ostringstream json;
    json << "{";
    json << "\"message\":\"Game action endpoint - to be implemented\",";
    json << "\"action\":" << request;
    json << "}";
    return createJsonResponse(json.str());
}

std::string BackendServer::handleGetSaves() {
    if (!db_connection) {
        return createErrorResponse(500, "Database connection not available");
    }
    
    // For now, get saves for the default "keith" user
    // In a real implementation, this would validate session/auth tokens
    const char* query = "SELECT s.id, s.save_slot, s.save_data, s.created_at, s.updated_at FROM saves s JOIN users u ON s.user_id = u.id WHERE u.username = 'keith' ORDER BY s.save_slot";
    PGresult* result = PQexec(db_connection, query);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::string error = PQresultErrorMessage(result);
        PQclear(result);
        return createErrorResponse(500, "Database query failed: " + error);
    }
    
    int numRows = PQntuples(result);
    std::ostringstream json;
    json << "{\"saves\":[";
    
    for (int i = 0; i < numRows; i++) {
        if (i > 0) json << ",";
        json << "{";
        json << "\"id\":\"" << PQgetvalue(result, i, 0) << "\",";
        json << "\"save_slot\":" << PQgetvalue(result, i, 1) << ",";
        json << "\"save_data\":" << PQgetvalue(result, i, 2) << ",";
        json << "\"created_at\":\"" << PQgetvalue(result, i, 3) << "\",";
        json << "\"updated_at\":\"" << PQgetvalue(result, i, 4) << "\"";
        json << "}";
    }
    
    json << "]}";
    PQclear(result);
    
    return createJsonResponse(json.str());
}

std::string BackendServer::handleSaveGame(const std::string& request) {
    if (!db_connection) {
        return createErrorResponse(500, "Database connection not available");
    }
    
    // Parse minimal fields from request body
    int saveSlot = 1;
    std::regex slotRegex(R"("save_slot":\s*(\d+))");
    std::smatch match;
    if (std::regex_search(request, match, slotRegex)) {
        saveSlot = std::stoi(match[1].str());
    }
    
    // Everything after the body is treated as the saved state JSON
    std::string saveJson = request;
    
    std::string error;
    if (!upsertSavedStateForUser("keith", saveSlot, saveJson, error)) {
        return createErrorResponse(500, std::string("Failed to save: ") + error);
    }
    
    std::ostringstream resp;
    resp << "{";
    resp << "\"status\":\"saved\",";
    resp << "\"save_slot\":" << saveSlot;
    resp << "}";
    return createJsonResponse(resp.str());
}
std::string BackendServer::loadSavedStateForUser(const std::string& username, int slot, bool& found) {
    found = false;
    if (!db_connection) {
        return "";
    }
    const char* query = "SELECT s.save_data FROM saves s JOIN users u ON s.user_id = u.id WHERE u.username = $1 AND s.save_slot = $2 LIMIT 1";
    const char* paramValues[] = { username.c_str(), std::to_string(slot).c_str() };
    int paramLengths[] = { static_cast<int>(username.length()), static_cast<int>(std::to_string(slot).length()) };
    int paramFormats[] = { 0, 0 };
    PGresult* result = PQexecParams(db_connection, query, 2, nullptr, paramValues, paramLengths, paramFormats, 0);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::string error = PQresultErrorMessage(result);
        std::cerr << "❌ Load save failed: " << error << std::endl;
        PQclear(result);
        return "";
    }
    if (PQntuples(result) == 0) {
        PQclear(result);
        return "";
    }
    found = true;
    std::string json = PQgetvalue(result, 0, 0);
    PQclear(result);
    return json;
}

bool BackendServer::upsertSavedStateForUser(const std::string& username, int slot, const std::string& saveJson, std::string& errorOut) {
    if (!db_connection) {
        errorOut = "No database connection";
        return false;
    }
    
    // Upsert using CTE to get user id
    const char* query =
        "WITH u AS (SELECT id FROM users WHERE username = $1),\n"
        "ins AS (\n"
        "  INSERT INTO saves (user_id, save_slot, save_data)\n"
        "  SELECT u.id, $2::int, $3::jsonb FROM u\n"
        "  ON CONFLICT (user_id, save_slot) DO UPDATE SET save_data = $3::jsonb, updated_at = NOW()\n"
        "  RETURNING id\n"
        ") SELECT id FROM ins";
    
    std::string slotStr = std::to_string(slot);
    const char* paramValues[] = { username.c_str(), slotStr.c_str(), saveJson.c_str() };
    int paramLengths[] = { static_cast<int>(username.length()), static_cast<int>(slotStr.length()), static_cast<int>(saveJson.length()) };
    int paramFormats[] = { 0, 0, 0 };
    PGresult* result = PQexecParams(db_connection, query, 3, nullptr, paramValues, paramLengths, paramFormats, 0);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        errorOut = PQresultErrorMessage(result);
        PQclear(result);
        return false;
    }
    PQclear(result);
    return true;
}

std::string BackendServer::handleLoadGame(const std::string& request) {
    if (!db_connection) {
        return createErrorResponse(500, "Database connection not available");
    }
    
    // Extract save ID from path
    std::regex pathRegex(R"(/api/saves/([^/\s]+))");
    std::smatch match;
    
    if (!std::regex_search(request, match, pathRegex)) {
        return createErrorResponse(400, "Invalid save ID");
    }
    
    std::string saveId = match[1].str();
    
    // For now, load save for the default "keith" user
    // In a real implementation, this would validate session/auth tokens
    const char* query = "SELECT s.save_data FROM saves s JOIN users u ON s.user_id = u.id WHERE s.id = $1 AND u.username = 'keith'";
    const char* paramValues[] = { saveId.c_str() };
    int paramLengths[] = { static_cast<int>(saveId.length()) };
    int paramFormats[] = { 0 }; // text format
    
    PGresult* result = PQexecParams(db_connection, query, 1, nullptr, paramValues, paramLengths, paramFormats, 0);
    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::string error = PQresultErrorMessage(result);
        PQclear(result);
        return createErrorResponse(500, "Database query failed: " + error);
    }
    
    if (PQntuples(result) == 0) {
        PQclear(result);
        return createErrorResponse(404, "Save not found");
    }
    
    std::string saveData = PQgetvalue(result, 0, 0);
    PQclear(result);
    
    return createJsonResponse(saveData);
}

bool BackendServer::connectToDatabase() {
    std::ostringstream connStr;
    connStr << "host=" << db_host 
            << " port=" << db_port 
            << " dbname=" << db_name 
            << " user=" << db_user 
            << " password=" << db_password;
    
    db_connection = PQconnectdb(connStr.str().c_str());
    
    if (PQstatus(db_connection) != CONNECTION_OK) {
        std::cerr << "❌ Database connection failed: " << PQerrorMessage(db_connection) << std::endl;
        PQfinish(db_connection);
        db_connection = nullptr;
        return false;
    }
    
    std::cout << "✅ Connected to PostgreSQL database" << std::endl;
    return true;
}

void BackendServer::disconnectFromDatabase() {
    if (db_connection) {
        PQfinish(db_connection);
        db_connection = nullptr;
    }
}

std::string BackendServer::testDatabaseConnection() {
    if (!db_connection) {
        return "No database connection";
    }
    
    PGresult* result = PQexec(db_connection, "SELECT NOW()");
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::string error = PQresultErrorMessage(result);
        PQclear(result);
        return error;
    }
    
    PQclear(result);
    return ""; // Empty string means success
}

void BackendServer::setDatabaseConfig(const std::string& host, const std::string& name, 
                                     const std::string& user, const std::string& password, int port) {
    db_host = host;
    db_name = name;
    db_user = user;
    db_password = password;
    db_port = port;
}

std::string BackendServer::createJsonResponse(const std::string& json) {
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    response << "Content-Length: " << json.length() << "\r\n";
    response << "\r\n";
    response << json;
    return response.str();
}

std::string BackendServer::createErrorResponse(int status, const std::string& message) {
    std::ostringstream json;
    json << "{\"error\":\"" << message << "\"}";
    
    std::ostringstream response;
    response << "HTTP/1.1 " << status << " Error\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    response << "Content-Length: " << json.str().length() << "\r\n";
    response << "\r\n";
    response << json.str();
    return response.str();
}

std::string BackendServer::createErrorResponse(const std::string& message) {
    return createErrorResponse(500, message);
}

std::string BackendServer::createCorsResponse() {
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    response << "Access-Control-Max-Age: 86400\r\n";
    response << "Content-Length: 0\r\n";
    response << "\r\n";
    return response.str();
}

std::string BackendServer::extractPath(const std::string& request) {
    std::istringstream stream(request);
    std::string method, path, version;
    stream >> method >> path >> version;
    return path;
}

std::string BackendServer::extractMethod(const std::string& request) {
    std::istringstream stream(request);
    std::string method;
    stream >> method;
    return method;
}

std::string BackendServer::extractBody(const std::string& request) {
    size_t bodyStart = request.find("\r\n\r\n");
    if (bodyStart != std::string::npos) {
        return request.substr(bodyStart + 4);
    }
    return "";
}

std::string BackendServer::serializeSystemDefinition(const SystemDefinition& systemDef) {
    std::ostringstream json;
    json << "{";
    json << "\"systemId\":\"" << systemDef.systemId << "\",";
    json << "\"systemName\":\"" << systemDef.systemName << "\",";
    json << "\"starType\":\"" << systemDef.starType << "\",";
    json << "\"starMass\":" << systemDef.starMass << ",";
    json << "\"starRadius\":" << systemDef.starRadius << ",";
    json << "\"starTemperature\":" << systemDef.starTemperature << ",";
    
    // Serialize planets
    json << "\"planets\":[";
    for (size_t i = 0; i < systemDef.planets.size(); i++) {
        if (i > 0) json << ",";
        const auto& planet = systemDef.planets[i];
        
        json << "{";
        json << "\"id\":\"" << planet.id << "\",";
        json << "\"name\":\"" << planet.name << "\",";
        json << "\"type\":\"" << planet.type << "\",";
        json << "\"distanceFromStar\":" << planet.distanceFromParent << ",";
        json << "\"radius\":" << planet.radius << ",";
        json << "\"diameter\":" << planet.diameter << ",";
        json << "\"mass\":" << planet.mass << ",";
        json << "\"gravity\":" << planet.gravity << ",";
        json << "\"habitability\":" << planet.habitability << ",";
        json << "\"atmosphere\":\"" << planet.atmosphere << "\",";
        json << "\"composition\":\"" << planet.composition << "\",";
        
        // Serialize resources
        json << "\"resources\":[";
        for (size_t j = 0; j < planet.resources.size(); j++) {
            if (j > 0) json << ",";
            const auto& resource = planet.resources[j];
            json << "{";
            json << "\"type\":" << static_cast<int>(resource.type) << ",";
            json << "\"abundance\":" << resource.abundance << ",";
            json << "\"accessibility\":" << resource.accessibility;
            json << "}";
        }
        json << "],";
        
        // Serialize moons
        json << "\"moons\":[";
        for (size_t j = 0; j < planet.moons.size(); j++) {
            if (j > 0) json << ",";
            const auto& moon = planet.moons[j];
            
            json << "{";
            json << "\"id\":\"" << moon.id << "\",";
            json << "\"name\":\"" << moon.name << "\",";
            json << "\"type\":\"" << moon.type << "\",";
            json << "\"distanceFromPlanet\":" << moon.distanceFromParent << ",";
            json << "\"radius\":" << moon.radius << ",";
            json << "\"diameter\":" << moon.diameter << ",";
            json << "\"mass\":" << moon.mass << ",";
            json << "\"gravity\":" << moon.gravity << ",";
            json << "\"habitability\":" << moon.habitability << ",";
            json << "\"atmosphere\":\"" << moon.atmosphere << "\",";
            json << "\"composition\":\"" << moon.composition << "\",";
            
            // Moon resources
            json << "\"resources\":[";
            for (size_t k = 0; k < moon.resources.size(); k++) {
                if (k > 0) json << ",";
                const auto& resource = moon.resources[k];
                json << "{";
                json << "\"type\":" << static_cast<int>(resource.type) << ",";
                json << "\"abundance\":" << resource.abundance << ",";
                json << "\"accessibility\":" << resource.accessibility;
                json << "}";
            }
            json << "]";
            json << "}";
        }
        json << "]";
        json << "}";
    }
    json << "],";
    
    // Serialize asteroids
    json << "\"asteroids\":[";
    for (size_t i = 0; i < systemDef.asteroids.size(); i++) {
        if (i > 0) json << ",";
        const auto& asteroid = systemDef.asteroids[i];
        
        json << "{";
        json << "\"id\":\"" << asteroid.id << "\",";
        json << "\"name\":\"" << asteroid.name << "\",";
        json << "\"type\":\"" << asteroid.type << "\",";
        json << "\"distanceFromStar\":" << asteroid.distanceFromParent << ",";
        json << "\"radius\":" << asteroid.radius << ",";
        json << "\"diameter\":" << asteroid.diameter << ",";
        json << "\"mass\":" << asteroid.mass << ",";
        json << "\"gravity\":" << asteroid.gravity << ",";
        json << "\"habitability\":" << asteroid.habitability << ",";
        json << "\"atmosphere\":\"" << asteroid.atmosphere << "\",";
        json << "\"composition\":\"" << asteroid.composition << "\",";
        
        // Asteroid resources
        json << "\"resources\":[";
        for (size_t j = 0; j < asteroid.resources.size(); j++) {
            if (j > 0) json << ",";
            const auto& resource = asteroid.resources[j];
            json << "{";
            json << "\"type\":" << static_cast<int>(resource.type) << ",";
            json << "\"abundance\":" << resource.abundance << ",";
            json << "\"accessibility\":" << resource.accessibility;
            json << "}";
        }
        json << "]";
        json << "}";
    }
    json << "]";
    
    json << "}";
    
    return createJsonResponse(json.str());
}

} // namespace space4x
