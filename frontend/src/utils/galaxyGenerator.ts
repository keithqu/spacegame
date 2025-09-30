import {
  Galaxy,
  GalaxyConfig,
  StarSystem,
  Anomaly,
  WarpLane,
  DEFAULT_GALAXY_CONFIG,
} from '../types/galaxy'

// Seeded random number generator for consistent galaxy generation
class SeededRandom {
  private seed: number

  constructor(seed: number) {
    this.seed = seed
  }

  // Linear congruential generator
  next(): number {
    this.seed = (this.seed * 1664525 + 1013904223) % 4294967296
    return this.seed / 4294967296
  }

  // Random number between min and max
  range(min: number, max: number): number {
    return min + this.next() * (max - min)
  }

  // Random integer between min and max (inclusive)
  int(min: number, max: number): number {
    return Math.floor(this.range(min, max + 1))
  }

  // Random boolean with given probability
  boolean(probability: number = 0.5): boolean {
    return this.next() < probability
  }
}

export class GalaxyGenerator {
  private config: GalaxyConfig
  private random: SeededRandom

  constructor(config: Partial<GalaxyConfig> = {}) {
    this.config = { ...DEFAULT_GALAXY_CONFIG, ...config }
    this.random = new SeededRandom(this.config.seed)
  }

  generateGalaxy(): Galaxy {
    console.log('🌌 Generating galaxy with config:', this.config)

    // Generate star systems
    const systems = this.generateStarSystems()

    // Generate anomalies (avoiding star systems)
    const anomalies = this.generateAnomalies(systems)

    // Generate warp lane network
    const warpLanes = this.generateWarpLanes(systems)

    const galaxy: Galaxy = {
      config: this.config,
      systems,
      anomalies,
      warpLanes,
      bounds: {
        minX: -this.config.radius,
        maxX: this.config.radius,
        minY: -this.config.radius,
        maxY: this.config.radius,
        radius: this.config.radius,
      },
    }

    // Calculate connectivity stats
    const avgConnections =
      systems.reduce((sum, s) => sum + s.connections.length, 0) / systems.length
    const maxDistance = Math.max(...warpLanes.map(w => w.distance))
    const avgDistance = warpLanes.reduce((sum, w) => sum + w.distance, 0) / warpLanes.length

    console.log(
      `✅ Generated galaxy: ${systems.length} systems, ${anomalies.length} anomalies, ${warpLanes.length} warp lanes`
    )
    console.log(
      `📊 Connectivity: ${avgConnections.toFixed(1)} avg connections, ${maxDistance.toFixed(1)} max distance, ${avgDistance.toFixed(1)} avg distance`
    )

    // Show distance distribution
    const shortLanes = warpLanes.filter(w => w.distance <= 4).length
    const mediumLanes = warpLanes.filter(w => w.distance > 4 && w.distance <= 6).length
    const longLanes = warpLanes.filter(w => w.distance > 6).length
    console.log(
      `📏 Distance distribution: ${shortLanes} short (≤4 LY), ${mediumLanes} medium (4-6 LY), ${longLanes} long (>6 LY)`
    )
    return galaxy
  }

  private generateStarSystems(): StarSystem[] {
    const systems: StarSystem[] = []

    // Add fixed systems first
    this.config.fixedSystems.forEach(fixedSystem => {
      systems.push({
        id: fixedSystem.id,
        name: fixedSystem.name,
        x: fixedSystem.x,
        y: fixedSystem.y,
        type: fixedSystem.type,
        isFixed: true,
        connections: [],
        explored: fixedSystem.type === 'origin', // Origin system starts explored
        population: fixedSystem.type === 'origin' ? 1000000 : 0,
        resources: {
          minerals: this.random.int(50, 200),
          energy: this.random.int(50, 200),
          research: this.random.int(50, 200),
        },
      })
    })

    // Generate remaining random systems
    const remainingSystems = this.config.starSystemCount - this.config.fixedSystems.length

    for (let i = 0; i < remainingSystems; i++) {
      let position: { x: number; y: number }
      let attempts = 0

      // Generate position within circle, avoiding conflicts
      do {
        position = this.generateRandomPositionInCircle()
        attempts++
      } while (
        attempts < 100 &&
        this.isPositionTooCloseToSystems(position, systems, 2.0) // Minimum 2 light years apart
      )

      const systemType = this.determineSystemType(position)

      systems.push({
        id: `system-${i + 1}`,
        name: this.generateSystemName(i + 1),
        x: position.x,
        y: position.y,
        type: systemType,
        isFixed: false,
        connections: [],
        explored: false,
        population: 0,
        resources: {
          minerals: this.random.int(10, 150),
          energy: this.random.int(10, 150),
          research: this.random.int(10, 150),
        },
      })
    }

    return systems
  }

