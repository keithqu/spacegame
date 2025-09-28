// Galaxy generation and visualization types

export interface StarSystem {
  id: string;
  name: string;
  x: number; // Position in light years
  y: number; // Position in light years
  type: 'origin' | 'major' | 'minor' | 'frontier';
  isFixed: boolean; // Whether this system has a fixed position
  connections: string[]; // IDs of connected systems via warp lanes
  // Game properties
  population?: number;
  resources?: {
    minerals: number;
    energy: number;
    research: number;
  };
  owner?: string; // Player ID
  explored: boolean;
}

export interface Anomaly {
  id: string;
  name: string;
  x: number; // Position in light years
  y: number; // Position in light years
  type: 'nebula' | 'blackhole' | 'wormhole' | 'artifact' | 'resource';
  discovered: boolean;
  // Game properties
  effect?: {
    type: string;
    value: number;
  };
}

export interface WarpLane {
  id: string;
  from: string; // System ID
  to: string; // System ID
  distance: number; // Light years
  travelTime: number; // Game turns
  discovered: boolean;
}

export interface GalaxyConfig {
  // Generation parameters
  seed: number;
  radius: number; // Light years
  starSystemCount: number;
  anomalyCount: number;
  
  // Fixed systems with real distances
  fixedSystems: {
    id: string;
    name: string;
    x: number;
    y: number;
    type: StarSystem['type'];
  }[];
  
  // Network generation
  connectivity: {
    minConnections: number; // Minimum connections per system
    maxConnections: number; // Maximum connections per system
    maxDistance: number; // Maximum warp lane distance in light years
    distanceDecayFactor: number; // How connection probability decreases with distance
  };
  
  // Visualization
  visualization: {
    width: number;
    height: number;
    scale: number; // Light years per pixel
  };
}

export interface Galaxy {
  config: GalaxyConfig;
  systems: StarSystem[];
  anomalies: Anomaly[];
  warpLanes: WarpLane[];
  bounds: {
    minX: number;
    maxX: number;
    minY: number;
    maxY: number;
    radius: number;
  };
}

// Default configuration
export const DEFAULT_GALAXY_CONFIG: GalaxyConfig = {
  seed: 42,
  radius: 500, // 500 light year radius
  starSystemCount: 350,
  anomalyCount: 50,
  
  fixedSystems: [
    {
      id: 'sol',
      name: 'Sol System',
      x: 0,
      y: 0,
      type: 'origin'
    },
    {
      id: 'alpha-centauri',
      name: 'Alpha Centauri',
      x: 4.37, // 4.37 light years from Sol
      y: 0,
      type: 'major'
    },
    {
      id: 'tau-ceti',
      name: 'Tau Ceti',
      x: 8.5, // Approximate position relative to Sol and Alpha Centauri
      y: 7.2,
      type: 'major'
    }
  ],
  
  connectivity: {
    minConnections: 1,
    maxConnections: 3,
    maxDistance: 8, // Maximum 8 light year warp lanes (reduced from 10)
    distanceDecayFactor: 0.5 // Higher decay = much lower probability for distant connections
  },
  
  visualization: {
    width: 1200,
    height: 800,
    scale: 3.0 // 3 pixels per light year for better visibility
  }
};

// Pathfinding types
export interface PathNode {
  systemId: string;
  gCost: number; // Distance from start
  hCost: number; // Heuristic distance to goal
  fCost: number; // Total cost
  parent?: PathNode;
}

export interface Path {
  systems: string[];
  totalDistance: number;
  totalTime: number;
  valid: boolean;
}

// Movement types for different game phases
export type MovementType = 'warp-lane' | 'sublight' | 'jump-drive';

export interface MovementOptions {
  type: MovementType;
  maxRange?: number; // For sublight and jump drive
  costMultiplier?: number;
  timeMultiplier?: number;
}
