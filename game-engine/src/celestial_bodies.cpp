#include "celestial_bodies.h"
#include <random>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>

using json = nlohmann::json;

namespace space4x {

SystemConfigManager::SystemConfigManager() {
    loadPredefinedSystems();
}

void SystemConfigManager::loadPredefinedSystems() {
    // Try multiple possible paths for the JSON file
    std::vector<std::string> possiblePaths = {
        "config/systems.json",
        "game-engine/config/systems.json",
        "../config/systems.json",
        "./config/systems.json"
    };
    
    for (const std::string& path : possiblePaths) {
        if (loadSystemsFromJson(path)) {
            std::cout << "✅ Loaded " << predefinedSystems.size() << " predefined star systems from JSON (" << path << ")" << std::endl;
            return;
        }
    }
    
    // No fallback - JSON configuration is required
    std::cout << "❌ Could not load systems.json from any of these paths:" << std::endl;
    for (const std::string& path : possiblePaths) {
        std::cout << "   - " << path << std::endl;
    }
    std::cout << "   Make sure config/systems.json exists and is valid" << std::endl;
}

const SystemDefinition* SystemConfigManager::getSystemDefinition(const std::string& systemId) const {
    auto it = predefinedSystems.find(systemId);
    if (it != predefinedSystems.end()) {
        return &it->second;
    }
    return nullptr;
}

bool SystemConfigManager::isSystemPredefined(const std::string& systemId) const {
    return predefinedSystems.find(systemId) != predefinedSystems.end();
}





bool SystemConfigManager::loadSystemsFromJson(const std::string& filename) {
    std::cout << "🔍 Trying to load: " << filename << std::endl;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "   ❌ File not found or cannot open: " << filename << std::endl;
        return false;
    }
    std::cout << "   ✅ File opened successfully: " << filename << std::endl;
    
