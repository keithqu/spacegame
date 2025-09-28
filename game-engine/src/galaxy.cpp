#include "galaxy.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace space4x {

GalaxyGenerator::GalaxyGenerator(const GalaxyConfig& cfg) 
    : config(cfg), random(cfg.seed) {
}

Galaxy GalaxyGenerator::generateGalaxy() {
    std::cout << "🌌 Generating galaxy with seed: " << config.seed << std::endl;
    
    // Generate star systems
    auto systems = generateStarSystems();
    
    // Generate anomalies
    auto anomalies = generateAnomalies(systems);
    
    // Generate warp lanes
    auto warpLanes = generateWarpLanes(systems);
    
    Galaxy galaxy;
    galaxy.config = config;
    galaxy.systems = systems;
    galaxy.anomalies = anomalies;
    galaxy.warpLanes = warpLanes;
    galaxy.bounds = {-config.radius, config.radius, -config.radius, config.radius, config.radius};
    
    // Calculate statistics
    double avgConnections = 0;
    for (const auto& system : systems) {
        avgConnections += system.connections.size();
    }
    avgConnections /= systems.size();
    
    double maxDistance = 0;
    double avgDistance = 0;
    for (const auto& lane : warpLanes) {
        maxDistance = std::max(maxDistance, lane.distance);
        avgDistance += lane.distance;
    }
    if (!warpLanes.empty()) {
        avgDistance /= warpLanes.size();
    }
    
    std::cout << "✅ Generated galaxy: " << systems.size() << " systems, " 
              << anomalies.size() << " anomalies, " << warpLanes.size() << " warp lanes" << std::endl;
    std::cout << "📊 Connectivity: " << std::fixed << std::setprecision(1) 
              << avgConnections << " avg connections, " << maxDistance << " max distance, " 
              << avgDistance << " avg distance" << std::endl;
    
    return galaxy;
}

std::vector<StarSystem> GalaxyGenerator::generateStarSystems() {
    std::vector<StarSystem> systems;
    
    // Add fixed systems first
    for (const auto& fixedSystem : config.fixedSystems) {
        StarSystem system;
        system.id = fixedSystem.id;
        system.name = fixedSystem.name;
        system.x = fixedSystem.x;
        system.y = fixedSystem.y;
        system.type = fixedSystem.type;
        system.isFixed = true;
        system.explored = (fixedSystem.type == "origin");
        system.population = (fixedSystem.type == "origin") ? 1000000 : 0;
        system.resources.minerals = random.intRange(50, 200);
        system.resources.energy = random.intRange(50, 200);
        system.resources.research = random.intRange(50, 200);
        systems.push_back(system);
    }
    
    // Generate remaining random systems
    int remainingSystems = config.starSystemCount - config.fixedSystems.size();
    
    for (int i = 0; i < remainingSystems; i++) {
        std::pair<double, double> position;
        int attempts = 0;
        
        // Generate position within circle, avoiding conflicts
        do {
            position = generateRandomPositionInCircle();
            attempts++;
        } while (attempts < 100 && 
                isPositionTooCloseToSystems(position, systems, 2.0));
        
        StarSystem system;
        system.id = "system-" + std::to_string(i + 1);
        system.name = generateSystemName(i + 1);
        system.x = position.first;
        system.y = position.second;
        system.type = determineSystemType(position);
        system.isFixed = false;
        system.explored = false;
        system.population = 0;
        system.resources.minerals = random.intRange(10, 150);
        system.resources.energy = random.intRange(10, 150);
        system.resources.research = random.intRange(10, 150);
        systems.push_back(system);
    }
    
    return systems;
}

std::vector<Anomaly> GalaxyGenerator::generateAnomalies(const std::vector<StarSystem>& systems) {
    std::vector<Anomaly> anomalies;
    
    for (int i = 0; i < config.anomalyCount; i++) {
        std::pair<double, double> position;
        int attempts = 0;
        
        // Generate position within circle, avoiding systems and other anomalies
        do {
            position = generateRandomPositionInCircle();
            attempts++;
        } while (attempts < 100 && 
                (isPositionTooCloseToSystems(position, systems, 3.0) ||
                 isPositionTooClose(position, anomalies, 2.0)));
        
        Anomaly anomaly;
        anomaly.id = "anomaly-" + std::to_string(i + 1);
        anomaly.type = generateAnomalyType();
        anomaly.name = generateAnomalyName(anomaly.type, i + 1);
        anomaly.x = position.first;
        anomaly.y = position.second;
        anomaly.discovered = false;
        
        // Set effect based on type
        if (anomaly.type == "nebula") {
            anomaly.effect = {"sensor_interference", -0.5};
        } else if (anomaly.type == "blackhole") {
            anomaly.effect = {"gravity_well", 2.0};
        } else if (anomaly.type == "wormhole") {
            anomaly.effect = {"fast_travel", 0.1};
        } else if (anomaly.type == "artifact") {
            anomaly.effect = {"research_bonus", 1.5};
        } else if (anomaly.type == "resource") {
            anomaly.effect = {"mining_bonus", 2.0};
        } else {
            anomaly.effect = {"none", 0.0};
        }
        
        anomalies.push_back(anomaly);
    }
    
    return anomalies;
}

