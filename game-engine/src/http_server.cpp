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
    config.starSystemCount = extractIntValue(json, "starSystemCount", 350);
    config.anomalyCount = extractIntValue(json, "anomalyCount", 50);
    
    // Set default fixed systems
    config.fixedSystems = {
        {"sol", "Sol System", 0.0, 0.0, "origin"},
        {"alpha-centauri", "Alpha Centauri", 4.37, 0.0, "major"},
        {"tau-ceti", "Tau Ceti", 8.5, 7.2, "major"}
    };
    
    // Set default connectivity
    config.connectivity = {1, 3, 8.0, 0.5};
    
    // Set default visualization
    config.visualization = {1200, 800, 3.0};
    
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
        json << "}";
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

} // namespace space4x