    try {
        json jsonData;
        file >> jsonData;
        file.close();
        
        std::cout << "   📄 JSON parsed successfully" << std::endl;
        
        if (!jsonData.contains("systems") || !jsonData["systems"].is_array()) {
            std::cout << "   ❌ No 'systems' array found in JSON" << std::endl;
            return false;
        }
        
        auto systemsArray = jsonData["systems"];
        std::cout << "   📊 Found " << systemsArray.size() << " systems in JSON" << std::endl;
        
        for (const auto& systemJson : systemsArray) {
            SystemDefinition system;
            
            // Parse basic system info
            system.systemId = systemJson.value("systemId", "");
            system.systemName = systemJson.value("systemName", "");
            system.starType = systemJson.value("starType", "G-class");
            system.starMass = systemJson.value("starMass", 1.0);
            system.starRadius = systemJson.value("starRadius", 1.0);
            system.starTemperature = systemJson.value("starTemperature", 5778);
            
            
            // Parse planets
            if (systemJson.contains("planets") && systemJson["planets"].is_array()) {
                for (const auto& planetJson : systemJson["planets"]) {
                    Planet planet;
                    planet.id = planetJson.value("id", "");
                    planet.name = planetJson.value("name", "");
                    planet.type = planetJson.value("type", "planet");
                    planet.distanceFromParent = planetJson.value("distanceFromStar", 1.0);
                    planet.radius = planetJson.value("radius", 6371.0);
                    planet.diameter = planetJson.value("diameter", 12742.0);
                    planet.mass = planetJson.value("mass", 1.0);
                    planet.gravity = planetJson.value("gravity", 100);
                    planet.habitability = planetJson.value("habitability", 0);
                    planet.atmosphere = planetJson.value("atmosphere", "None");
                    planet.composition = planetJson.value("composition", "Rock");
                    
                    // Parse planet resources
                    if (planetJson.contains("resources") && planetJson["resources"].is_array()) {
                        for (const auto& resourceJson : planetJson["resources"]) {
                            ResourceDeposit resource;
                            resource.type = static_cast<ResourceType>(resourceJson.value("type", 0));
                            resource.abundance = resourceJson.value("abundance", 50);
                            resource.accessibility = resourceJson.value("accessibility", 50);
                            planet.resources.push_back(resource);
                        }
                    }
                    
                    // Parse moons
                    if (planetJson.contains("moons") && planetJson["moons"].is_array()) {
                        for (const auto& moonJson : planetJson["moons"]) {
                            Moon moon;
                            moon.id = moonJson.value("id", "");
                            moon.name = moonJson.value("name", "");
                            moon.type = moonJson.value("type", "moon");
                            moon.distanceFromParent = moonJson.value("distanceFromPlanet", 384400.0);
                            moon.radius = moonJson.value("radius", 1737.4);
                            moon.diameter = moonJson.value("diameter", 3474.8);
                            moon.mass = moonJson.value("mass", 0.012);
                            moon.gravity = moonJson.value("gravity", 17);
                            moon.habitability = moonJson.value("habitability", 0);
                            moon.atmosphere = moonJson.value("atmosphere", "None");
                            moon.composition = moonJson.value("composition", "Rock");
                            
                            // Parse moon resources
                            if (moonJson.contains("resources") && moonJson["resources"].is_array()) {
                                for (const auto& resourceJson : moonJson["resources"]) {
                                    ResourceDeposit resource;
                                    resource.type = static_cast<ResourceType>(resourceJson.value("type", 0));
                                    resource.abundance = resourceJson.value("abundance", 50);
                                    resource.accessibility = resourceJson.value("accessibility", 50);
                                    moon.resources.push_back(resource);
                                }
                            }
                            
                            planet.moons.push_back(moon);
                        }
                    }
                    
                    system.planets.push_back(planet);
                }
            }
            
            // Parse asteroids
            if (systemJson.contains("asteroids") && systemJson["asteroids"].is_array()) {
                for (const auto& asteroidJson : systemJson["asteroids"]) {
                    Asteroid asteroid;
                    asteroid.id = asteroidJson.value("id", "");
                    asteroid.name = asteroidJson.value("name", "");
                    asteroid.type = asteroidJson.value("type", "asteroid");
                    asteroid.distanceFromParent = asteroidJson.value("distanceFromStar", 2.77);
                    asteroid.radius = asteroidJson.value("radius", 473.0);
                    asteroid.diameter = asteroidJson.value("diameter", 946.0);
                    asteroid.mass = asteroidJson.value("mass", 0.00016);
                    asteroid.gravity = asteroidJson.value("gravity", 3);
                    asteroid.habitability = asteroidJson.value("habitability", 0);
                    asteroid.atmosphere = asteroidJson.value("atmosphere", "None");
                    asteroid.composition = asteroidJson.value("composition", "Rock");
                    
                    // Parse asteroid resources
                    if (asteroidJson.contains("resources") && asteroidJson["resources"].is_array()) {
                        for (const auto& resourceJson : asteroidJson["resources"]) {
                            ResourceDeposit resource;
                            resource.type = static_cast<ResourceType>(resourceJson.value("type", 0));
                            resource.abundance = resourceJson.value("abundance", 50);
                            resource.accessibility = resourceJson.value("accessibility", 50);
                            asteroid.resources.push_back(resource);
                        }
                    }
                    
                    system.asteroids.push_back(asteroid);
                }
            }
            
            predefinedSystems[system.systemId] = system;
        }
        
        std::cout << "   📊 Total systems stored: " << predefinedSystems.size() << std::endl;
        return !predefinedSystems.empty();
        
    } catch (const json::exception& e) {
        std::cout << "   ❌ JSON parsing error: " << e.what() << std::endl;
        return false;
    }
}


SystemDefinition SystemConfigManager::generateRandomSystem(const std::string& systemId, const std::string& systemName) {
    SystemDefinition system;
    system.systemId = systemId;
    system.systemName = systemName;
    
    // Create deterministic seed from system ID
    std::hash<std::string> hasher;
    size_t seed = hasher(systemId);
    std::mt19937 gen(seed);
    
    // Random star properties
    std::vector<std::string> starTypes = {"G-class", "K-class", "M-class", "F-class", "A-class"};
    std::uniform_int_distribution<> starTypeDist(0, starTypes.size() - 1);
    std::uniform_real_distribution<> massDist(0.5, 2.0);
    std::uniform_real_distribution<> radiusDist(0.7, 1.8);
    std::uniform_int_distribution<> tempDist(3000, 7000);
    
    system.starType = starTypes[starTypeDist(gen)];
    system.starMass = massDist(gen);
    system.starRadius = radiusDist(gen);
    system.starTemperature = tempDist(gen);
    
    // Generate 4-10 planets
    std::uniform_int_distribution<> planetCountDist(4, 10);
    int planetCount = planetCountDist(gen);
    
    double currentDistance = 0.3; // Start close to star
    for (int i = 0; i < planetCount; i++) {
        Planet planet = generateRandomPlanet(i, currentDistance, gen);
        system.planets.push_back(planet);
        
        // Increase distance for next planet
        std::uniform_real_distribution<> distanceIncrease(1.3, 2.2);
        currentDistance *= distanceIncrease(gen);
    }
    
    return system;
}

