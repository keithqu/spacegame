#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <nlohmann/json.hpp>

namespace space4x {

// Strategic resource types
enum class ResourceType {
    MINERALS,
    RARE_METALS,
    ENERGY_CRYSTALS,
    WATER_ICE,
    HELIUM_3,
    DEUTERIUM,
    ANTIMATTER,
    EXOTIC_MATTER
};

// Resource deposit on a celestial body
struct ResourceDeposit {
    ResourceType type;
    int abundance;      // 0-100 scale
    int accessibility;  // 0-100 scale (how easy to extract)
};

// Base class for all celestial bodies
struct CelestialBody {
    std::string id;
    std::string name;
    std::string type;           // "planet", "moon", "asteroid", "station"
    double distanceFromParent;  // AU for planets from star, km for moons from planet
    double radius;              // km
    double diameter;            // km (2 * radius, but stored separately for clarity)
    double mass;                // Earth masses
    double gravity;             // Percentage of Earth gravity (100 = 1g)
    int habitability;           // Percentage of Earth habitability (100 = fully habitable)
    std::string atmosphere;     // Description of atmosphere
    std::string composition;    // Primary composition
    std::vector<ResourceDeposit> resources;
    
    // Default constructor
    CelestialBody() : distanceFromParent(0), radius(0), diameter(0), mass(0), gravity(0), habitability(0) {}
};

// Moon orbiting a planet
struct Moon : public CelestialBody {
    Moon() {
        type = "moon";
    }
};

// Planet in a star system
struct Planet : public CelestialBody {
    std::vector<Moon> moons;
    
    Planet() {
        type = "planet";
    }
};

// Asteroid or asteroid belt
struct Asteroid : public CelestialBody {
    Asteroid() {
        type = "asteroid";
    }
};

// Complete star system definition
struct SystemDefinition {
    std::string systemId;
    std::string systemName;
    std::string starType;
    double starMass;            // Solar masses
    double starRadius;          // Solar radii
    int starTemperature;        // Kelvin
    
    std::vector<Planet> planets;
    std::vector<Asteroid> asteroids;
    
    // Constructor
    SystemDefinition() : starMass(1.0), starRadius(1.0), starTemperature(5778) {}
};

// System configuration manager
class SystemConfigManager {
public:
    SystemConfigManager();
    
    // Load predefined systems
    void loadPredefinedSystems();
    
    // Get system definition by ID
    const SystemDefinition* getSystemDefinition(const std::string& systemId) const;
    
    // Generate random system
    SystemDefinition generateRandomSystem(const std::string& systemId, const std::string& systemName);
    
    // Check if system is predefined
    bool isSystemPredefined(const std::string& systemId) const;

private:
    std::unordered_map<std::string, SystemDefinition> predefinedSystems;
    
    // Helper methods for random generation
    Planet generateRandomPlanet(int planetIndex, double distanceFromStar, std::mt19937& gen);
    Moon generateRandomMoon(int moonIndex, const Planet& parentPlanet, std::mt19937& gen);
    std::vector<ResourceDeposit> generateRandomResources(const std::string& bodyType, int habitability);
    
    // JSON loading methods
    bool loadSystemsFromJson(const std::string& filename);
};

} // namespace space4x