std::vector<WarpLane> GalaxyGenerator::generateWarpLanes(std::vector<StarSystem>& systems) {
    std::vector<WarpLane> warpLanes;
    std::unordered_map<std::string, std::vector<std::string>> connections;
    
    // Initialize connection tracking
    for (auto& system : systems) {
        connections[system.id] = std::vector<std::string>();
    }
    
    // Phase 1: Create initial connections based on proximity
    for (auto& system : systems) {
        auto& currentConnections = connections[system.id];
        
        // Find candidates within max distance
        std::vector<std::pair<StarSystem*, double>> candidates;
        for (auto& other : systems) {
            if (other.id != system.id) {
                double distance = calculateDistance({system.x, system.y}, {other.x, other.y});
                if (distance <= config.connectivity.maxDistance) {
                    candidates.push_back({&other, distance});
                }
            }
        }
        
        // Sort by distance
        std::sort(candidates.begin(), candidates.end(), 
                 [](const auto& a, const auto& b) { return a.second < b.second; });
        
        // Determine target connections
        int targetConnections = random.intRange(config.connectivity.minConnections, 
                                              config.connectivity.maxConnections);
        
        // Add connections with distance-based probability
        for (const auto& candidate : candidates) {
            if (static_cast<int>(currentConnections.size()) >= targetConnections) break;
            
            // Check if already connected
            auto& otherConnections = connections[candidate.first->id];
            if (std::find(currentConnections.begin(), currentConnections.end(), 
                         candidate.first->id) != currentConnections.end()) {
                continue;
            }
            
            // Distance-based probability with penalty for long distances
            double normalizedDistance = candidate.second / config.connectivity.maxDistance;
            double probability = std::exp(-normalizedDistance * config.connectivity.distanceDecayFactor);
            double distancePenalty = candidate.second > 6.0 ? 0.3 : 1.0;
            double finalProbability = probability * distancePenalty;
            
            if (random.next() < finalProbability) {
                createWarpLane(system, *candidate.first, candidate.second, warpLanes, connections);
            }
        }
    }
    
    // Phase 2: Ensure minimum connectivity
    ensureMinimumConnectivity(systems, warpLanes, connections);
    
    // Phase 3: Ensure network connectivity
    ensureNetworkConnectivity(systems, warpLanes, connections);
    
    return warpLanes;
}

std::pair<double, double> GalaxyGenerator::generateRandomPositionInCircle() {
    double angle = random.range(0, 2 * M_PI);
    double radius = std::sqrt(random.next()) * config.radius;
    return {radius * std::cos(angle), radius * std::sin(angle)};
}

bool GalaxyGenerator::isPositionTooCloseToSystems(const std::pair<double, double>& pos,
                                                const std::vector<StarSystem>& systems,
                                                double minDistance) {
    for (const auto& system : systems) {
        if (calculateDistance(pos, {system.x, system.y}) < minDistance) {
            return true;
        }
    }
    return false;
}

bool GalaxyGenerator::isPositionTooClose(const std::pair<double, double>& pos,
                                       const std::vector<Anomaly>& anomalies,
                                       double minDistance) {
    for (const auto& anomaly : anomalies) {
        if (calculateDistance(pos, {anomaly.x, anomaly.y}) < minDistance) {
            return true;
        }
    }
    return false;
}

double GalaxyGenerator::calculateDistance(const std::pair<double, double>& a,
                                        const std::pair<double, double>& b) {
    double dx = a.first - b.first;
    double dy = a.second - b.second;
    return std::sqrt(dx * dx + dy * dy);
}

