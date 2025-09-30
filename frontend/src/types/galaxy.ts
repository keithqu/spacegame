// Galaxy generation and visualization types

export interface StarSystem {
  id: string
  name: string
  x: number // Position in light years (always resolved by backend)
  y: number // Position in light years (always resolved by backend)
  type: 'origin' | 'core' | 'rim'
  isFixed: boolean // Whether this system has a fixed position
  connections: string[] // IDs of connected systems via warp lanes
  // Game properties
  population?: number
  gdp?: number // Gross Domestic Product in credits
  resources?: {
    minerals: number
    energy: number
    research: number
  }
  systemInfo?: {
    starType: string // "G-class", "K-class", etc.
    planetCount: number
    moonCount: number
    asteroidCount: number
  }
  hasDetailedData?: boolean // Whether detailed celestial body data is available
  owner?: string // Player ID
  explored: boolean
}

// Strategic resource types
export enum ResourceType {
  MINERALS = 0,
  RARE_METALS = 1,
  ENERGY_CRYSTALS = 2,
  WATER_ICE = 3,
  HELIUM_3 = 4,
  DEUTERIUM = 5,
  ANTIMATTER = 6,
  EXOTIC_MATTER = 7,
}

export interface ResourceDeposit {
  type: ResourceType
  abundance: number // 0-100 scale
  accessibility: number // 0-100 scale (how easy to extract)
}

export interface CelestialBody {
  id: string
  name: string
  type: 'planet' | 'moon' | 'asteroid'
  distanceFromStar?: number // AU for planets from star
  distanceFromPlanet?: number // km for moons from planet
  radius: number // km
  diameter: number // km (2 * radius, but stored separately for clarity)
  mass: number // Earth masses
  gravity: number // Percentage of Earth gravity (100 = 1g)
  habitability: number // Percentage of Earth habitability (100 = fully habitable)
  atmosphere: string // Description of atmosphere
  composition: string // Primary composition
  resources: ResourceDeposit[]
}

export interface Moon extends CelestialBody {
  type: 'moon'
  distanceFromPlanet: number // km from parent planet
}

export interface Planet extends CelestialBody {
  type: 'planet'
  distanceFromStar: number // AU from star
  moons: Moon[]
}

export interface Asteroid extends CelestialBody {
  type: 'asteroid'
  distanceFromStar: number // AU from star
}

export interface DetailedSystemData {
  systemId: string
  systemName: string
  starType: string
  starMass: number // Solar masses
  starRadius: number // Solar radii
  starTemperature: number // Kelvin
  planets: Planet[]
  asteroids: Asteroid[]
}

export interface Anomaly {
  id: string
  name: string
  x: number // Position in light years
  y: number // Position in light years
  type: 'nebula' | 'blackhole' | 'wormhole' | 'artifact' | 'resource'
  discovered: boolean
  // Game properties
  effect?: {
    type: string
    value: number
  }
}

export interface WarpLane {
  id: string
  from: string // System ID
  to: string // System ID
  distance: number // Light years
  travelTime: number // Game turns
  discovered: boolean
}

export interface GalaxyConfig {
  // Generation parameters
  seed: number
  radius: number // Light years
  starSystemCount: number
  anomalyCount: number

  // Fixed systems with real distances
  fixedSystems: {
    id: string
    name: string
    x: number
    y: number
    type: StarSystem['type']
    // Optional distance constraints for fictional systems
    distance?: number // target distance from origin
    distanceTolerance?: number // allowed deviation from target distance
  }[]

  // Network generation
  connectivity: {
    minConnections: number // Minimum connections per system
    maxConnections: number // Maximum connections per system
    maxDistance: number // Maximum warp lane distance in light years
    distanceDecayFactor: number // How connection probability decreases with distance
  }

  // Visualization
  visualization: {
    width: number
    height: number
    scale: number // Light years per pixel
  }
}

export interface Galaxy {
  config: GalaxyConfig
  systems: StarSystem[]
  anomalies: Anomaly[]
  warpLanes: WarpLane[]
  bounds: {
    minX: number
    maxX: number
    minY: number
    maxY: number
    radius: number
  }
}

// Default configuration with automatic scaling
export const DEFAULT_GALAXY_CONFIG: GalaxyConfig = {
  seed: 1111111111,
  radius: 500, // 500 light year radius for full galaxy experience
  starSystemCount: 400, // Full-scale galaxy with 400 systems
  anomalyCount: 25, // 25 anomalies for variety

  fixedSystems: [
    {
      id: 'sol',
      name: 'Sol System',
      x: 0,
      y: 0,
      type: 'origin',
    },
    {
      id: 'alpha-centauri',
      name: 'Alpha Centauri',
      x: 4.37, // 4.37 light years from Sol
      y: 0,
      type: 'core',
    },
    {
      id: 'tau-ceti',
      name: 'Tau Ceti',
      x: -7.8, // 11.9 light years from Sol, different direction
      y: 9.1,
      type: 'core',
    },
    {
      id: 'barnards-star',
      name: "Barnard's Star",
      x: 2.1, // 5.96 light years from Sol
      y: -5.6,
      type: 'core',
    },
    {
      id: 'bellatrix',
      name: 'Bellatrix',
      x: 180, // ~245 light years from Sol
      y: 165,
      type: 'rim',
    },
    {
      id: 'lumiere',
      name: 'Lumière',
      x: 0, // Distance-constrained: 250 ± 20 LY
      y: 0,
      distance: 250,
      distanceTolerance: 20,
      type: 'rim',
    },
    {
      id: 'aspida',
      name: 'Aspida',
      x: 0, // Distance-constrained: 350 ± 20 LY
      y: 0,
      distance: 350,
      distanceTolerance: 20,
      type: 'rim',
    },
  ],

  connectivity: {
    minConnections: 1,
    maxConnections: 3,
    maxDistance: 8, // Maximum 8 light year warp lanes (reduced from 10)
    distanceDecayFactor: 0.5, // Higher decay = much lower probability for distant connections
  },

  visualization: {
    width: 1200,
    height: 800,
    scale: 12.0, // 12 pixels per light year for truly massive galaxy feel
  },
}

// Pathfinding types
export interface PathNode {
  systemId: string
  gCost: number // Distance from start
  hCost: number // Heuristic distance to goal
  fCost: number // Total cost
  parent?: PathNode
}

export interface Path {
  systems: string[]
  totalDistance: number
  totalTime: number
  valid: boolean
}

// Movement types for different game phases
export type MovementType = 'warp-lane' | 'sublight' | 'jump-drive'

export interface MovementOptions {
  type: MovementType
  maxRange?: number // For sublight and jump drive
  costMultiplier?: number
  timeMultiplier?: number
}