  private generateAnomalies(systems: StarSystem[]): Anomaly[] {
    const anomalies: Anomaly[] = []

    for (let i = 0; i < this.config.anomalyCount; i++) {
      let position: { x: number; y: number }
      let attempts = 0

      // Generate position within circle, avoiding star systems and other anomalies
      do {
        position = this.generateRandomPositionInCircle()
        attempts++
      } while (
        attempts < 100 &&
        (this.isPositionTooCloseToSystems(position, systems, 3.0) || // Keep 3 light years from systems
          this.isPositionTooClose(position, anomalies, 2.0)) // Keep 2 light years from other anomalies
      )

      const anomalyType = this.generateAnomalyType()

      anomalies.push({
        id: `anomaly-${i + 1}`,
        name: this.generateAnomalyName(anomalyType, i + 1),
        x: position.x,
        y: position.y,
        type: anomalyType,
        discovered: false,
        effect: this.generateAnomalyEffect(anomalyType),
      })
    }

    return anomalies
  }

  private generateWarpLanes(systems: StarSystem[]): WarpLane[] {
    const warpLanes: WarpLane[] = []
    const connections = new Map<string, Set<string>>()

    // Initialize connection tracking
    systems.forEach(system => {
      connections.set(system.id, new Set())
    })

    // Phase 1: Create initial connections based on proximity and rules
    systems.forEach(system => {
      const currentConnections = connections.get(system.id)!

      // Find all systems within max distance
      const candidates = systems
        .filter(other => other.id !== system.id)
        .filter(other => !currentConnections.has(other.id))
        .map(other => ({
          system: other,
          distance: this.calculateDistance(system, other),
        }))
        .filter(candidate => candidate.distance <= this.config.connectivity.maxDistance)
        .sort((a, b) => a.distance - b.distance)

      // Determine target connections (not based on distance from origin)
      const targetConnections = this.random.int(
        this.config.connectivity.minConnections,
        this.config.connectivity.maxConnections
      )

      // Add connections with distance-based probability (heavily favoring close systems)
      for (const candidate of candidates) {
        if (currentConnections.size >= targetConnections) break

        // Much higher probability for closer systems, very low for distant ones
        const normalizedDistance = candidate.distance / this.config.connectivity.maxDistance
        const probability = Math.exp(
          -normalizedDistance * this.config.connectivity.distanceDecayFactor
        )

        // Additional distance penalty - systems beyond 6 LY have much lower chance
        const distancePenalty = candidate.distance > 6 ? 0.3 : 1.0
        const finalProbability = probability * distancePenalty

        if (this.random.next() < finalProbability) {
          this.createWarpLane(system, candidate.system, candidate.distance, warpLanes, connections)
        }
      }
    })

    // Phase 2: Ensure all systems have at least one connection
    this.ensureMinimumConnectivity(systems, warpLanes, connections)

    // Phase 3: Ensure network connectivity (all systems reachable)
    this.ensureNetworkConnectivity(systems, warpLanes, connections)

    return warpLanes
  }

