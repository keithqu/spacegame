#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <csignal>
#include "galaxy.h"
#include "http_server.h"

// Global server instance for signal handling
space4x::SimpleHttpServer* global_server = nullptr;

void signalHandler(int signum) {
    std::cout << "\n🛑 Received signal " << signum << ", shutting down..." << std::endl;
    if (global_server) {
        global_server->stop();
    }
    exit(signum);
}

void runAsService() {
    std::cout << "🎮 Space 4X Game Engine starting in service mode..." << std::endl;
    
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    space4x::SimpleHttpServer server(3002);
    global_server = &server;
    
    if (!server.start()) {
        std::cerr << "❌ Failed to start HTTP server" << std::endl;
        return;
    }
    
    std::cout << "🌟 Game Engine HTTP server running on port 3002" << std::endl;
    std::cout << "🔄 Ready to process galaxy generation requests..." << std::endl;
    
    // Run the server
    server.run();
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--mode" && argc > 2 && std::string(argv[2]) == "service") {
        runAsService();
    } else {
        std::cout << "🎮 Space 4X Game Engine" << std::endl;
        std::cout << "Usage: " << argv[0] << " --mode service" << std::endl;
        std::cout << "Or run without arguments for single execution" << std::endl;
    }
    
    return 0;
}
