#include "celestial_bodies.h"
#include <random>
#include <cmath>
#include <iostream>

namespace space4x {

SystemConfigManager::SystemConfigManager() {
    loadPredefinedSystems();
}

void SystemConfigManager::loadPredefinedSystems() {
    defineSolSystem();
    std::cout << "✅ Loaded " << predefinedSystems.size() << " predefined star systems" << std::endl;
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

void SystemConfigManager::defineSolSystem() {
    SystemDefinition sol;
    sol.systemId = "sol";
    sol.systemName = "Sol System";
    sol.starType = "G-class";
    sol.starMass = 1.0;
    sol.starRadius = 1.0;
    sol.starTemperature = 5778;
    
    // Mercury
    Planet mercury;
    mercury.id = "mercury";
    mercury.name = "Mercury";
    mercury.distanceFromParent = 0.39;
    mercury.radius = 2439.7;
    mercury.diameter = 4879.4; // 2 * radius
    mercury.mass = 0.055;
    mercury.gravity = 38;
    mercury.habitability = 0;
    mercury.atmosphere = "Extremely thin (oxygen, sodium, hydrogen, helium, potassium)";
    mercury.composition = "Iron core, silicate mantle and crust";
    mercury.resources = {
        {ResourceType::MINERALS, 85, 70},
        {ResourceType::RARE_METALS, 60, 45},
        {ResourceType::ENERGY_CRYSTALS, 25, 30}
    };
    
    // Venus
    Planet venus;
    venus.id = "venus";
    venus.name = "Venus";
    venus.distanceFromParent = 0.72;
    venus.radius = 6051.8;
    venus.diameter = 12103.6; // 2 * radius
    venus.mass = 0.815;
    venus.gravity = 91;
    venus.habitability = 0;
    venus.atmosphere = "Dense carbon dioxide with sulfuric acid clouds";
    venus.composition = "Iron core, silicate mantle and crust";
    venus.resources = {
        {ResourceType::MINERALS, 75, 25},
        {ResourceType::RARE_METALS, 40, 20},
        {ResourceType::ENERGY_CRYSTALS, 35, 15}
    };
    
    // Earth
    Planet earth;
    earth.id = "earth";
    earth.name = "Earth";
    earth.distanceFromParent = 1.0;
    earth.radius = 6371.0;
    earth.diameter = 12742.0; // 2 * radius
    earth.mass = 1.0;
    earth.gravity = 100;
    earth.habitability = 100;
    earth.atmosphere = "Nitrogen-oxygen with trace gases";
    earth.composition = "Iron core, silicate mantle and crust, 71% water surface";
    earth.resources = {
        {ResourceType::MINERALS, 60, 85},
        {ResourceType::RARE_METALS, 45, 80},
        {ResourceType::WATER_ICE, 95, 95},
        {ResourceType::DEUTERIUM, 30, 60}
    };
    
    // Earth's Moon
    Moon luna;
    luna.id = "luna";
    luna.name = "Luna";
    luna.distanceFromParent = 384400; // km
    luna.radius = 1737.4;
    luna.diameter = 3474.8; // 2 * radius
    luna.mass = 0.012;
    luna.gravity = 17;
    luna.habitability = 0;
    luna.atmosphere = "Extremely thin (argon, neon, hydrogen, helium)";
    luna.composition = "Iron core, silicate mantle and crust";
    luna.resources = {
        {ResourceType::MINERALS, 70, 75},
        {ResourceType::RARE_METALS, 35, 60},
        {ResourceType::HELIUM_3, 80, 70},
        {ResourceType::WATER_ICE, 40, 50}
    };
    earth.moons.push_back(luna);
    
    // Mars (Terraformed)
    Planet mars;
    mars.id = "mars";
    mars.name = "Mars";
    mars.distanceFromParent = 1.52;
    mars.radius = 3389.5;
    mars.diameter = 6779.0; // 2 * radius
    mars.mass = 0.107;
    mars.gravity = 45; // Increased from 38% due to terraforming efforts
    mars.habitability = 75; // Terraformed
    mars.atmosphere = "Thickened nitrogen-oxygen with CO2 (terraformed)";
    mars.composition = "Iron core, silicate mantle and crust";
    mars.resources = {
        {ResourceType::MINERALS, 80, 80},
        {ResourceType::RARE_METALS, 55, 70},
        {ResourceType::WATER_ICE, 85, 85},
        {ResourceType::DEUTERIUM, 25, 65}
    };
    
    // Mars moons
    Moon phobos;
    phobos.id = "phobos";
    phobos.name = "Phobos";
    phobos.distanceFromParent = 9376;
    phobos.radius = 11.3;
    phobos.diameter = 22.6; // 2 * radius
    phobos.mass = 0.000000018;
    phobos.gravity = 0.6;
    phobos.habitability = 0;
    phobos.atmosphere = "None";
    phobos.composition = "Carbonaceous chondrite";
    phobos.resources = {
        {ResourceType::MINERALS, 65, 85},
        {ResourceType::RARE_METALS, 30, 75}
    };
    
    Moon deimos;
    deimos.id = "deimos";
    deimos.name = "Deimos";
    deimos.distanceFromParent = 23463;
    deimos.radius = 6.2;
    deimos.diameter = 12.4; // 2 * radius
    deimos.mass = 0.000000002;
    deimos.gravity = 0.3;
    deimos.habitability = 0;
    deimos.atmosphere = "None";
    deimos.composition = "Carbonaceous chondrite";
    deimos.resources = {
        {ResourceType::MINERALS, 60, 80},
        {ResourceType::RARE_METALS, 25, 70}
    };
    
    mars.moons.push_back(phobos);
    mars.moons.push_back(deimos);
    
    // Jupiter
    Planet jupiter;
    jupiter.id = "jupiter";
    jupiter.name = "Jupiter";
    jupiter.distanceFromParent = 5.20;
    jupiter.radius = 69911;
    jupiter.diameter = 139822; // 2 * radius
    jupiter.mass = 317.8;
    jupiter.gravity = 236;
    jupiter.habitability = 0;
    jupiter.atmosphere = "Hydrogen and helium with trace compounds";
    jupiter.composition = "Hydrogen and helium gas giant";
    jupiter.resources = {
        {ResourceType::HELIUM_3, 95, 40},
        {ResourceType::DEUTERIUM, 90, 45},
        {ResourceType::ENERGY_CRYSTALS, 15, 10}
    };
    
    // Jupiter's major moons
    Moon io;
    io.id = "io";
    io.name = "Io";
    io.distanceFromParent = 421700;
    io.radius = 1821.6;
    io.diameter = 3643.2; // 2 * radius
    io.mass = 0.015;
    io.gravity = 18;
    io.habitability = 0;
    io.atmosphere = "Extremely thin sulfur dioxide";
    io.composition = "Silicate rock with iron core";
    io.resources = {
        {ResourceType::MINERALS, 90, 60},
        {ResourceType::RARE_METALS, 70, 50},
        {ResourceType::ENERGY_CRYSTALS, 45, 35}
    };
    
    Moon europa;
    europa.id = "europa";
    europa.name = "Europa";
    europa.distanceFromParent = 671034;
    europa.radius = 1560.8;
    europa.diameter = 3121.6; // 2 * radius
    europa.mass = 0.008;
    europa.gravity = 13;
    europa.habitability = 25; // Subsurface ocean
    europa.atmosphere = "Extremely thin oxygen";
    europa.composition = "Water ice over silicate interior";
    europa.resources = {
        {ResourceType::WATER_ICE, 95, 80},
        {ResourceType::DEUTERIUM, 60, 70},
        {ResourceType::MINERALS, 50, 40},
        {ResourceType::EXOTIC_MATTER, 20, 15}
    };
    
    Moon ganymede;
    ganymede.id = "ganymede";
    ganymede.name = "Ganymede";
    ganymede.distanceFromParent = 1070412;
    ganymede.radius = 2634.1;
    ganymede.diameter = 5268.2; // 2 * radius
    ganymede.mass = 0.025;
    ganymede.gravity = 15;
    ganymede.habitability = 15; // Subsurface ocean
    ganymede.atmosphere = "Extremely thin oxygen";
    ganymede.composition = "Water ice and silicate rock";
    ganymede.resources = {
        {ResourceType::WATER_ICE, 90, 85},
        {ResourceType::MINERALS, 65, 70},
        {ResourceType::RARE_METALS, 40, 60},
        {ResourceType::DEUTERIUM, 45, 75}
    };
    
    Moon callisto;
    callisto.id = "callisto";
    callisto.name = "Callisto";
    callisto.distanceFromParent = 1882709;
    callisto.radius = 2410.3;
    callisto.diameter = 4820.6; // 2 * radius
    callisto.mass = 0.018;
    callisto.gravity = 13;
    callisto.habitability = 10; // Possible subsurface ocean
    callisto.atmosphere = "Extremely thin carbon dioxide";
    callisto.composition = "Water ice and silicate rock";
    callisto.resources = {
        {ResourceType::WATER_ICE, 85, 80},
        {ResourceType::MINERALS, 60, 65},
        {ResourceType::RARE_METALS, 35, 55}
    };
    
    jupiter.moons = {io, europa, ganymede, callisto};
    
    // Saturn
    Planet saturn;
    saturn.id = "saturn";
    saturn.name = "Saturn";
    saturn.distanceFromParent = 9.58;
    saturn.radius = 58232;
    saturn.diameter = 116464; // 2 * radius
    saturn.mass = 95.2;
    saturn.gravity = 91;
    saturn.habitability = 0;
    saturn.atmosphere = "Hydrogen and helium with trace compounds";
    saturn.composition = "Hydrogen and helium gas giant";
    saturn.resources = {
        {ResourceType::HELIUM_3, 90, 35},
        {ResourceType::DEUTERIUM, 85, 40},
        {ResourceType::ENERGY_CRYSTALS, 20, 12}
    };
    
    // Saturn's major moons
    Moon titan;
    titan.id = "titan";
    titan.name = "Titan";
    titan.distanceFromParent = 1221830;
    titan.radius = 2574;
    titan.diameter = 5148; // 2 * radius
    titan.mass = 0.023;
    titan.gravity = 14;
    titan.habitability = 30; // Dense atmosphere, liquid methane
    titan.atmosphere = "Dense nitrogen with methane";
    titan.composition = "Water ice and silicate rock";
    titan.resources = {
        {ResourceType::WATER_ICE, 80, 70},
        {ResourceType::DEUTERIUM, 70, 80},
        {ResourceType::RARE_METALS, 45, 55},
        {ResourceType::EXOTIC_MATTER, 25, 20}
    };
    
    Moon enceladus;
    enceladus.id = "enceladus";
    enceladus.name = "Enceladus";
    enceladus.distanceFromParent = 238020;
    enceladus.radius = 252.1;
    enceladus.diameter = 504.2; // 2 * radius
    enceladus.mass = 0.0002;
    enceladus.gravity = 1;
    enceladus.habitability = 20; // Subsurface ocean
    enceladus.atmosphere = "Extremely thin water vapor";
    enceladus.composition = "Water ice over silicate core";
    enceladus.resources = {
        {ResourceType::WATER_ICE, 95, 85},
        {ResourceType::DEUTERIUM, 55, 75},
        {ResourceType::EXOTIC_MATTER, 30, 25}
    };
    
    saturn.moons = {titan, enceladus};
    
    // Uranus
    Planet uranus;
    uranus.id = "uranus";
    uranus.name = "Uranus";
    uranus.distanceFromParent = 19.2;
    uranus.radius = 25362;
    uranus.diameter = 50724; // 2 * radius
    uranus.mass = 14.5;
    uranus.gravity = 89;
    uranus.habitability = 0;
    uranus.atmosphere = "Hydrogen, helium, and methane";
    uranus.composition = "Water, methane, and ammonia ices over rock core";
    uranus.resources = {
        {ResourceType::HELIUM_3, 75, 25},
        {ResourceType::DEUTERIUM, 70, 30},
        {ResourceType::RARE_METALS, 40, 20}
    };
    
    // Neptune
    Planet neptune;
    neptune.id = "neptune";
    neptune.name = "Neptune";
    neptune.distanceFromParent = 30.1;
    neptune.radius = 24622;
    neptune.diameter = 49244; // 2 * radius
    neptune.mass = 17.1;
    neptune.gravity = 113;
    neptune.habitability = 0;
    neptune.atmosphere = "Hydrogen, helium, and methane";
    neptune.composition = "Water, methane, and ammonia ices over rock core";
    neptune.resources = {
        {ResourceType::HELIUM_3, 80, 20},
        {ResourceType::DEUTERIUM, 75, 25},
        {ResourceType::RARE_METALS, 45, 15},
        {ResourceType::ANTIMATTER, 10, 5}
    };
    
    // Add all planets to Sol system
    sol.planets = {mercury, venus, earth, mars, jupiter, saturn, uranus, neptune};
    
    // Major asteroids and asteroid belt
    Asteroid ceres;
    ceres.id = "ceres";
    ceres.name = "Ceres";
    ceres.distanceFromParent = 2.77;
    ceres.radius = 473;
    ceres.diameter = 946; // 2 * radius
    ceres.mass = 0.00016;
    ceres.gravity = 3;
    ceres.habitability = 0;
    ceres.atmosphere = "Extremely thin water vapor";
    ceres.composition = "Water ice and silicate rock";
    ceres.resources = {
        {ResourceType::WATER_ICE, 85, 90},
        {ResourceType::MINERALS, 75, 85},
        {ResourceType::RARE_METALS, 50, 80}
    };
    
    Asteroid vesta;
    vesta.id = "vesta";
    vesta.name = "Vesta";
    vesta.distanceFromParent = 2.36;
    vesta.radius = 262.7;
    vesta.diameter = 525.4; // 2 * radius
    vesta.mass = 0.000044;
    vesta.gravity = 3;
    vesta.habitability = 0;
    vesta.atmosphere = "None";
    vesta.composition = "Basaltic crust over differentiated interior";
    vesta.resources = {
        {ResourceType::MINERALS, 90, 90},
        {ResourceType::RARE_METALS, 70, 85},
        {ResourceType::ENERGY_CRYSTALS, 35, 70}
    };
    
    Asteroid pallas;
    pallas.id = "pallas";
    pallas.name = "Pallas";
    pallas.distanceFromParent = 2.77;
    pallas.radius = 272;
    pallas.diameter = 544; // 2 * radius
    pallas.mass = 0.000036;
    pallas.gravity = 2;
    pallas.habitability = 0;
    pallas.atmosphere = "None";
    pallas.composition = "Silicate rock";
    pallas.resources = {
        {ResourceType::MINERALS, 85, 85},
        {ResourceType::RARE_METALS, 60, 80}
    };
    
    sol.asteroids = {ceres, vesta, pallas};
    
    predefinedSystems["sol"] = sol;
}

SystemDefinition SystemConfigManager::generateRandomSystem(const std::string& systemId, const std::string& systemName) {
    SystemDefinition system;
    system.systemId = systemId;
    system.systemName = systemName;
    
    // Random star properties
    std::vector<std::string> starTypes = {"G-class", "K-class", "M-class", "F-class", "A-class"};
    std::random_device rd;
    std::mt19937 gen(rd());
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
        Planet planet = generateRandomPlanet(i, currentDistance);
        system.planets.push_back(planet);
        
        // Increase distance for next planet
        std::uniform_real_distribution<> distanceIncrease(1.3, 2.2);
        currentDistance *= distanceIncrease(gen);
    }
    
    return system;
}

Planet SystemConfigManager::generateRandomPlanet(int planetIndex, double distanceFromStar) {
    Planet planet;
    planet.id = "planet-" + std::to_string(planetIndex + 1);
    planet.name = "Planet " + std::to_string(planetIndex + 1);
    planet.distanceFromParent = distanceFromStar;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
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
            Moon moon = generateRandomMoon(i, planet);
            planet.moons.push_back(moon);
        }
    }
    
    return planet;
}

Moon SystemConfigManager::generateRandomMoon(int moonIndex, const Planet& parentPlanet) {
    Moon moon;
    moon.id = parentPlanet.id + "-moon-" + std::to_string(moonIndex + 1);
    moon.name = parentPlanet.name + " Moon " + std::to_string(moonIndex + 1);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
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