void GalaxyGenerator::createWarpLane(StarSystem& system1, StarSystem& system2, double distance,
                                   std::vector<WarpLane>& warpLanes,
                                   std::unordered_map<std::string, std::vector<std::string>>& connections) {
    // Avoid duplicate connections
    auto& conn1 = connections[system1.id];
    if (std::find(conn1.begin(), conn1.end(), system2.id) != conn1.end()) {
        return;
    }
    
    WarpLane lane;
    lane.id = system1.id + "-" + system2.id;
    lane.from = system1.id;
    lane.to = system2.id;
    lane.distance = distance;
    lane.travelTime = static_cast<int>(std::ceil(distance / 5.0)); // 5 LY per turn
    lane.discovered = system1.explored && system2.explored;
    
    warpLanes.push_back(lane);
    
    // Update connections
    connections[system1.id].push_back(system2.id);
    connections[system2.id].push_back(system1.id);
    system1.connections.push_back(system2.id);
    system2.connections.push_back(system1.id);
}

void GalaxyGenerator::ensureMinimumConnectivity(std::vector<StarSystem>& systems,
                                              std::vector<WarpLane>& warpLanes,
                                              std::unordered_map<std::string, std::vector<std::string>>& connections) {
    // Find isolated systems
    for (auto& system : systems) {
        if (connections[system.id].empty()) {
            // Find nearest system
            StarSystem* nearest = nullptr;
            double minDistance = std::numeric_limits<double>::max();
            
            for (auto& other : systems) {
                if (other.id != system.id) {
                    double distance = calculateDistance({system.x, system.y}, {other.x, other.y});
                    if (distance < minDistance) {
                        minDistance = distance;
                        nearest = &other;
                    }
                }
            }
            
            if (nearest && minDistance <= 15.0) {
                createWarpLane(system, *nearest, minDistance, warpLanes, connections);
                std::cout << "🔗 Connected isolated system " << system.name 
                         << " to " << nearest->name << " (" << minDistance << " LY)" << std::endl;
            }
        }
    }
}

void GalaxyGenerator::ensureNetworkConnectivity(std::vector<StarSystem>& systems,
                                              std::vector<WarpLane>& warpLanes,
                                              std::unordered_map<std::string, std::vector<std::string>>& connections) {
    // Simple connectivity check - could be improved with Union-Find
    // For now, just ensure no completely isolated systems
    for (auto& system : systems) {
        if (connections[system.id].empty()) {
            std::cout << "⚠️ Warning: System " << system.name << " remains isolated" << std::endl;
        }
    }
}

std::string GalaxyGenerator::generateSystemName(int index) {
    std::vector<std::string> prefixes = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta"};
    std::vector<std::string> suffixes = {"Centauri", "Draconis", "Leonis", "Aquarii", "Orionis", "Cygni", "Lyrae"};
    
    std::string prefix = prefixes[index % prefixes.size()];
    std::string suffix = suffixes[(index / prefixes.size()) % suffixes.size()];
    
    return prefix + " " + suffix;
}

std::string GalaxyGenerator::generateAnomalyName(const std::string& type, int index) {
    std::unordered_map<std::string, std::vector<std::string>> typeNames = {
        {"nebula", {"Crimson Nebula", "Azure Cloud", "Stellar Nursery", "Dark Nebula"}},
        {"blackhole", {"Void Maw", "Event Horizon", "Singularity", "Dark Star"}},
        {"wormhole", {"Quantum Gate", "Space Fold", "Dimensional Rift", "Warp Tunnel"}},
        {"artifact", {"Ancient Relic", "Precursor Site", "Mysterious Structure", "Alien Beacon"}},
        {"resource", {"Asteroid Field", "Resource Cluster", "Mining Zone", "Rare Elements"}}
    };
    
    auto& names = typeNames[type];
    return names[index % names.size()] + " " + std::to_string((index / names.size()) + 1);
}

std::string GalaxyGenerator::determineSystemType(const std::pair<double, double>& position) {
    double distanceFromOrigin = std::sqrt(position.first * position.first + position.second * position.second);
    double normalizedDistance = distanceFromOrigin / config.radius;
    
    if (normalizedDistance < 0.3) return "major";
    if (normalizedDistance < 0.7) return "minor";
    return "frontier";
}

std::string GalaxyGenerator::generateAnomalyType() {
    std::vector<std::string> types = {"nebula", "blackhole", "wormhole", "artifact", "resource"};
    std::vector<double> weights = {0.4, 0.1, 0.1, 0.2, 0.2};
    
    double random_val = random.next();
    double cumulative = 0;
    
    for (size_t i = 0; i < types.size(); i++) {
        cumulative += weights[i];
        if (random_val < cumulative) {
            return types[i];
        }
    }
    
    return "nebula";
}

} // namespace space4x