Planet SystemConfigManager::generateRandomPlanet(int planetIndex, double distanceFromStar, std::mt19937& gen) {
    Planet planet;
    planet.id = "planet-" + std::to_string(planetIndex + 1);
    planet.name = "Planet " + std::to_string(planetIndex + 1);
    planet.distanceFromParent = distanceFromStar;
    
    // Determine planet type based on distance from star and random factors
    std::uniform_real_distribution<> typeDist(0.0, 1.0);
    double typeRoll = typeDist(gen);
    
    if (distanceFromStar < 2.0) {
        // Inner system - mostly terrestrial planets
        if (typeRoll < 0.8) {
            // Terrestrial planet
            std::uniform_real_distribution<> radiusDist(2000, 8000); // Mercury to Super-Earth size
            planet.radius = radiusDist(gen);
            planet.diameter = planet.radius * 2;
            
            // Realistic mass-radius relationship for terrestrial planets
            // Mass scales roughly with radius^3 but with density variations
            double earthRadiusRatio = planet.radius / 6371.0;
            std::uniform_real_distribution<> densityFactor(0.7, 1.3); // Density variation
            planet.mass = earthRadiusRatio * earthRadiusRatio * earthRadiusRatio * densityFactor(gen);
            
            // Gravity = mass / radius^2 (relative to Earth)
            planet.gravity = static_cast<int>(planet.mass * 100 / (earthRadiusRatio * earthRadiusRatio));
            planet.composition = "Silicate rock with iron core";
        } else {
            // Small gas planet (mini-Neptune)
            std::uniform_real_distribution<> radiusDist(8000, 25000);
            planet.radius = radiusDist(gen);
            planet.diameter = planet.radius * 2;
            
            double earthRadiusRatio = planet.radius / 6371.0;
            std::uniform_real_distribution<> densityFactor(0.3, 0.8); // Lower density for gas planets
            planet.mass = earthRadiusRatio * earthRadiusRatio * earthRadiusRatio * densityFactor(gen);
            
            planet.gravity = static_cast<int>(planet.mass * 100 / (earthRadiusRatio * earthRadiusRatio));
            planet.composition = "Hydrogen and helium with rocky core";
        }
    } else {
        // Outer system - mix of gas giants and ice worlds
        if (typeRoll < 0.4) {
            // Gas giant
            std::uniform_real_distribution<> radiusDist(25000, 80000); // Neptune to Jupiter size
            planet.radius = radiusDist(gen);
            planet.diameter = planet.radius * 2;
            
            double earthRadiusRatio = planet.radius / 6371.0;
            std::uniform_real_distribution<> densityFactor(0.2, 0.6); // Very low density
            planet.mass = earthRadiusRatio * earthRadiusRatio * earthRadiusRatio * densityFactor(gen);
            
            planet.gravity = static_cast<int>(planet.mass * 100 / (earthRadiusRatio * earthRadiusRatio));
            planet.composition = "Hydrogen and helium gas giant";
        } else if (typeRoll < 0.7) {
            // Ice giant
            std::uniform_real_distribution<> radiusDist(15000, 30000); // Uranus/Neptune size
            planet.radius = radiusDist(gen);
            planet.diameter = planet.radius * 2;
            
            double earthRadiusRatio = planet.radius / 6371.0;
            std::uniform_real_distribution<> densityFactor(0.4, 0.9); // Medium density
            planet.mass = earthRadiusRatio * earthRadiusRatio * earthRadiusRatio * densityFactor(gen);
            
            planet.gravity = static_cast<int>(planet.mass * 100 / (earthRadiusRatio * earthRadiusRatio));
            planet.composition = "Water, methane, and ammonia ices over rock core";
        } else {
            // Ice world (terrestrial with lots of ice)
            std::uniform_real_distribution<> radiusDist(3000, 10000);
            planet.radius = radiusDist(gen);
            planet.diameter = planet.radius * 2;
            
            double earthRadiusRatio = planet.radius / 6371.0;
            std::uniform_real_distribution<> densityFactor(0.5, 1.1); // Lower density due to ice
            planet.mass = earthRadiusRatio * earthRadiusRatio * earthRadiusRatio * densityFactor(gen);
            
            planet.gravity = static_cast<int>(planet.mass * 100 / (earthRadiusRatio * earthRadiusRatio));
            planet.composition = "Water ice and silicate rock";
        }
    }
    
    // Habitability based on distance (habitable zone around 0.8-1.5 AU)
    if (distanceFromStar >= 0.8 && distanceFromStar <= 1.5) {
        std::uniform_int_distribution<> habitDist(20, 80);
        planet.habitability = habitDist(gen);
    } else if (distanceFromStar >= 0.5 && distanceFromStar <= 2.0) {
        std::uniform_int_distribution<> habitDist(5, 30);
        planet.habitability = habitDist(gen);
    } else {
        planet.habitability = 0;
    }
    
    // Random atmosphere and composition
    std::vector<std::string> atmospheres = {
        "Thin carbon dioxide", "Dense nitrogen-oxygen", "Methane and hydrogen",
        "Thick carbon dioxide", "Hydrogen and helium", "None"
    };
    std::vector<std::string> compositions = {
        "Silicate rock with iron core", "Gas giant", "Ice and rock",
        "Mostly iron", "Carbon and silicate"
    };
    
    std::uniform_int_distribution<> atmoDist(0, atmospheres.size() - 1);
    std::uniform_int_distribution<> compDist(0, compositions.size() - 1);
    
    planet.atmosphere = atmospheres[atmoDist(gen)];
    planet.composition = compositions[compDist(gen)];
    
    // Generate resources
    planet.resources = generateRandomResources("planet", planet.habitability);
    
    // 10% chance for at least one moon
    std::uniform_real_distribution<> moonChance(0.0, 1.0);
    if (moonChance(gen) <= 0.1) {
        std::uniform_int_distribution<> moonCountDist(1, 3);
        int moonCount = moonCountDist(gen);
        
        for (int i = 0; i < moonCount; i++) {
            Moon moon = generateRandomMoon(i, planet, gen);
            planet.moons.push_back(moon);
        }
    }
    
    return planet;
}

