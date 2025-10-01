import * as d3 from 'd3'
import React, { useEffect, useRef, useState } from 'react'
import { GalaxyApiService } from '../services/galaxyApi'
import { StarSystem, DetailedSystemData } from '../types/galaxy'

interface Planet {
  id: string
  name: string
  type: 'terrestrial' | 'gas-giant' | 'ice-giant' | 'asteroid-belt'
  distance: number // AU from star
  radius: number // Planet radius for rendering
  color: string
  moons: Moon[]
}

interface Moon {
  id: string
  name: string
  distance: number // Distance from planet (scaled)
  radius: number
  color: string
  // Additional properties for moon selection
  diameter?: number // km
  mass?: number // Earth masses
  gravity?: number // Percentage of Earth gravity
}

interface SystemOrbitMapProps {
  system: StarSystem
  onPlanetClick?: (planetId: string) => void
  selectedPlanetId?: string | null
}

export const SystemOrbitMap: React.FC<SystemOrbitMapProps> = ({
  system,
  onPlanetClick,
  selectedPlanetId,
}) => {
  const svgRef = useRef<SVGSVGElement>(null)
  const onPlanetClickRef = useRef(onPlanetClick)
  const [dimensions, setDimensions] = useState({ width: 0, height: 0 })
  const [planets, setPlanets] = useState<Planet[]>([])
  const [detailedSystemData, setDetailedSystemData] = useState<DetailedSystemData | null>(null)

  // Keep the callback ref up to date
  useEffect(() => {
    onPlanetClickRef.current = onPlanetClick
  }, [onPlanetClick])

  // Fetch detailed system data if available
  useEffect(() => {
    const fetchDetailedData = async () => {
      // eslint-disable-next-line no-console
      console.log(
        'Fetching system data for:',
        system.id,
        'hasDetailedData:',
        system.hasDetailedData
      )

      if (system.hasDetailedData) {
        try {
          const detailedData = await GalaxyApiService.getSystemDetails(system.id)
          // eslint-disable-next-line no-console
          console.log('Received detailed data:', detailedData)
          // eslint-disable-next-line no-console
          console.log('Planets in detailed data:', detailedData?.planets?.length || 0)
          setDetailedSystemData(detailedData)
        } catch (error) {
          // eslint-disable-next-line no-console
          console.error('Failed to fetch detailed system data:', error)
          setDetailedSystemData(null)
        }
      } else {
        // eslint-disable-next-line no-console
        console.log('System has no detailed data, using fallback')
        setDetailedSystemData(null)
      }
    }

    fetchDetailedData()
  }, [system.id, system.hasDetailedData])

  // Generate planets based on system info or detailed data
  useEffect(() => {
    console.log('Planet generation effect triggered:', {
      hasSystemInfo: !!system.systemInfo,
      hasDetailedData: !!detailedSystemData,
      systemInfo: system.systemInfo,
      detailedDataPlanets: detailedSystemData?.planets?.length || 0,
    })

    // Use detailed system data if available
    if (detailedSystemData) {
      // eslint-disable-next-line no-console
      console.log('Processing detailed system data:', {
        systemId: system.id,
        hasPlanets: detailedSystemData.planets?.length > 0,
        planetCount: detailedSystemData.planets?.length || 0,
        firstPlanet: detailedSystemData.planets?.[0],
      })
      // Find the largest planet diameter for scaling reference
      const maxPlanetDiameter = Math.max(...detailedSystemData.planets.map(p => p.diameter))
      const minPlanetDiameter = Math.min(...detailedSystemData.planets.map(p => p.diameter))

      // eslint-disable-next-line no-console
      console.log('System data:', {
        systemId: system.id,
        planetCount: detailedSystemData.planets.length,
        maxDiameter: maxPlanetDiameter,
        minDiameter: minPlanetDiameter,
        planets: detailedSystemData.planets.map(p => ({ name: p.name, diameter: p.diameter })),
      })

      const detailedPlanets: Planet[] = detailedSystemData.planets.map((planet, index) => {
        // Scale planet size from 3-10 based on diameter relative to other planets in system
        const diameterRange = maxPlanetDiameter - minPlanetDiameter
        const diameterRatio =
          diameterRange > 0 ? (planet.diameter - minPlanetDiameter) / diameterRange : 0.5 // If all planets have same diameter, use middle value
        const scaledRadius = 3 + diameterRatio * 7 // Scale from 3 to 10 (tripled minimum size)

        return {
          id: planet.id,
          name: planet.name,
          type: getPlanetTypeFromComposition(planet.composition),
          distance: index + 1, // Uniform spacing for display
          radius: Math.max(3, Math.min(10, scaledRadius)), // Ensure 3-10 range
          color: getPlanetColor(planet.composition, planet.habitability),
          moons: planet.moons.map((moon, moonIndex) => {
            // Scale moon size from 0.3-1.5 based on diameter relative to largest moon (smaller than before)
            const maxMoonDiameter = Math.max(...planet.moons.map(m => m.diameter), 1)
            const moonDiameterRatio = moon.diameter / maxMoonDiameter
            const scaledMoonRadius = 0.3 + moonDiameterRatio * 1.2 // Scale from 0.3 to 1.5 (smaller)

            return {
              id: moon.id,
              name: moon.name,
              distance: 20 + moonIndex * 15, // Increased distance for visibility
              radius: Math.max(0.3, Math.min(1.5, scaledMoonRadius)),
              color: '#cccccc',
              // Include moon properties for selection
              diameter: moon.diameter,
              mass: moon.mass,
              gravity: moon.gravity,
            }
          }),
        }
      })

      // Remove large asteroids for now - they will be handled separately later
      const asteroidPlanets: Planet[] = []

      const allPlanets = [...detailedPlanets, ...asteroidPlanets]
      // eslint-disable-next-line no-console
      console.log(
        'Final planets array:',
        allPlanets.map(p => ({
          name: p.name,
          radius: p.radius,
          distance: p.distance,
          type: p.type,
        }))
      )

      // eslint-disable-next-line no-console
      console.log('Setting planets:', allPlanets.length, 'planets')
      setPlanets(allPlanets)
      return
    }

    // Fallback to system info if no detailed data
    if (!system.systemInfo) return

    // Safety check for corrupted data
    const planetCount = system.systemInfo?.planetCount || 0
    const moonCount = system.systemInfo?.moonCount || 0
    const asteroidCount = system.systemInfo?.asteroidCount || 0

    // Prevent frontend stalling with corrupted data
    if (planetCount > 50 || moonCount > 1000 || asteroidCount > 100000) {
      // eslint-disable-next-line no-console
      console.error('Corrupted system data detected, using fallback values')
      setPlanets([])
      return
    }

    const generatePlanets = (): Planet[] => {
      const planetTypes = ['terrestrial', 'gas-giant', 'ice-giant'] as const
      const planetColors = {
        terrestrial: ['#8B4513', '#CD853F', '#D2691E', '#A0522D'],
        'gas-giant': ['#FF4500', '#FF6347', '#FFA500', '#FFD700'],
        'ice-giant': ['#4682B4', '#87CEEB', '#B0E0E6', '#E0FFFF'],
      }

      const generatedPlanets: Planet[] = []

      for (let i = 0; i < (system.systemInfo?.planetCount || 0); i++) {
        const distance = 0.5 + i * 1.2 + Math.random() * 0.8 // AU from star
        const type = planetTypes[Math.floor(Math.random() * planetTypes.length)]
        const colors = planetColors[type]

        // Generate moons
        const moonCount = Math.min(
          Math.floor(Math.random() * 4),
          system.systemInfo &&
            system.systemInfo.moonCount !== undefined &&
            system.systemInfo.planetCount
            ? Math.floor(system.systemInfo.moonCount / system.systemInfo.planetCount)
            : 0
        )
        const moons: Moon[] = []

        for (let j = 0; j < moonCount; j++) {
          moons.push({
            id: `${system.id}-planet-${i}-moon-${j}`,
            name: `Moon ${j + 1}`,
            distance: 20 + j * 15, // Distance from planet in pixels
            radius: 0.3 + Math.random() * 1.2, // Smaller moons (0.3-1.5)
            color: '#999',
          })
        }

        generatedPlanets.push({
          id: `${system.id}-planet-${i}`,
          name: `Planet ${String.fromCharCode(65 + i)}`, // A, B, C, etc.
          type,
          distance,
          radius: type === 'gas-giant' ? 6 + Math.random() * 4 : 3 + Math.random() * 5, // Scale 3-10 for variety
          color: colors[Math.floor(Math.random() * colors.length)],
          moons,
        })
      }

      // Add asteroid belt if there are enough asteroids
      if (system.systemInfo && system.systemInfo.asteroidCount > 1000) {
        generatedPlanets.push({
          id: `${system.id}-asteroid-belt`,
          name: 'Asteroid Belt',
          type: 'asteroid-belt',
          distance: 2.5 + Math.random() * 2,
          radius: 0, // Special handling for asteroid belt
          color: '#666',
          moons: [],
        })
      }

      return generatedPlanets.sort((a, b) => a.distance - b.distance)
    }

    setPlanets(generatePlanets())
  }, [system, detailedSystemData])

  // Helper functions for detailed system data
  const getPlanetTypeFromComposition = (composition: string): Planet['type'] => {
    if (
      composition.toLowerCase().includes('gas giant') ||
      composition.toLowerCase().includes('hydrogen')
    ) {
      return 'gas-giant'
    }
    if (
      composition.toLowerCase().includes('ice') ||
      composition.toLowerCase().includes('methane')
    ) {
      return 'ice-giant'
    }
    return 'terrestrial'
  }

  const getPlanetColor = (composition: string, habitability: number): string => {
    if (habitability > 50) return '#4A90E2' // Habitable blue-green
    if (composition.toLowerCase().includes('gas giant')) return '#FFA500' // Orange for gas giants
    if (composition.toLowerCase().includes('ice')) return '#87CEEB' // Light blue for ice
    if (composition.toLowerCase().includes('iron')) return '#8B4513' // Brown for iron-rich
    return '#CD853F' // Default terrestrial color
  }

  // Handle container resize
  useEffect(() => {
    const handleResize = () => {
      if (svgRef.current) {
        const rect = svgRef.current.parentElement?.getBoundingClientRect()
        if (rect) {
          setDimensions({ width: rect.width, height: rect.height })
        }
      }
    }

    handleResize()
    window.addEventListener('resize', handleResize)
    return () => window.removeEventListener('resize', handleResize)
  }, [])

  // Render orbital map
  useEffect(() => {
    if (!svgRef.current || dimensions.width === 0 || dimensions.height === 0) return

    const svg = d3.select(svgRef.current)
    svg.selectAll('*').remove()

    const { width, height } = dimensions
    const centerX = width / 2
    const centerY = height / 2
    const maxRadius = Math.min(width, height) / 2 - 50
    const auScale =
      maxRadius / (planets.length > 0 ? Math.max(...planets.map(p => p.distance)) + 1 : 5)

    // Add zoom behavior
    const zoom = d3
      .zoom<SVGSVGElement, unknown>()
      .scaleExtent([0.5, 5])
      .on('zoom', event => {
        mainGroup.attr('transform', event.transform)
      })

    svg.call(zoom)

    const mainGroup = svg.append('g').attr('class', 'orbit-main-group')

    // Draw star at center
    const starGroup = mainGroup.append('g').attr('class', 'star')

    // Star glow effect
    const starGlow = svg
      .append('defs')
      .append('radialGradient')
      .attr('id', 'star-glow')
      .attr('cx', '50%')
      .attr('cy', '50%')
      .attr('r', '50%')

    starGlow
      .append('stop')
      .attr('offset', '0%')
      .attr('stop-color', '#FFD700')
      .attr('stop-opacity', '1')

    starGlow
      .append('stop')
      .attr('offset', '100%')
      .attr('stop-color', '#FFA500')
      .attr('stop-opacity', '0')

    starGroup
      .append('circle')
      .attr('cx', centerX)
      .attr('cy', centerY)
      .attr('r', 20)
      .attr('fill', 'url(#star-glow)')
      .attr('stroke', '#FFD700')
      .attr('stroke-width', 2)

    // Add star type label
    const starType = detailedSystemData?.starType || system.systemInfo?.starType || 'Unknown Star'
    starGroup
      .append('text')
      .attr('x', centerX)
      .attr('y', centerY - 35)
      .attr('text-anchor', 'middle')
      .attr('fill', '#FFD700')
      .attr('font-size', '14px')
      .attr('font-weight', 'bold')
      .text(starType)

    // Draw orbits and planets
    console.log('Rendering planets:', planets.length, 'planets')
    console.log('Planets data:', planets)
    planets.forEach((planet, index) => {
      const orbitRadius = planet.distance * auScale

      if (planet.type === 'asteroid-belt') {
        // Draw asteroid belt as a dotted circle
        const asteroidGroup = mainGroup.append('g').attr('class', 'asteroid-belt')

        for (let i = 0; i < 50; i++) {
          const angle = (i / 50) * 2 * Math.PI
          const radius = orbitRadius + (Math.random() - 0.5) * 20
          const x = centerX + radius * Math.cos(angle)
          const y = centerY + radius * Math.sin(angle)

          asteroidGroup
            .append('circle')
            .attr('cx', x)
            .attr('cy', y)
            .attr('r', 0.5 + Math.random() * 1.5)
            .attr('fill', planet.color)
            .attr('opacity', 0.6)
        }
      } else {
        // Draw orbit line
        mainGroup
          .append('circle')
          .attr('cx', centerX)
          .attr('cy', centerY)
          .attr('r', orbitRadius)
          .attr('fill', 'none')
          .attr('stroke', '#444')
          .attr('stroke-width', 1)
          .attr('stroke-dasharray', '2,2')
          .attr('opacity', 0.5)

        // Calculate planet position (static for now to avoid performance issues)
        const angle = (index / planets.length) * 2 * Math.PI
        const planetX = centerX + orbitRadius * Math.cos(angle)
        const planetY = centerY + orbitRadius * Math.sin(angle)

        const planetGroup = mainGroup.append('g').attr('class', 'planet')

        // Draw planet
        planetGroup
          .append('circle')
          .attr('cx', planetX)
          .attr('cy', planetY)
          .attr('r', planet.radius)
          .attr('fill', planet.color)
          .attr('stroke', selectedPlanetId === planet.id ? '#fff' : 'none')
          .attr('stroke-width', selectedPlanetId === planet.id ? 2 : 0)
          .style('cursor', 'pointer')
          .on('click', () => {
            onPlanetClickRef.current?.(planet.id)
          })

        // Draw moons
        planet.moons.forEach((moon, moonIndex) => {
          const moonAngle = (moonIndex / planet.moons.length) * 2 * Math.PI
          const moonX = planetX + moon.distance * Math.cos(moonAngle)
          const moonY = planetY + moon.distance * Math.sin(moonAngle)

          // Moon orbit
          planetGroup
            .append('circle')
            .attr('cx', planetX)
            .attr('cy', planetY)
            .attr('r', moon.distance)
            .attr('fill', 'none')
            .attr('stroke', '#333')
            .attr('stroke-width', 0.5)
            .attr('opacity', 0.3)

          // Moon
          planetGroup
            .append('circle')
            .attr('cx', moonX)
            .attr('cy', moonY)
            .attr('r', moon.radius)
            .attr('fill', moon.color)
            .attr('stroke', selectedPlanetId === moon.id ? '#fff' : 'none')
            .attr('stroke-width', selectedPlanetId === moon.id ? 1 : 0)
            .attr('opacity', 0.8)
            .style('cursor', 'pointer')
            .on('click', () => {
              onPlanetClickRef.current?.(moon.id)
            })
        })

        // Planet label
        planetGroup
          .append('text')
          .attr('x', planetX)
          .attr('y', planetY + planet.radius + 15)
          .attr('text-anchor', 'middle')
          .attr('fill', '#ccc')
          .attr('font-size', '10px')
          .text(planet.name)
      }
    })

    // Add system name
    svg
      .append('text')
      .attr('x', 20)
      .attr('y', 30)
      .attr('fill', '#60a5fa')
      .attr('font-size', '20px')
      .attr('font-weight', 'bold')
      .text(system.name)

    // Add system stats
    const statsGroup = svg.append('g').attr('class', 'system-stats')

    const stats = [
      `Population: ${system.population ? (system.population / 1000000).toFixed(1) : '0'}M`,
      `GDP: ${system.gdp ? (system.gdp / 1000000).toFixed(1) : '0'}M credits`,
      `Planets: ${system.systemInfo?.planetCount || 0}`,
      `Moons: ${system.systemInfo?.moonCount || 0}`,
    ]

    stats.forEach((stat, i) => {
      statsGroup
        .append('text')
        .attr('x', 20)
        .attr('y', 60 + i * 20)
        .attr('fill', '#ccc')
        .attr('font-size', '12px')
        .text(stat)
    })
  }, [planets, dimensions, selectedPlanetId, system, detailedSystemData?.starType])

  return (
    <div className="orbital-map" style={{ width: '100%', height: '100%' }}>
      <svg
        ref={svgRef}
        style={{
          background: 'radial-gradient(ellipse at center, #0a0a2e 0%, #000000 100%)',
          width: '100%',
          height: '100%',
        }}
      />
    </div>
  )
}