  private createWarpLane(
    system1: StarSystem,
    system2: StarSystem,
    distance: number,
    warpLanes: WarpLane[],
    connections: Map<string, Set<string>>
  ): void {
    // Avoid duplicate connections
    if (connections.get(system1.id)!.has(system2.id)) return

    const laneId = `${system1.id}-${system2.id}`

    // Update connection tracking
    connections.get(system1.id)!.add(system2.id)
    connections.get(system2.id)!.add(system1.id)

    // Update system connections
    system1.connections.push(system2.id)
    system2.connections.push(system1.id)

    // Create warp lane
    warpLanes.push({
      id: laneId,
      from: system1.id,
      to: system2.id,
      distance,
      travelTime: Math.ceil(distance / 5), // 5 light years per turn (faster than before)
      discovered: system1.explored && system2.explored,
    })
  }

  private ensureMinimumConnectivity(
    systems: StarSystem[],
    warpLanes: WarpLane[],
    connections: Map<string, Set<string>>
  ): void {
    // Find systems with no connections
    const isolatedSystems = systems.filter(system => connections.get(system.id)!.size === 0)

    for (const system of isolatedSystems) {
      // Find nearest system within reasonable distance
      const candidates = systems
        .filter(other => other.id !== system.id)
        .map(other => ({
          system: other,
          distance: this.calculateDistance(system, other),
        }))
        .sort((a, b) => a.distance - b.distance)

      // Connect to nearest system (even if beyond normal max distance)
      if (candidates.length > 0) {
        const nearest = candidates[0]
        // Allow connection up to 15 LY for isolated systems (reduced from 20)
        if (nearest.distance <= 15) {
          this.createWarpLane(system, nearest.system, nearest.distance, warpLanes, connections)
          console.log(
            `🔗 Connected isolated system ${system.name} to ${nearest.system.name} (${nearest.distance.toFixed(1)} LY)`
          )
        }
      }
    }
  }

  private generateRandomPositionInCircle(): { x: number; y: number } {
    // Generate random point within circle using polar coordinates
    const angle = this.random.range(0, 2 * Math.PI)
    const radius = Math.sqrt(this.random.next()) * this.config.radius

    return {
      x: radius * Math.cos(angle),
      y: radius * Math.sin(angle),
    }
  }

  private isPositionTooClose(
    position: { x: number; y: number },
    existingObjects: { x: number; y: number }[],
    minDistance: number
  ): boolean {
    return existingObjects.some(obj => this.calculateDistance(position, obj) < minDistance)
  }

  private isPositionTooCloseToSystems(
    position: { x: number; y: number },
    existingSystems: StarSystem[],
    minDistance: number
  ): boolean {
    return existingSystems.some(system => this.calculateDistance(position, system) < minDistance)
  }

  private calculateDistance(a: { x: number; y: number }, b: { x: number; y: number }): number {
    const dx = a.x - b.x
    const dy = a.y - b.y
    return Math.sqrt(dx * dx + dy * dy)
  }

  private determineSystemType(position: { x: number; y: number }): StarSystem['type'] {
    const distanceFromOrigin = Math.sqrt(position.x * position.x + position.y * position.y)
    const normalizedDistance = distanceFromOrigin / this.config.radius

    if (normalizedDistance < 0.7) return 'core'
    return 'rim'
  }

  private generateSystemName(index: number): string {
    const prefixes = ['Alpha', 'Beta', 'Gamma', 'Delta', 'Epsilon', 'Zeta', 'Eta', 'Theta']
    const suffixes = ['Centauri', 'Draconis', 'Leonis', 'Aquarii', 'Orionis', 'Cygni', 'Lyrae']

    const prefix = prefixes[index % prefixes.length]
    const suffix = suffixes[Math.floor(index / prefixes.length) % suffixes.length]

    return `${prefix} ${suffix}`
  }

  private generateAnomalyType(): Anomaly['type'] {
    const types: Anomaly['type'][] = ['nebula', 'blackhole', 'wormhole', 'artifact', 'resource']
    const weights = [0.4, 0.1, 0.1, 0.2, 0.2] // Nebulae are most common

    const random = this.random.next()
    let cumulative = 0

    for (let i = 0; i < types.length; i++) {
      cumulative += weights[i]
      if (random < cumulative) {
        return types[i]
      }
    }

    return 'nebula'
  }

