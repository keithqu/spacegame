#include "http_server.h"
#include "galaxy.h"
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <ctime>

namespace space4x {

SimpleHttpServer::SimpleHttpServer(int p) : port(p), server_fd(-1), running(false) {}

SimpleHttpServer::~SimpleHttpServer() {
    stop();
}

bool SimpleHttpServer::start() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        std::cerr << "❌ Socket creation failed" << std::endl;
        return false;
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "❌ Setsockopt failed" << std::endl;
        return false;
    }
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "❌ Bind failed on port " << port << std::endl;
        return false;
    }
    
    if (listen(server_fd, 3) < 0) {
        std::cerr << "❌ Listen failed" << std::endl;
        return false;
    }
    
    running = true;
    std::cout << "🌐 HTTP Server started on port " << port << std::endl;
    return true;
}

void SimpleHttpServer::stop() {
    running = false;
    if (server_fd != -1) {
        close(server_fd);
        server_fd = -1;
    }
}

void SimpleHttpServer::run() {
    while (running) {
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        
        if (new_socket < 0) {
            if (running) {
                std::cerr << "❌ Accept failed" << std::endl;
            }
            continue;
        }
        
        // Handle request in a separate thread
        std::thread([this, new_socket]() {
            handleRequest(new_socket);
            close(new_socket);
        }).detach();
    }
}

void SimpleHttpServer::handleRequest(int socket) {
    char buffer[4096] = {0};
    read(socket, buffer, 4096);
    
    std::string request(buffer);
    std::string response;
    
    if (request.find("POST /generate-galaxy") != std::string::npos) {
        response = handleGalaxyGeneration(request);
    } else if (request.find("GET /system/") != std::string::npos) {
        response = handleSystemDetails(request);
    } else if (request.find("GET /health") != std::string::npos) {
        response = handleHealthCheck();
    } else {
        response = handleNotFound();
    }
    
    send(socket, response.c_str(), response.length(), 0);
}

std::string SimpleHttpServer::handleGalaxyGeneration(const std::string& request) {
    try {
        // Extract JSON from POST body
        size_t body_start = request.find("\r\n\r\n");
        if (body_start == std::string::npos) {
            return createErrorResponse(400, "No request body found");
        }
        
        std::string json_body = request.substr(body_start + 4);
        
        // Parse config
        GalaxyConfig config = parseSimpleGalaxyConfig(json_body);
        
        // Generate galaxy
        GalaxyGenerator generator(config);
        Galaxy galaxy = generator.generateGalaxy();
        
        // Store the current galaxy for system lookups
        currentGalaxy = galaxy;
        
        // Convert to JSON
        std::string json_response = galaxyToSimpleJson(galaxy);
        
        return createJsonResponse(json_response);
    } catch (const std::exception& e) {
        return createErrorResponse(500, std::string("Galaxy generation failed: ") + e.what());
    }
}

std::string SimpleHttpServer::handleHealthCheck() {
    std::string json = R"({"status":"healthy","service":"space4x-engine","timestamp":")" + 
                      std::to_string(std::time(nullptr)) + R"("})";
    return createJsonResponse(json);
}

std::string SimpleHttpServer::handleNotFound() {
    return createErrorResponse(404, "Endpoint not found");
}

std::string SimpleHttpServer::createJsonResponse(const std::string& json) {
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type\r\n";
    response << "Content-Length: " << json.length() << "\r\n";
    response << "\r\n";
    response << json;
    return response.str();
}

std::string SimpleHttpServer::createErrorResponse(int code, const std::string& message) {
    std::string json = R"({"error":")" + message + R"("})";
    std::ostringstream response;
    response << "HTTP/1.1 " << code << " Error\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Content-Length: " << json.length() << "\r\n";
    response << "\r\n";
    response << json;
    return response.str();
}

