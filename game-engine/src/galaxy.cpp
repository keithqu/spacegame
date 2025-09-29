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

// ============================================================================
// VORONOI-BASED GALAXY GENERATION (from original game)
// ============================================================================

std::vector<VoronoiSite> GalaxyGenerator::generateVoronoiSites(int numSites) {
    std::vector<VoronoiSite> sites;
    sites.reserve(numSites);
    
    std::cout << "📐 Generating " << numSites << " Voronoi sites with min distance " << config.minDistance << " LY" << std::endl;
    
    for (int i = 0; i < numSites; i++) {
        std::pair<double, double> pos;
        int attempts = 0;
        
        do {
            pos = generateRandomPositionInCircle();
            attempts++;
        } while (!isValidVoronoiPosition(pos, config.minDistance) && attempts < 500);
        
        if (attempts < 500) {
            VoronoiSite site;
            site.x = pos.first;
            site.y = pos.second;
            site.systemId = "";
            site.hasSystem = false;
            sites.push_back(site);
        }
    }
    
    std::cout << "✅ Generated " << sites.size() << " valid Voronoi sites" << std::endl;
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
    // Simple Delaunay triangulation approximation for neighbor relationships
    // For each site, find nearby sites within a reasonable distance
    
    std::cout << "🔗 Computing Voronoi neighbor relationships..." << std::endl;
    
    for (size_t i = 0; i < voronoiSites.size(); i++) {
        voronoiSites[i].neighbors.clear();
        
        // Find potential neighbors - be very generous to ensure connectivity
        double maxNeighborDistance = config.radius * 0.5;  // Up to half the galaxy radius
        
        std::vector<std::pair<size_t, double>> candidates;
        for (size_t j = 0; j < voronoiSites.size(); j++) {
            if (i == j) continue;
            
            double dist = calculateDistance({voronoiSites[i].x, voronoiSites[i].y}, 
                                          {voronoiSites[j].x, voronoiSites[j].y});
            
            if (dist <= maxNeighborDistance) {
                candidates.push_back({j, dist});
            }
        }
        
        // Sort by distance and take closest 3-6 neighbors
        std::sort(candidates.begin(), candidates.end(), 
                 [](const auto& a, const auto& b) { return a.second < b.second; });
        
        int maxNeighbors = random.intRange(4, 8);  // Increased from 3-6 to 4-8
        for (size_t k = 0; k < candidates.size() && k < static_cast<size_t>(maxNeighbors); k++) {
            voronoiSites[i].neighbors.push_back(candidates[k].first);
        }
        
        // Removed debug output for production
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
    
    std::cout << "✅ Computed neighbor relationships for " << voronoiSites.size() << " sites" << std::endl;
}

std::vector<StarSystem> GalaxyGenerator::generateSystemsFromVoronoi() {
    std::vector<StarSystem> systems;
    
    std::cout << "🌟 Assigning systems to Voronoi sites..." << std::endl;
    
    // First, place fixed systems at closest Voronoi sites
    for (const auto& fixedSystem : config.fixedSystems) {
        // Find closest Voronoi site
        size_t closestSite = 0;
        double minDistance = std::numeric_limits<double>::max();
        
        for (size_t i = 0; i < voronoiSites.size(); i++) {
            if (voronoiSites[i].hasSystem) continue;
            
            double dist = calculateDistance({fixedSystem.x, fixedSystem.y}, 
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
        system.id = fixedSystem.id;
        system.name = fixedSystem.name;
        system.x = voronoiSites[closestSite].x;  // Use Voronoi site position
        system.y = voronoiSites[closestSite].y;
        system.type = fixedSystem.type;
        system.isFixed = true;
        system.explored = (fixedSystem.type == "origin");
        system.population = (fixedSystem.type == "origin") ? 1000000 : 0;
        system.resources.minerals = random.intRange(50, 200);
        system.resources.energy = random.intRange(50, 200);
        system.resources.research = random.intRange(50, 200);
        
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
        system.id = voronoiSites[i].systemId;
        system.name = generateSystemName(systemIndex);
        system.x = voronoiSites[i].x;
        system.y = voronoiSites[i].y;
        system.type = determineSystemType({system.x, system.y});
        system.isFixed = false;
        system.explored = false;
        system.population = 0;
        system.resources.minerals = random.intRange(10, 150);
        system.resources.energy = random.intRange(10, 150);
        system.resources.research = random.intRange(10, 150);
        
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
                    
                    // For Voronoi connectivity, be more generous with distance
                    double maxVoronoiDistance = config.connectivity.maxDistance * 1.5;
                    
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

} // namespace space4x
