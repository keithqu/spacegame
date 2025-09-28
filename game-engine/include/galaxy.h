#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <random>

namespace space4x {

struct StarSystem {
    std::string id;
    std::string name;
    double x, y;  // Position in light years
    std::string type;  // "origin", "major", "minor", "frontier"
    bool isFixed;
    std::vector<std::string> connections;  // Connected system IDs
    bool explored;
    int population;
    struct {
        int minerals;
        int energy;
        int research;
    } resources;
};

struct Anomaly {
    std::string id;
    std::string name;
    double x, y;  // Position in light years
    std::string type;  // "nebula", "blackhole", "wormhole", "artifact", "resource"
    bool discovered;
    struct {
        std::string type;
        double value;
    } effect;
};

struct WarpLane {
    std::string id;
    std::string from;
    std::string to;
    double distance;
    int travelTime;
    bool discovered;
};

struct FixedSystem {
    std::string id;
    std::string name;
    double x, y;
    std::string type;
};

struct GalaxyConfig {
    int seed;
    double radius;  // Light years
    int starSystemCount;
    int anomalyCount;
    
    std::vector<FixedSystem> fixedSystems;
    
    struct {
        int minConnections;
        int maxConnections;
        double maxDistance;
        double distanceDecayFactor;
    } connectivity;
    
    struct {
        int width;
        int height;
        double scale;
    } visualization;
};

struct Galaxy {
    GalaxyConfig config;
    std::vector<StarSystem> systems;
    std::vector<Anomaly> anomalies;
    std::vector<WarpLane> warpLanes;
    struct {
        double minX, maxX, minY, maxY, radius;
    } bounds;
};

class SeededRandom {
private:
    std::mt19937 generator;
    std::uniform_real_distribution<double> distribution;

public:
    SeededRandom(int seed) : generator(seed), distribution(0.0, 1.0) {}
    
    double next() { return distribution(generator); }
    double range(double min, double max) { return min + next() * (max - min); }
    int intRange(int min, int max) { return min + static_cast<int>(next() * (max - min + 1)); }
    bool boolean(double probability = 0.5) { return next() < probability; }
};

class GalaxyGenerator {
private:
    GalaxyConfig config;
    SeededRandom random;
    
    std::vector<StarSystem> generateStarSystems();
    std::vector<Anomaly> generateAnomalies(const std::vector<StarSystem>& systems);
    std::vector<WarpLane> generateWarpLanes(std::vector<StarSystem>& systems);
    
    std::pair<double, double> generateRandomPositionInCircle();
    bool isPositionTooCloseToSystems(const std::pair<double, double>& pos, 
                                   const std::vector<StarSystem>& systems, 
                                   double minDistance);
    bool isPositionTooClose(const std::pair<double, double>& pos,
                          const std::vector<Anomaly>& anomalies,
                          double minDistance);
    double calculateDistance(const std::pair<double, double>& a, 
                           const std::pair<double, double>& b);
    
    void createWarpLane(StarSystem& system1, StarSystem& system2, double distance,
                       std::vector<WarpLane>& warpLanes,
                       std::unordered_map<std::string, std::vector<std::string>>& connections);
    void ensureMinimumConnectivity(std::vector<StarSystem>& systems,
                                 std::vector<WarpLane>& warpLanes,
                                 std::unordered_map<std::string, std::vector<std::string>>& connections);
    void ensureNetworkConnectivity(std::vector<StarSystem>& systems,
                                 std::vector<WarpLane>& warpLanes,
                                 std::unordered_map<std::string, std::vector<std::string>>& connections);
    
    std::string generateSystemName(int index);
    std::string generateAnomalyName(const std::string& type, int index);
    std::string determineSystemType(const std::pair<double, double>& position);
    std::string generateAnomalyType();

public:
    GalaxyGenerator(const GalaxyConfig& cfg);
    Galaxy generateGalaxy();
};

// JSON serialization functions
std::string galaxyToJson(const Galaxy& galaxy);
GalaxyConfig parseGalaxyConfig(const std::string& json);

} // namespace space4x