GalaxyConfig SimpleHttpServer::parseSimpleGalaxyConfig(const std::string& json) {
    GalaxyConfig config;
    
    // Simple JSON parsing
    config.seed = extractIntValue(json, "seed", 42);
    config.radius = extractDoubleValue(json, "radius", 500.0);
    
    // Scale system and anomaly counts with galaxy area (radius²) to maintain consistent density
    // Base density: ~400 systems in 500 LY radius = ~0.00032 systems per LY²
    double baseRadius = 500.0;
    double baseSystems = 400.0;
    double baseAnomalies = 25.0;
    
    double areaScalingFactor = (config.radius * config.radius) / (baseRadius * baseRadius);
    
    // Allow override from JSON, but default to scaled values
    int requestedSystems = extractIntValue(json, "starSystemCount", -1);
    int requestedAnomalies = extractIntValue(json, "anomalyCount", -1);
    
    if (requestedSystems > 0) {
        config.starSystemCount = requestedSystems;  // Use explicit request
    } else {
        config.starSystemCount = static_cast<int>(baseSystems * areaScalingFactor);  // Scale with area
    }
    
    if (requestedAnomalies > 0) {
        config.anomalyCount = requestedAnomalies;  // Use explicit request
    } else {
        config.anomalyCount = static_cast<int>(baseAnomalies * areaScalingFactor);  // Scale with area
    }
    
    std::cout << "🌌 Galaxy scaling: radius=" << config.radius 
             << " LY, systems=" << config.starSystemCount 
             << ", anomalies=" << config.anomalyCount 
             << " (area factor: " << areaScalingFactor << ")" << std::endl;
    config.minDistance = extractDoubleValue(json, "minDistance", 2.0);
    
    // Set default fixed systems with both real and fictional stars
    config.fixedSystems = {
        // Real star systems with accurate positions
        {"sol", "Sol System", 0.0, 0.0, "origin", true, 0.0, 0.0},
        {"alpha-centauri", "Alpha Centauri", 4.37, 0.0, "core", true, 0.0, 0.0},
        {"tau-ceti", "Tau Ceti", -7.8, 9.1, "core", true, 0.0, 0.0},  // 11.9 LY from Sol
        {"barnards-star", "Barnard's Star", 2.1, -5.6, "core", true, 0.0, 0.0},  // 5.96 LY from Sol
        {"bellatrix", "Bellatrix", 180.0, 165.0, "core", true, 0.0, 0.0},  // ~245 LY from Sol (core: ≤300 LY)
        
        // Fictional star systems with distance constraints
        {"lumiere", "Lumière", 0.0, 0.0, "core", false, 250.0, 20.0},  // 250 ± 20 LY (core: ≤300 LY)
        {"aspida", "Aspida", 0.0, 0.0, "rim", false, 350.0, 20.0}     // 350 ± 20 LY
    };
    
    // Parse connectivity settings from nested JSON object or use defaults
    std::string connectivitySection = extractJsonSection(json, "connectivity");
    if (!connectivitySection.empty()) {
        config.connectivity.minConnections = extractIntValue(connectivitySection, "minConnections", 2);
        config.connectivity.maxConnections = extractIntValue(connectivitySection, "maxConnections", 5);
        config.connectivity.maxDistance = extractDoubleValue(connectivitySection, "maxDistance", 12.0);
        config.connectivity.distanceDecayFactor = extractDoubleValue(connectivitySection, "distanceDecayFactor", 0.3);
        config.connectivity.useVoronoiConnectivity = extractBoolValue(connectivitySection, "useVoronoiConnectivity", true);
        
    } else {
        // Use defaults if connectivity section not found
        config.connectivity = {2, 5, 12.0, 0.3, true};
    }
    
    // Set default visualization
    config.visualization = {1200, 800, 12.0};
    
    return config;
}

int SimpleHttpServer::extractIntValue(const std::string& json, const std::string& key, int defaultValue) {
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return defaultValue;
    
    pos += search.length();
    size_t end = json.find_first_of(",}", pos);
    if (end == std::string::npos) return defaultValue;
    
    try {
        return std::stoi(json.substr(pos, end - pos));
    } catch (...) {
        return defaultValue;
    }
}

double SimpleHttpServer::extractDoubleValue(const std::string& json, const std::string& key, double defaultValue) {
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return defaultValue;
    
    pos += search.length();
    size_t end = json.find_first_of(",}", pos);
    if (end == std::string::npos) return defaultValue;
    
    try {
        return std::stod(json.substr(pos, end - pos));
    } catch (...) {
        return defaultValue;
    }
}

bool SimpleHttpServer::extractBoolValue(const std::string& json, const std::string& key, bool defaultValue) {
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return defaultValue;
    
    pos += search.length();
    while (pos < json.length() && std::isspace(json[pos])) pos++;
    
    if (pos + 4 <= json.length() && json.substr(pos, 4) == "true") return true;
    if (pos + 5 <= json.length() && json.substr(pos, 5) == "false") return false;
    
    return defaultValue;
}

std::string SimpleHttpServer::extractJsonSection(const std::string& json, const std::string& sectionName) {
    std::string search = "\"" + sectionName + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    
    pos += search.length();
    while (pos < json.length() && std::isspace(json[pos])) pos++;
    
    if (pos >= json.length() || json[pos] != '{') return "";
    
    // Find matching closing brace
    int braceCount = 1;
    size_t start = pos;
    pos++; // Skip opening brace
    
    while (pos < json.length() && braceCount > 0) {
        if (json[pos] == '{') braceCount++;
        else if (json[pos] == '}') braceCount--;
        pos++;
    }
    
    if (braceCount == 0) {
        return json.substr(start, pos - start);
    }
    
    return "";
}