  private generateAnomalyName(type: Anomaly['type'], index: number): string {
    const typeNames = {
      nebula: ['Crimson Nebula', 'Azure Cloud', 'Stellar Nursery', 'Dark Nebula'],
      blackhole: ['Void Maw', 'Event Horizon', 'Singularity', 'Dark Star'],
      wormhole: ['Quantum Gate', 'Space Fold', 'Dimensional Rift', 'Warp Tunnel'],
      artifact: ['Ancient Relic', 'Precursor Site', 'Mysterious Structure', 'Alien Beacon'],
      resource: ['Asteroid Field', 'Resource Cluster', 'Mining Zone', 'Rare Elements'],
    }

    const names = typeNames[type]
    return `${names[index % names.length]} ${Math.floor(index / names.length) + 1}`
  }

  private generateAnomalyEffect(type: Anomaly['type']) {
    switch (type) {
      case 'nebula':
        return { type: 'sensor_interference', value: -0.5 }
      case 'blackhole':
        return { type: 'gravity_well', value: 2.0 }
      case 'wormhole':
        return { type: 'fast_travel', value: 0.1 }
      case 'artifact':
        return { type: 'research_bonus', value: 1.5 }
      case 'resource':
        return { type: 'mining_bonus', value: 2.0 }
      default:
        return { type: 'none', value: 0 }
    }
  }

  private ensureNetworkConnectivity(
    systems: StarSystem[],
    warpLanes: WarpLane[],
    connections: Map<string, Set<string>>
  ): void {
    // Use Union-Find to detect disconnected components
    const parent = new Map<string, string>()
    const rank = new Map<string, number>()

    // Initialize Union-Find
    systems.forEach(system => {
      parent.set(system.id, system.id)
      rank.set(system.id, 0)
    })

    const find = (id: string): string => {
      if (parent.get(id) !== id) {
        parent.set(id, find(parent.get(id)!))
      }
      return parent.get(id)!
    }

    const union = (a: string, b: string): void => {
      const rootA = find(a)
      const rootB = find(b)

      if (rootA !== rootB) {
        const rankA = rank.get(rootA)!
        const rankB = rank.get(rootB)!

        if (rankA < rankB) {
          parent.set(rootA, rootB)
        } else if (rankA > rankB) {
          parent.set(rootB, rootA)
        } else {
          parent.set(rootB, rootA)
          rank.set(rootA, rankA + 1)
        }
      }
    }

    // Add existing connections to Union-Find
    warpLanes.forEach(lane => {
      union(lane.from, lane.to)
    })

    // Find disconnected components and connect them
    const components = new Map<string, string[]>()
    systems.forEach(system => {
      const root = find(system.id)
      if (!components.has(root)) {
        components.set(root, [])
      }
      components.get(root)!.push(system.id)
    })

    // If there are multiple components, connect them
    const componentRoots = Array.from(components.keys())
    for (let i = 1; i < componentRoots.length; i++) {
      const component1 = components.get(componentRoots[0])!
      const component2 = components.get(componentRoots[i])!

      // Find closest systems between components
      let minDistance = Infinity
      let bestConnection: { from: StarSystem; to: StarSystem } | null = null

      for (const id1 of component1) {
        for (const id2 of component2) {
          const system1 = systems.find(s => s.id === id1)!
          const system2 = systems.find(s => s.id === id2)!
          const distance = this.calculateDistance(system1, system2)

          if (distance < minDistance) {
            minDistance = distance
            bestConnection = { from: system1, to: system2 }
          }
        }
      }

      // Create connection
      if (bestConnection) {
        const laneId = `${bestConnection.from.id}-${bestConnection.to.id}`

        bestConnection.from.connections.push(bestConnection.to.id)
        bestConnection.to.connections.push(bestConnection.from.id)

        warpLanes.push({
          id: laneId,
          from: bestConnection.from.id,
          to: bestConnection.to.id,
          distance: minDistance,
          travelTime: Math.ceil(minDistance / 10),
          discovered: false,
        })

        union(bestConnection.from.id, bestConnection.to.id)
      }
    }
  }
}
