#include "galaxy.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace space4x {

GalaxyGenerator::GalaxyGenerator(const GalaxyConfig& cfg) 
    : config(cfg), random(cfg.seed) {
}

Galaxy GalaxyGenerator::generateGalaxy() {
    std::cout << "🌌 Generating galaxy with seed: " << config.seed << std::endl;
    
    std::vector<StarSystem> systems;
    std::vector<WarpLane> warpLanes;
    
    if (config.connectivity.useVoronoiConnectivity) {
        std::cout << "📐 Using Voronoi-based galaxy generation (like original game)" << std::endl;
        
        // Generate Voronoi sites
        voronoiSites = generateVoronoiSites(config.starSystemCount);
        
        // Compute Voronoi neighbors
        computeVoronoiNeighbors();
        
        // Generate systems from Voronoi sites
        systems = generateSystemsFromVoronoi();
        
        // Generate warp lanes based on Voronoi connectivity
        warpLanes = generateVoronoiWarpLanes(systems);
    } else {
        std::cout << "🔗 Using traditional distance-based galaxy generation" << std::endl;
        
        // Generate star systems
        systems = generateStarSystems();
        
        // Generate warp lanes
        warpLanes = generateWarpLanes(systems);
    }
    
    // Add strategic redundant connections (from original game)
    // Build connections map from existing warp lanes
    std::unordered_map<std::string, std::vector<std::string>> connections;
    for (auto& system : systems) {
        connections[system.id] = std::vector<std::string>();
    }
    
    // Populate connections from existing warp lanes
    for (const auto& lane : warpLanes) {
        connections[lane.from].push_back(lane.to);
        connections[lane.to].push_back(lane.from);
    }
    addRedundantConnections(systems, warpLanes, connections);
    
    // Final safety net: ensure all systems are connected
    ensureMinimumConnectivity(systems, warpLanes, connections);
    
    // Update system connections after redundant connections
    for (auto& system : systems) {
        system.connections = connections[system.id];
    }
    
    // Generate anomalies (same for both approaches)
    auto anomalies = generateAnomalies(systems);
    
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
        // Explicitly initialize all fields to avoid corruption
        system.id = fixedSystem.id;
        system.name = fixedSystem.name;
        system.x = 0.0;
        system.y = 0.0;
        system.type = fixedSystem.type;
        system.isFixed = true;
        system.connections.clear();
        system.explored = (fixedSystem.type == "origin");
        system.population = (fixedSystem.type == "origin") ? 1000000 : 0;
        system.gdp = system.population * random.range(0.8, 1.5);
        
        // Explicitly initialize resources
        system.resources.minerals = random.intRange(50, 200);
        system.resources.energy = random.intRange(50, 200);
        system.resources.research = random.intRange(50, 200);
        
        // Check if this is a predefined system
        const SystemDefinition* detailedSystem = systemConfigManager.getSystemDefinition(fixedSystem.id);
        if (detailedSystem) {
            // Use detailed system data
            system.systemInfo.starType = detailedSystem->starType;
            system.systemInfo.planetCount = detailedSystem->planets.size();
            
            // Count moons
            int totalMoons = 0;
            for (const auto& planet : detailedSystem->planets) {
                totalMoons += planet.moons.size();
            }
            system.systemInfo.moonCount = totalMoons;
            system.systemInfo.asteroidCount = detailedSystem->asteroids.size();
            system.detailedSystem = detailedSystem;
        } else {
            // Use random generation
            std::vector<std::string> starTypes = {"G-class", "K-class", "M-class", "F-class", "A-class"};
            int starTypeIndex = random.intRange(0, starTypes.size() - 1);
            system.systemInfo.starType = starTypes[starTypeIndex];
            system.systemInfo.planetCount = random.intRange(2, 12);
            system.systemInfo.moonCount = random.intRange(0, system.systemInfo.planetCount * 3);
            system.systemInfo.asteroidCount = random.intRange(100, 5000);
            system.detailedSystem = nullptr;
        }
        
        // Debug output for fixed systems
        std::cout << "  " << system.name << " system info: " 
                 << "star=" << system.systemInfo.starType
                 << " planets=" << system.systemInfo.planetCount 
                 << " moons=" << system.systemInfo.moonCount
                 << " asteroids=" << system.systemInfo.asteroidCount
                 << " gdp=" << system.gdp << std::endl;
        
        if (fixedSystem.hasFixedPosition) {
            // Use exact coordinates for real star systems
            system.x = fixedSystem.x;
            system.y = fixedSystem.y;
        } else {
            // Generate position within distance constraint for fictional systems
            double targetDist = fixedSystem.targetDistance;
            double tolerance = fixedSystem.distanceTolerance;
            double minDist = targetDist - tolerance;
            double maxDist = targetDist + tolerance;
            
            // Generate random position within the distance range
            double distance = random.range(minDist, maxDist);
            double angle = random.range(0, 2 * M_PI);
            
            system.x = distance * std::cos(angle);
            system.y = distance * std::sin(angle);
            
            std::cout << "  Placed " << system.name << " at distance " 
                     << std::sqrt(system.x * system.x + system.y * system.y) 
                     << " LY (target: " << targetDist << " ± " << tolerance << ")" << std::endl;
        }
        
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
        // Explicitly initialize all fields to avoid corruption
        system.id = "system-" + std::to_string(i + 1);
        system.name = generateSystemName(i + 1);
        system.x = position.first;
        system.y = position.second;
        system.type = determineSystemType(position);
        system.isFixed = false;
        system.connections.clear();
        system.explored = false;
        system.population = 0;
        system.gdp = 0.0;
        
        // Explicitly initialize resources
        system.resources.minerals = random.intRange(10, 150);
        system.resources.energy = random.intRange(10, 150);
        system.resources.research = random.intRange(10, 150);
        
        // Generate random system using new rules
        SystemDefinition randomSystemDef = systemConfigManager.generateRandomSystem(system.id, system.name);
        system.systemInfo.starType = randomSystemDef.starType;
        system.systemInfo.planetCount = randomSystemDef.planets.size();
        
        // Count moons from generated system
        int totalMoons = 0;
        for (const auto& planet : randomSystemDef.planets) {
            totalMoons += planet.moons.size();
        }
        system.systemInfo.moonCount = totalMoons;
        system.systemInfo.asteroidCount = random.intRange(0, 5); // Few asteroids for random systems
        system.detailedSystem = nullptr; // Random systems don't store detailed data
        
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
        
        // Determine target connections - more for central systems
        double distanceFromOrigin = std::sqrt(system.x * system.x + system.y * system.y);
        double normalizedDistanceFromOrigin = distanceFromOrigin / config.radius;
        
        // Central systems get more connections
        int baseConnections = config.connectivity.minConnections;
        int maxConnections = config.connectivity.maxConnections;
        if (normalizedDistanceFromOrigin < 0.3) {
            maxConnections += 2; // Core systems get up to 2 extra connections
        }
        
        int targetConnections = random.intRange(baseConnections, maxConnections);
        
        // Phase 1a: Always connect to closest systems (guaranteed connectivity)
        int guaranteedConnections = std::min(2, static_cast<int>(candidates.size()));
        for (int i = 0; i < guaranteedConnections && i < static_cast<int>(candidates.size()); i++) {
            const auto& candidate = candidates[i];
            if (std::find(currentConnections.begin(), currentConnections.end(), 
                         candidate.first->id) == currentConnections.end()) {
                createWarpLane(system, *candidate.first, candidate.second, warpLanes, connections);
            }
        }
        
        // Phase 1b: Add additional connections with distance-based probability
        for (const auto& candidate : candidates) {
            if (static_cast<int>(currentConnections.size()) >= targetConnections) break;
            
            // Check if already connected
            if (std::find(currentConnections.begin(), currentConnections.end(), 
                         candidate.first->id) != currentConnections.end()) {
                continue;
            }
            
            // More generous probability for connections
            double normalizedDistance = candidate.second / config.connectivity.maxDistance;
            double probability = std::exp(-normalizedDistance * config.connectivity.distanceDecayFactor);
            
            // Bonus for creating network diversity
            double diversityBonus = 1.0;
            if (connections[candidate.first->id].size() < 2) {
                diversityBonus = 1.5; // Help isolated systems
            }
            
            double finalProbability = probability * diversityBonus;
            
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
            
            if (nearest && minDistance <= config.radius * 0.3) {  // 30% of galaxy radius, very generous for edge systems
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
    std::cout << "🌉 Ensuring network connectivity using MST approach..." << std::endl;
    
    if (systems.size() < 2) return;
    
    // Create mapping from system ID to index for union-find
    std::unordered_map<std::string, size_t> systemToIndex;
    for (size_t i = 0; i < systems.size(); i++) {
        systemToIndex[systems[i].id] = i;
    }
    
    // Union-Find data structures
    std::vector<size_t> parent(systems.size());
    std::vector<size_t> rank(systems.size(), 0);
    
    // Initialize union-find
    for (size_t i = 0; i < systems.size(); i++) {
        parent[i] = i;
    }
    
    // Find function for union-find
    std::function<size_t(size_t)> find = [&](size_t x) -> size_t {
        if (parent[x] != x) {
            parent[x] = find(parent[x]);
        }
        return parent[x];
    };
    
    // Union function for union-find
    auto unite = [&](size_t x, size_t y) -> bool {
        size_t px = find(x);
        size_t py = find(y);
        if (px == py) return false;
        
        if (rank[px] < rank[py]) {
            parent[px] = py;
        } else if (rank[px] > rank[py]) {
            parent[py] = px;
        } else {
            parent[py] = px;
            rank[px]++;
        }
        return true;
    };
    
    // Mark existing connections in union-find
    for (const auto& lane : warpLanes) {
        auto it1 = systemToIndex.find(lane.from);
        auto it2 = systemToIndex.find(lane.to);
        
        if (it1 != systemToIndex.end() && it2 != systemToIndex.end()) {
            unite(it1->second, it2->second);
        }
    }
    
    // Find disconnected components and connect them with minimal bridges
    std::vector<std::pair<double, std::pair<size_t, size_t>>> bridgeEdges;
    
    // Only consider edges between different components
    for (size_t i = 0; i < systems.size(); i++) {
        for (size_t j = i + 1; j < systems.size(); j++) {
            if (find(i) != find(j)) { // Different components
                double dist = calculateDistance({systems[i].x, systems[i].y},
                                              {systems[j].x, systems[j].y});
                bridgeEdges.push_back({dist, {i, j}});
            }
        }
    }
    
    // Sort by distance to prefer shorter bridge connections
    std::sort(bridgeEdges.begin(), bridgeEdges.end());
    
    int bridgesAdded = 0;
    for (const auto& edge : bridgeEdges) {
        size_t u = edge.second.first;
        size_t v = edge.second.second;
        
        // Only add if this connection bridges different components
        if (unite(u, v)) {
            createWarpLane(systems[u], systems[v], edge.first, warpLanes, connections);
            bridgesAdded++;
            std::cout << "  Added bridge lane: " << systems[u].name << " ↔ " << systems[v].name 
                     << " (" << edge.first << " LY)" << std::endl;
        }
    }
    
    if (bridgesAdded > 0) {
        std::cout << "  Added " << bridgesAdded << " bridge connections to ensure full connectivity" << std::endl;
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
    
    // Systems up to 300 LY from origin are considered "core" for connectivity purposes
    if (distanceFromOrigin <= 300.0) return "core";
    return "rim";
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

// ============================================================================
// VORONOI-BASED GALAXY GENERATION (from original game)
// ============================================================================

std::vector<VoronoiSite> GalaxyGenerator::generateVoronoiSites(int numSites) {
    std::vector<VoronoiSite> sites;
    sites.reserve(numSites);
    
    std::cout << "📐 Generating " << numSites << " Voronoi sites (original game approach)" << std::endl;
    
    // Use original game's simple approach: uniform distribution with only minimum distance
    const double minDistance = 2.5;  // Minimum distance between any two systems (slightly more than original 2.0)
    
    for (int i = 0; i < numSites; i++) {
        std::pair<double, double> pos;
        int attempts = 0;
        
        do {
            pos = generateRandomPositionInCircle();
            attempts++;
        } while (!isValidVoronoiPosition(pos, minDistance) && attempts < 500);
        
        if (attempts < 500) {
            VoronoiSite site;
            site.x = pos.first;
            site.y = pos.second;
            site.systemId = "";
            site.hasSystem = false;
            sites.push_back(site);
        }
    }
    
    std::cout << "✅ Generated " << sites.size() << " Voronoi sites using original game distribution" << std::endl;
    return sites;
}

bool GalaxyGenerator::isValidVoronoiPosition(const std::pair<double, double>& pos, double minDistance) {
    for (const auto& site : voronoiSites) {
        if (calculateDistance(pos, {site.x, site.y}) < minDistance) {
            return false;
        }
    }
    return true;
}

void GalaxyGenerator::computeVoronoiNeighbors() {
    std::cout << "🔗 Computing Voronoi neighbor relationships (original game approach)..." << std::endl;
    
    // Clear existing neighbor relationships
    for (auto& site : voronoiSites) {
        site.neighbors.clear();
    }
    
    // Use original game's conservative approach: connect each site to only 1-3 closest neighbors
    for (size_t i = 0; i < voronoiSites.size(); i++) {
        std::vector<std::pair<double, size_t>> distances;
        
        // Calculate distances to all other sites
        for (size_t j = 0; j < voronoiSites.size(); j++) {
            if (i != j) {
                double dist = calculateDistance({voronoiSites[i].x, voronoiSites[i].y},
                                              {voronoiSites[j].x, voronoiSites[j].y});
                distances.push_back({dist, j});
            }
        }
        
        // Sort by distance
        std::sort(distances.begin(), distances.end());
        
        // Create initial connections - connect to more neighbors for better connectivity
        int maxConnections = std::min(6, static_cast<int>(distances.size()));  // Up to 6 instead of 3
        
        // Connect to closest neighbors to ensure good connectivity
        
        for (int k = 0; k < maxConnections && k < static_cast<int>(distances.size()); k++) {
            size_t neighborIdx = distances[k].second;
            double dist = distances[k].first;
            
            // Always connect to the closest neighbors to ensure connectivity
            // Only skip if distance is truly unreasonable (> full galaxy diameter)
            if (dist <= config.radius * 2.0) {
                voronoiSites[i].neighbors.push_back(neighborIdx);
            }
        }
        
        // Debug output removed for production
    }
    
    // Make neighbor relationships symmetric
    for (size_t i = 0; i < voronoiSites.size(); i++) {
        for (size_t neighborIdx : voronoiSites[i].neighbors) {
            auto& neighborNeighbors = voronoiSites[neighborIdx].neighbors;
            if (std::find(neighborNeighbors.begin(), neighborNeighbors.end(), i) == neighborNeighbors.end()) {
                neighborNeighbors.push_back(i);
            }
        }
    }
    
    std::cout << "✅ Computed conservative neighbor relationships for " << voronoiSites.size() << " sites" << std::endl;
}

std::vector<StarSystem> GalaxyGenerator::generateSystemsFromVoronoi() {
    std::vector<StarSystem> systems;
    
    std::cout << "🌟 Assigning systems to Voronoi sites..." << std::endl;
    
    // First, place fixed systems at closest Voronoi sites
    for (const auto& fixedSystem : config.fixedSystems) {
        double systemX, systemY;
        
        if (fixedSystem.hasFixedPosition) {
            // Use exact coordinates for real star systems
            systemX = fixedSystem.x;
            systemY = fixedSystem.y;
        } else {
            // Generate position within distance constraint for fictional systems
            double targetDist = fixedSystem.targetDistance;
            double tolerance = fixedSystem.distanceTolerance;
            double minDist = targetDist - tolerance;
            double maxDist = targetDist + tolerance;
            
            // Generate random position within the distance range
            double distance = random.range(minDist, maxDist);
            double angle = random.range(0, 2 * M_PI);
            
            systemX = distance * std::cos(angle);
            systemY = distance * std::sin(angle);
            
            std::cout << "  Placed " << fixedSystem.name << " at distance " 
                     << std::sqrt(systemX * systemX + systemY * systemY) 
                     << " LY (target: " << targetDist << " ± " << tolerance << ")" << std::endl;
        }
        
        // Find closest Voronoi site to the system position
        size_t closestSite = 0;
        double minDistance = std::numeric_limits<double>::max();
        
        for (size_t i = 0; i < voronoiSites.size(); i++) {
            if (voronoiSites[i].hasSystem) continue;
            
            double dist = calculateDistance({systemX, systemY}, 
                                          {voronoiSites[i].x, voronoiSites[i].y});
            if (dist < minDistance) {
                minDistance = dist;
                closestSite = i;
            }
        }
        
        // Assign system to this site
        voronoiSites[closestSite].hasSystem = true;
        voronoiSites[closestSite].systemId = fixedSystem.id;
        
        StarSystem system;
        // Explicitly initialize all fields to avoid corruption
        system.id = fixedSystem.id;
        system.name = fixedSystem.name;
        system.x = systemX;  // Use calculated position (not Voronoi site position)
        system.y = systemY;
        system.type = fixedSystem.type;
        system.isFixed = true;
        system.connections.clear();
        system.explored = (fixedSystem.type == "origin");
        system.population = (fixedSystem.type == "origin") ? 1000000 : 0;
        system.gdp = system.population * random.range(0.8, 1.5);
        
        // Explicitly initialize resources
        system.resources.minerals = random.intRange(50, 200);
        system.resources.energy = random.intRange(50, 200);
        system.resources.research = random.intRange(50, 200);
        
        // Check if this is a predefined system
        const SystemDefinition* detailedSystem = systemConfigManager.getSystemDefinition(fixedSystem.id);
        if (detailedSystem) {
            // Use detailed system data
            system.systemInfo.starType = detailedSystem->starType;
            system.systemInfo.planetCount = detailedSystem->planets.size();
            
            // Count moons
            int totalMoons = 0;
            for (const auto& planet : detailedSystem->planets) {
                totalMoons += planet.moons.size();
            }
            system.systemInfo.moonCount = totalMoons;
            system.systemInfo.asteroidCount = detailedSystem->asteroids.size();
            system.detailedSystem = detailedSystem;
        } else {
            // Use random generation
            std::vector<std::string> starTypes = {"G-class", "K-class", "M-class", "F-class", "A-class"};
            system.systemInfo.starType = starTypes[random.intRange(0, starTypes.size() - 1)];
            system.systemInfo.planetCount = random.intRange(4, 10); // Updated to new rules
            system.systemInfo.moonCount = random.intRange(0, system.systemInfo.planetCount / 2);
            system.systemInfo.asteroidCount = random.intRange(0, 5);
            system.detailedSystem = nullptr;
        }
        
        systems.push_back(system);
        
        std::cout << "  Fixed system: " << system.name << " at (" << system.x << ", " << system.y << ")" << std::endl;
    }
    
    // Generate remaining systems at remaining Voronoi sites
    int systemIndex = 1;
    for (size_t i = 0; i < voronoiSites.size() && systems.size() < static_cast<size_t>(config.starSystemCount); i++) {
        if (voronoiSites[i].hasSystem) continue;
        
        voronoiSites[i].hasSystem = true;
        voronoiSites[i].systemId = "system-" + std::to_string(systemIndex);
        
        StarSystem system;
        // Explicitly initialize all fields to avoid corruption
        system.id = voronoiSites[i].systemId;
        system.name = generateSystemName(systemIndex);
        system.x = voronoiSites[i].x;
        system.y = voronoiSites[i].y;
        system.type = determineSystemType({system.x, system.y});
        system.isFixed = false;
        system.connections.clear();
        system.explored = false;
        system.population = 0;
        system.gdp = 0.0;
        
        // Explicitly initialize resources
        system.resources.minerals = random.intRange(10, 150);
        system.resources.energy = random.intRange(10, 150);
        system.resources.research = random.intRange(10, 150);
        
        // Generate random system using new rules
        SystemDefinition randomSystemDef = systemConfigManager.generateRandomSystem(system.id, system.name);
        system.systemInfo.starType = randomSystemDef.starType;
        system.systemInfo.planetCount = randomSystemDef.planets.size();
        
        // Count moons from generated system
        int totalMoons = 0;
        for (const auto& planet : randomSystemDef.planets) {
            totalMoons += planet.moons.size();
        }
        system.systemInfo.moonCount = totalMoons;
        system.systemInfo.asteroidCount = random.intRange(0, 5); // Few asteroids for random systems
        system.detailedSystem = nullptr; // Random systems don't store detailed data
        
        // Debug output for first few systems
        if (systemIndex <= 7) {
            std::cout << "  " << system.name << " (Voronoi) system info: " 
                     << "star=" << system.systemInfo.starType
                     << " planets=" << system.systemInfo.planetCount 
                     << " moons=" << system.systemInfo.moonCount
                     << " asteroids=" << system.systemInfo.asteroidCount
                     << " gdp=" << system.gdp << std::endl;
        }
        
        systems.push_back(system);
        systemIndex++;
    }
    
    std::cout << "✅ Generated " << systems.size() << " star systems using Voronoi distribution" << std::endl;
    return systems;
}

std::vector<WarpLane> GalaxyGenerator::generateVoronoiWarpLanes(std::vector<StarSystem>& systems) {
    std::vector<WarpLane> warpLanes;
    std::unordered_map<std::string, std::vector<std::string>> connections;
    
    std::cout << "🔗 Generating warp lanes using Voronoi connectivity..." << std::endl;
    
    // Initialize connection tracking
    for (auto& system : systems) {
        connections[system.id] = std::vector<std::string>();
    }
    
    // Create warp lanes based on Voronoi neighbor relationships
    int potentialLanes = 0;
    int createdLanes = 0;
    
    for (size_t i = 0; i < voronoiSites.size(); i++) {
        if (!voronoiSites[i].hasSystem) continue;
        
        for (size_t neighborIdx : voronoiSites[i].neighbors) {
            if (neighborIdx >= voronoiSites.size() || !voronoiSites[neighborIdx].hasSystem) continue;
            
            // Only create each connection once (avoid duplicates)
            if (i < neighborIdx) {
                potentialLanes++;
                
                // Find the systems corresponding to these sites
                StarSystem* system1 = nullptr;
                StarSystem* system2 = nullptr;
                
                for (auto& sys : systems) {
                    if (sys.id == voronoiSites[i].systemId) system1 = &sys;
                    if (sys.id == voronoiSites[neighborIdx].systemId) system2 = &sys;
                }
                
                if (system1 && system2) {
                    double distance = calculateDistance({system1->x, system1->y}, {system2->x, system2->y});
                    
                    // Calculate base distance threshold
                    double baseVoronoiDistance = config.connectivity.maxDistance * 1.5;
                    double galaxyScaledDistance = config.radius * 0.25;  // 25% of galaxy radius
                    double baseMaxDistance = std::max(baseVoronoiDistance, galaxyScaledDistance);
                    
                    // Apply tiered connectivity based on system types
                    double maxVoronoiDistance = calculateTieredDistance(system1, system2, baseMaxDistance);
                    
                    if (distance <= maxVoronoiDistance) {
                        createWarpLane(*system1, *system2, distance, warpLanes, connections);
                        createdLanes++;
                    }
                }
            }
        }
    }
    
    std::cout << "  Evaluated " << potentialLanes << " potential lanes, created " << createdLanes << std::endl;
    
    // Ensure minimum connectivity using traditional approach as fallback
    ensureMinimumConnectivity(systems, warpLanes, connections);
    
    std::cout << "✅ Generated " << warpLanes.size() << " warp lanes using Voronoi method" << std::endl;
    return warpLanes;
}

void GalaxyGenerator::addRedundantConnections(std::vector<StarSystem>& systems,
                                            std::vector<WarpLane>& warpLanes,
                                            std::unordered_map<std::string, std::vector<std::string>>& connections) {
    std::cout << "🔗 Adding strategic redundant connections..." << std::endl;
    
    if (systems.size() < 3) {
        std::cout << "  Not enough systems for redundant connections." << std::endl;
        return;
    }
    
    // Find vulnerable systems (systems with 1-2 connections or far from center)
    std::vector<StarSystem*> vulnerableSystems;
    double centerX = 0, centerY = 0;
    
    // Calculate galaxy center
    for (const auto& system : systems) {
        centerX += system.x;
        centerY += system.y;
    }
    centerX /= systems.size();
    centerY /= systems.size();
    
    for (auto& system : systems) {
        int connectionCount = connections[system.id].size();
        double distanceFromCenter = calculateDistance({system.x, system.y}, {centerX, centerY});
        
        // Mark as vulnerable if:
        // - Has only 1-2 connections, OR
        // - Is far from center (> 60% of galaxy radius) and has < 4 connections
        bool isVulnerable = (connectionCount <= 2) || 
                           (distanceFromCenter > config.radius * 0.6 && connectionCount < 4);
        
        if (isVulnerable) {
            vulnerableSystems.push_back(&system);
        }
    }
    
    std::cout << "  Found " << vulnerableSystems.size() << " vulnerable/outlying systems" << std::endl;
    
    // Add redundant connections for vulnerable systems
    int redundantConnectionsAdded = 0;
    const int maxRedundantConnections = std::min(static_cast<int>(systems.size() / 4), 40);  // Slightly more generous for gameplay
    
    for (StarSystem* vulnSystem : vulnerableSystems) {
        if (redundantConnectionsAdded >= maxRedundantConnections) break;
        
        // Find potential connection targets (systems not already connected)
        std::vector<std::pair<double, StarSystem*>> potentialConnections;
        
        for (auto& system : systems) {
            StarSystem* targetSystem = &system;
            
            // Skip if same system or already connected
            if (targetSystem == vulnSystem) continue;
            
            bool alreadyConnected = false;
            for (const std::string& connectedId : connections[vulnSystem->id]) {
                if (connectedId == targetSystem->id) {
                    alreadyConnected = true;
                    break;
                }
            }
            if (alreadyConnected) continue;
            
            double distance = calculateDistance({vulnSystem->x, vulnSystem->y}, 
                                              {targetSystem->x, targetSystem->y});
            
            // Adjust score based on target system's connectivity (prefer well-connected systems)
            int targetConnections = connections[targetSystem->id].size();
            double connectionScore = distance / (1.0 + targetConnections * 0.2);
            
            potentialConnections.push_back({connectionScore, targetSystem});
        }
        
        // Sort by connection score (distance adjusted for target connectivity)
        std::sort(potentialConnections.begin(), potentialConnections.end());
        
        // Add 1-2 redundant connections for this vulnerable system  
        int connectionsToAdd = (connections[vulnSystem->id].size() == 1) ? 2 : 1;
        
        for (int i = 0; i < connectionsToAdd && 
                        i < static_cast<int>(potentialConnections.size()) &&
                        redundantConnectionsAdded < maxRedundantConnections; ++i) {
            
            StarSystem* targetSystem = potentialConnections[i].second;
            double distance = calculateDistance({vulnSystem->x, vulnSystem->y},
                                              {targetSystem->x, targetSystem->y});
            
            // Only add if distance is reasonable (more generous for redundant connections)
            // Use 40% of galaxy radius for redundant connections to ensure better connectivity
            if (distance < config.radius * 0.4) {
                createWarpLane(*vulnSystem, *targetSystem, distance, warpLanes, connections);
                redundantConnectionsAdded++;
                
                std::cout << "  Added redundant connection: " << vulnSystem->name 
                         << " ↔ " << targetSystem->name 
                         << " (distance: " << distance << " LY)" << std::endl;
            }
        }
    }
    
    if (redundantConnectionsAdded > 0) {
        std::cout << "  Added " << redundantConnectionsAdded 
                 << " redundant connections for network resilience" << std::endl;
    } else {
        std::cout << "  No suitable redundant connections found within distance limits" << std::endl;
    }
}

double GalaxyGenerator::calculateTieredDistance(const StarSystem* system1, const StarSystem* system2, double baseDistance) {
    // Determine connectivity tier based on system types
    // origin system gets +150% range (2.5x) - galactic capital
    // core systems get +100% range (2.0x) - extremely well connected core
    // rim systems get -60% range (0.4x) - very isolated outer rim
    // Mixed connections use the more generous (higher) threshold
    
    auto getDistanceMultiplier = [](const std::string& systemType) -> double {
        if (systemType == "origin") {
            return 2.5;   // +150% for origin system (galactic capital)
        } else if (systemType == "core") {
            return 2.0;   // +100% for core systems (extremely well connected)
        } else {
            return 0.4;   // -60% for rim systems (very isolated)
        }
    };
    
    double multiplier1 = getDistanceMultiplier(system1->type);
    double multiplier2 = getDistanceMultiplier(system2->type);
    
    // Use the more generous (higher) multiplier for mixed connections
    double finalMultiplier = std::max(multiplier1, multiplier2);
    
    return baseDistance * finalMultiplier;
}

} // namespace space4x