std::string SimpleHttpServer::galaxyToSimpleJson(const Galaxy& galaxy) {
    std::ostringstream json;
    json << "{";
    json << "\"config\":{";
    json << "\"seed\":" << galaxy.config.seed << ",";
    json << "\"radius\":" << galaxy.config.radius << ",";
    json << "\"starSystemCount\":" << galaxy.config.starSystemCount << ",";
    json << "\"anomalyCount\":" << galaxy.config.anomalyCount;
    json << "},";
    
    // Systems
    json << "\"systems\":[";
    for (size_t i = 0; i < galaxy.systems.size(); i++) {
        if (i > 0) json << ",";
        const auto& sys = galaxy.systems[i];
        json << "{";
        json << "\"id\":\"" << sys.id << "\",";
        json << "\"name\":\"" << sys.name << "\",";
        json << "\"x\":" << sys.x << ",";
        json << "\"y\":" << sys.y << ",";
        json << "\"type\":\"" << sys.type << "\",";
        json << "\"isFixed\":" << (sys.isFixed ? "true" : "false") << ",";
        json << "\"explored\":" << (sys.explored ? "true" : "false") << ",";
        json << "\"population\":" << sys.population << ",";
        json << "\"gdp\":" << sys.gdp << ",";
        json << "\"connections\":[";
        for (size_t j = 0; j < sys.connections.size(); j++) {
            if (j > 0) json << ",";
            json << "\"" << sys.connections[j] << "\"";
        }
        json << "],";
        json << "\"resources\":{";
        json << "\"minerals\":" << sys.resources.minerals << ",";
        json << "\"energy\":" << sys.resources.energy << ",";
        json << "\"research\":" << sys.resources.research;
        json << "},";
        json << "\"systemInfo\":{";
        json << "\"starType\":\"" << sys.systemInfo.starType << "\",";
        json << "\"planetCount\":" << sys.systemInfo.planetCount << ",";
        json << "\"moonCount\":" << sys.systemInfo.moonCount << ",";
        json << "\"asteroidCount\":" << sys.systemInfo.asteroidCount;
        json << "},";
        json << "\"hasDetailedData\":true";  // All systems now have detailed data available
        json << "}";
    }
    json << "],";
    
    // Anomalies
    json << "\"anomalies\":[";
    for (size_t i = 0; i < galaxy.anomalies.size(); i++) {
        if (i > 0) json << ",";
        const auto& anom = galaxy.anomalies[i];
        json << "{";
        json << "\"id\":\"" << anom.id << "\",";
        json << "\"name\":\"" << anom.name << "\",";
        json << "\"x\":" << anom.x << ",";
        json << "\"y\":" << anom.y << ",";
        json << "\"type\":\"" << anom.type << "\",";
        json << "\"discovered\":" << (anom.discovered ? "true" : "false") << ",";
        json << "\"effect\":{";
        json << "\"type\":\"" << anom.effect.type << "\",";
        json << "\"value\":" << anom.effect.value;
        json << "}";
        json << "}";
    }
    json << "],";
    
    // Warp lanes
    json << "\"warpLanes\":[";
    for (size_t i = 0; i < galaxy.warpLanes.size(); i++) {
        if (i > 0) json << ",";
        const auto& lane = galaxy.warpLanes[i];
        json << "{";
        json << "\"id\":\"" << lane.id << "\",";
        json << "\"from\":\"" << lane.from << "\",";
        json << "\"to\":\"" << lane.to << "\",";
        json << "\"distance\":" << lane.distance << ",";
        json << "\"travelTime\":" << lane.travelTime << ",";
        json << "\"discovered\":" << (lane.discovered ? "true" : "false");
        json << "}";
    }
    json << "],";
    
    // Bounds
    json << "\"bounds\":{";
    json << "\"minX\":" << galaxy.bounds.minX << ",";
    json << "\"maxX\":" << galaxy.bounds.maxX << ",";
    json << "\"minY\":" << galaxy.bounds.minY << ",";
    json << "\"maxY\":" << galaxy.bounds.maxY << ",";
    json << "\"radius\":" << galaxy.bounds.radius;
    json << "}";
    
    json << "}";
    return json.str();
}

std::string SimpleHttpServer::handleSystemDetails(const std::string& request) {
    // Extract system ID from URL path
    size_t systemPos = request.find("GET /system/");
    if (systemPos == std::string::npos) {
        return createErrorResponse("Invalid system request");
    }
    
    size_t idStart = systemPos + 12; // Length of "GET /system/"
    size_t idEnd = request.find(" ", idStart);
    if (idEnd == std::string::npos) {
        return createErrorResponse("Invalid system ID");
    }
    
    std::string systemId = request.substr(idStart, idEnd - idStart);
    
    // First, try to get predefined system definition
    SystemConfigManager configManager;
    const SystemDefinition* systemDef = configManager.getSystemDefinition(systemId);
    
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
    SystemDefinition generatedSystem = configManager.generateRandomSystem(systemId, galaxySystem->name);
    
    // Override with galaxy system info
    generatedSystem.systemId = galaxySystem->id;
    generatedSystem.systemName = galaxySystem->name;
    generatedSystem.starType = galaxySystem->systemInfo.starType;
    
    // Serialize the generated system
    return serializeSystemDefinition(generatedSystem);
}

std::string SimpleHttpServer::serializeSystemDefinition(const SystemDefinition& systemDef) {
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

std::string SimpleHttpServer::createErrorResponse(const std::string& message) {
    std::ostringstream response;
    response << "HTTP/1.1 400 Bad Request\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type\r\n";
    response << "\r\n";
    response << "{\"error\":\"" << message << "\"}";
    return response.str();
}

} // namespace space4x