Moon SystemConfigManager::generateRandomMoon(int moonIndex, const Planet& parentPlanet, std::mt19937& gen) {
    Moon moon;
    moon.id = parentPlanet.id + "-moon-" + std::to_string(moonIndex + 1);
    moon.name = parentPlanet.name + " Moon " + std::to_string(moonIndex + 1);
    
    // Distance from parent planet (in km)
    std::uniform_real_distribution<> distanceDist(10000, 500000);
    moon.distanceFromParent = distanceDist(gen);
    
    // Smaller than parent planet
    std::uniform_real_distribution<> radiusRatio(0.1, 0.4);
    moon.radius = parentPlanet.radius * radiusRatio(gen);
    moon.diameter = moon.radius * 2;
    
    // Realistic mass-radius relationship for moons (similar density to parent)
    double earthRadiusRatio = moon.radius / 6371.0;
    std::uniform_real_distribution<> densityFactor(0.6, 1.2); // Similar to terrestrial bodies
    moon.mass = earthRadiusRatio * earthRadiusRatio * earthRadiusRatio * densityFactor(gen);
    
    moon.gravity = static_cast<int>(moon.mass * 100 / (earthRadiusRatio * earthRadiusRatio));
    moon.habitability = std::min(parentPlanet.habitability / 2, 20); // Lower than parent
    
    moon.atmosphere = "Extremely thin or none";
    moon.composition = "Silicate rock and ice";
    
    // Generate resources
    moon.resources = generateRandomResources("moon", moon.habitability);
    
    return moon;
}

std::vector<ResourceDeposit> SystemConfigManager::generateRandomResources(const std::string& bodyType, int habitability) {
    std::vector<ResourceDeposit> resources;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Base resource types that most bodies have
    std::vector<ResourceType> commonResources = {ResourceType::MINERALS, ResourceType::RARE_METALS};
    
    for (ResourceType resType : commonResources) {
        std::uniform_int_distribution<> abundanceDist(20, 80);
        std::uniform_int_distribution<> accessDist(30, 90);
        
        ResourceDeposit deposit;
        deposit.type = resType;
        deposit.abundance = abundanceDist(gen);
        deposit.accessibility = accessDist(gen);
        resources.push_back(deposit);
    }
    
    // Additional resources based on body type and habitability
    std::uniform_real_distribution<> chance(0.0, 1.0);
    
    if (habitability > 20 && chance(gen) < 0.7) {
        ResourceDeposit water;
        water.type = ResourceType::WATER_ICE;
        water.abundance = 40 + habitability / 2;
        water.accessibility = 60 + habitability / 3;
        resources.push_back(water);
    }
    
    if (bodyType == "planet" && chance(gen) < 0.3) {
        ResourceDeposit energy;
        energy.type = ResourceType::ENERGY_CRYSTALS;
        energy.abundance = 15 + (rand() % 30);
        energy.accessibility = 20 + (rand() % 40);
        resources.push_back(energy);
    }
    
    return resources;
}

} // namespace space4x
