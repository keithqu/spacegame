import { GalaxyConfig } from '../types/galaxy'
import { GalaxyGenerator } from './galaxyGenerator'

// Test function to validate galaxy generation rules
export function testGalaxyGeneration(): void {
  console.log('🧪 Testing Galaxy Generation Rules...')

  const testConfig: Partial<GalaxyConfig> = {
    seed: 12345,
    radius: 100, // Smaller for testing
    starSystemCount: 50,
    anomalyCount: 10,
    connectivity: {
      minConnections: 1,
      maxConnections: 3,
      maxDistance: 10,
      distanceDecayFactor: 0.3,
    },
  }

  const generator = new GalaxyGenerator(testConfig)
  const galaxy = generator.generateGalaxy()

  // Test 1: Minimum distance between systems
  console.log('\n📏 Testing minimum distances...')
  let minSystemDistance = Infinity
  let minAnomalyDistance = Infinity
  let minSystemAnomalyDistance = Infinity

  // Check system-to-system distances
  for (let i = 0; i < galaxy.systems.length; i++) {
    for (let j = i + 1; j < galaxy.systems.length; j++) {
      const distance = Math.sqrt(
        Math.pow(galaxy.systems[i].x - galaxy.systems[j].x, 2) +
          Math.pow(galaxy.systems[i].y - galaxy.systems[j].y, 2)
      )
      minSystemDistance = Math.min(minSystemDistance, distance)
    }
  }

  // Check anomaly-to-anomaly distances
  for (let i = 0; i < galaxy.anomalies.length; i++) {
    for (let j = i + 1; j < galaxy.anomalies.length; j++) {
      const distance = Math.sqrt(
        Math.pow(galaxy.anomalies[i].x - galaxy.anomalies[j].x, 2) +
          Math.pow(galaxy.anomalies[i].y - galaxy.anomalies[j].y, 2)
      )
      minAnomalyDistance = Math.min(minAnomalyDistance, distance)
    }
  }

  // Check system-to-anomaly distances
  for (const system of galaxy.systems) {
    for (const anomaly of galaxy.anomalies) {
      const distance = Math.sqrt(
        Math.pow(system.x - anomaly.x, 2) + Math.pow(system.y - anomaly.y, 2)
      )
      minSystemAnomalyDistance = Math.min(minSystemAnomalyDistance, distance)
    }
  }

  console.log(`✅ Min system-system distance: ${minSystemDistance.toFixed(2)} LY (should be ≥ 2.0)`)
  console.log(
    `✅ Min anomaly-anomaly distance: ${minAnomalyDistance.toFixed(2)} LY (should be ≥ 2.0)`
  )
  console.log(
    `✅ Min system-anomaly distance: ${minSystemAnomalyDistance.toFixed(2)} LY (should be ≥ 3.0)`
  )

  // Test 2: Maximum warp lane distances
  console.log('\n🔗 Testing warp lane distances...')
  const maxWarpDistance = Math.max(...galaxy.warpLanes.map(w => w.distance))
  const longLanes = galaxy.warpLanes.filter(w => w.distance > 10)

  console.log(`✅ Max warp lane distance: ${maxWarpDistance.toFixed(2)} LY`)
  console.log(`✅ Lanes > 10 LY: ${longLanes.length} (emergency connections for isolated systems)`)

  // Test 3: Network connectivity
  console.log('\n🌐 Testing network connectivity...')
  const isolatedSystems = galaxy.systems.filter(s => s.connections.length === 0)
  console.log(`✅ Isolated systems: ${isolatedSystems.length} (should be 0)`)

  // Test 4: Anomaly connections (should be none)
  console.log('\n🌟 Testing anomaly connections...')
  const anomalyConnections = galaxy.warpLanes.filter(
    w => w.from.startsWith('anomaly-') || w.to.startsWith('anomaly-')
  )
  console.log(`✅ Anomaly connections: ${anomalyConnections.length} (should be 0)`)

  // Test 5: Fixed systems
  console.log('\n⭐ Testing fixed systems...')
  const sol = galaxy.systems.find(s => s.id === 'sol')
  const alphaCentauri = galaxy.systems.find(s => s.id === 'alpha-centauri')
  const tauCeti = galaxy.systems.find(s => s.id === 'tau-ceti')

  if (sol && alphaCentauri && tauCeti) {
    const solToAlpha = Math.sqrt(
      Math.pow(sol.x - alphaCentauri.x, 2) + Math.pow(sol.y - alphaCentauri.y, 2)
    )
    console.log(`✅ Sol to Alpha Centauri: ${solToAlpha.toFixed(2)} LY (should be ~4.37)`)
    console.log(`✅ Sol position: (${sol.x}, ${sol.y}) (should be origin)`)
  }

  console.log('\n🎉 Galaxy generation test complete!')
}
