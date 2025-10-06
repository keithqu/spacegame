import * as d3 from 'd3'
import React, { useEffect, useRef, useState } from 'react'
import { GalaxyApiService } from '../services/galaxyApi'
import { Galaxy, StarSystem, GalaxyConfig } from '../types/galaxy'
import { SystemContextMenu } from './SystemContextMenu'

interface GalaxyMapProps {
  config?: Partial<GalaxyConfig>
  onSystemClick?: Function
  onAnomalyClick?: Function
  onGalaxyLoad?: Function
  onViewSystem?: Function
  selectedSystemId?: string | null
  className?: string
  forceNew?: boolean
  initialGalaxy?: Galaxy | null
}

export const GalaxyMap: React.FC<GalaxyMapProps> = ({
  config = {},
  onSystemClick,
  onAnomalyClick: _onAnomalyClick,
  onGalaxyLoad,
  onViewSystem,
  selectedSystemId,
  className = '',
  forceNew = false,
  initialGalaxy = null,
}) => {
  const svgRef = useRef<SVGSVGElement>(null)
  const zoomRef = useRef<d3.ZoomBehavior<SVGSVGElement, unknown> | null>(null)
  const zoomTransformRef = useRef<d3.ZoomTransform>(d3.zoomIdentity)
  const isUserInteractingRef = useRef<boolean>(false)
  const lastCenteredIdRef = useRef<string | null>(null)
  const [galaxy, setGalaxy] = useState<Galaxy | null>(initialGalaxy)
  const [selectedSystem, setSelectedSystem] = useState<string | null>(null)
  const [loading, setLoading] = useState(!initialGalaxy)
  const [error, setError] = useState<string | null>(null)
  const [dimensions, setDimensions] = useState({ width: 0, height: 0 })
  const [contextMenu, setContextMenu] = useState<{
    system: StarSystem
    position: { x: number; y: number }
  } | null>(null)

  // Load saved state or generate new depending on forceNew
  useEffect(() => {
    if (initialGalaxy) {
      setGalaxy(initialGalaxy)
      setLoading(false)
      setError(null)
      onGalaxyLoad?.(initialGalaxy)
      return
    }
    let cancelled = false
    const loadOrGenerate = async () => {
      try {
        setLoading(true)
        setError(null)

        // If forceNew, skip checking saved state
        if (!forceNew) {
          try {
            const saved = await GalaxyApiService.getSavedState()
            if (!cancelled && saved) {
              setGalaxy(saved)
              onGalaxyLoad?.(saved)
              return
            }
          } catch {
            // ignore saved-state fetch errors; fall through to generate
          }
        }

        // Otherwise generate
        const requestBody = forceNew ? { ...config, use_saved: false } : config
        const newGalaxy = await GalaxyApiService.generateGalaxy(requestBody)
        if (!cancelled) {
          setGalaxy(newGalaxy)
          onGalaxyLoad?.(newGalaxy)
        }
      } catch (err) {
        if (!cancelled) {
          setError(err instanceof Error ? err.message : 'Failed to generate galaxy')
        }
      } finally {
        if (!cancelled) setLoading(false)
      }
    }

    loadOrGenerate()
    return () => {
      cancelled = true
    }
  }, [config, onGalaxyLoad, forceNew, initialGalaxy])

  // Handle window resize
  useEffect(() => {
    const handleResize = () => {
      if (svgRef.current) {
        const container = svgRef.current.parentElement
        const width = container?.clientWidth || window.innerWidth
        const height = container?.clientHeight || window.innerHeight - 200
        setDimensions({ width, height })
      }
    }

    // Initial size
    handleResize()

    // Add resize listener
    window.addEventListener('resize', handleResize)
    return () => window.removeEventListener('resize', handleResize)
  }, [galaxy])

  // Render D3 visualization when galaxy or dimensions change (not on selection)
  useEffect(() => {
    if (!galaxy || !svgRef.current || !galaxy.config?.visualization || dimensions.width === 0)
      return

    const svg = d3.select(svgRef.current)
    const { width, height } = dimensions

    // Update SVG dimensions
    svg.attr('width', width).attr('height', height)

    // Clear previous content
    svg.selectAll('*').remove()

    // Set up scales with the configured scale factor
    const scale = galaxy.config.visualization?.scale || 3.0
    const scaledRadius = (galaxy.config.radius || 500) * scale

    const radius = galaxy.config.radius || 500

    const xScale = d3
      .scaleLinear()
      .domain([-radius, radius])
      .range([width / 2 - scaledRadius / 2, width / 2 + scaledRadius / 2])

    const yScale = d3
      .scaleLinear()
      .domain([-radius, radius])
      .range([height / 2 + scaledRadius / 2, height / 2 - scaledRadius / 2])

    // Create main group for zoom/pan
    const mainGroup = svg.append('g').attr('class', 'main-group')
    // Apply any previously saved transform BEFORE drawing to avoid center flash
    mainGroup.attr('transform', zoomTransformRef.current.toString())

    // Add zoom behavior with no boundary constraints - free exploration
    const zoom = d3
      .zoom<SVGSVGElement, unknown>()
      .scaleExtent([0.1, 10])
      .on('start', () => {
        isUserInteractingRef.current = true
      })
      .on('zoom', event => {
        zoomTransformRef.current = event.transform
        mainGroup.attr('transform', event.transform)
      })
      .on('end', () => {
        isUserInteractingRef.current = false
      })

    // Store zoom behavior reference for later use
    zoomRef.current = zoom
    svg.call(zoom)

    // Reapply previous zoom/pan to preserve view between renders
    // Casting to any to satisfy d3 typing for call with transform
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    ;(svg as any).call(zoom.transform, zoomTransformRef.current)

    // Ensure there is a full-size invisible background to capture drag events
    mainGroup
      .append('rect')
      .attr('class', 'drag-capture')
      .attr('x', 0)
      .attr('y', 0)
      .attr('width', width)
      .attr('height', height)
      .attr('fill', 'transparent')

    // Draw galaxy boundary circle
    mainGroup
      .append('circle')
      .attr('cx', xScale(0))
      .attr('cy', yScale(0))
      .attr('r', scaledRadius / 2)
      .attr('fill', 'none')
      .attr('stroke', '#444')
      .attr('stroke-width', 2)
      .attr('stroke-dasharray', '5,5')
      .attr('opacity', 0.7)

    // Draw warp lanes first (so they appear behind systems)
    const warpLaneGroup = mainGroup.append('g').attr('class', 'warp-lanes')

    // Create warp lane lines with enhanced visibility
    const warpLanes = warpLaneGroup
      .selectAll('g.warp-lane')
      .data(galaxy.warpLanes)
      .enter()
      .append('g')
      .attr('class', 'warp-lane')

    // Add glow effect for discovered lanes
    warpLanes
      .filter(d => d.discovered)
      .append('line')
      .attr('x1', d => {
        const fromSystem = galaxy.systems.find(s => s.id === d.from)
        return fromSystem ? xScale(fromSystem.x) : 0
      })
      .attr('y1', d => {
        const fromSystem = galaxy.systems.find(s => s.id === d.from)
        return fromSystem ? yScale(fromSystem.y) : 0
      })
      .attr('x2', d => {
        const toSystem = galaxy.systems.find(s => s.id === d.to)
        return toSystem ? xScale(toSystem.x) : 0
      })
      .attr('y2', d => {
        const toSystem = galaxy.systems.find(s => s.id === d.to)
        return toSystem ? yScale(toSystem.y) : 0
      })
      .attr('stroke', '#ffdd00')
      .attr('stroke-width', 6)
      .attr('opacity', 0.3)
      .attr('filter', 'blur(2px)')

    // Main warp lane lines
    warpLanes
      .append('line')
      .attr('x1', d => {
        const fromSystem = galaxy.systems.find(s => s.id === d.from)
        return fromSystem ? xScale(fromSystem.x) : 0
      })
      .attr('y1', d => {
        const fromSystem = galaxy.systems.find(s => s.id === d.from)
        return fromSystem ? yScale(fromSystem.y) : 0
      })
      .attr('x2', d => {
        const toSystem = galaxy.systems.find(s => s.id === d.to)
        return toSystem ? xScale(toSystem.x) : 0
      })
      .attr('y2', d => {
        const toSystem = galaxy.systems.find(s => s.id === d.to)
        return toSystem ? yScale(toSystem.y) : 0
      })
      .attr('stroke', d => {
        if (d.discovered) return '#ffdd00'
        return d.distance <= 6 ? '#888' : '#555'
      })
      .attr('stroke-width', d => {
        if (d.discovered) return 2.5
        return d.distance <= 6 ? 2 : 1.5
      })
      .attr('opacity', d => (d.discovered ? 0.9 : 0.6))
      .attr('stroke-dasharray', d => (d.discovered ? 'none' : '3,3'))
      .style('cursor', 'pointer')
      .on('mouseover', function (event, d) {
        d3.select(this)
          .attr('stroke-width', d.discovered ? 4 : 3)
          .attr('opacity', 1)
      })
      .on('mouseout', function (event, d) {
        d3.select(this)
          .attr('stroke-width', d.discovered ? 2.5 : d.distance <= 6 ? 2 : 1.5)
          .attr('opacity', d.discovered ? 0.9 : 0.6)
      })

    // Draw anomalies
    const anomalyGroup = mainGroup.append('g').attr('class', 'anomalies')

    const anomalyColors = {
      nebula: '#ff6b9d',
      blackhole: '#1a1a1a',
      wormhole: '#9b59b6',
      artifact: '#f39c12',
      resource: '#27ae60',
    }

    anomalyGroup
      .selectAll('circle')
      .data(galaxy.anomalies)
      .enter()
      .append('circle')
      .attr('cx', d => xScale(d.x))
      .attr('cy', d => yScale(d.y))
      .attr('r', d => (d.discovered ? 8 : 6))
      .attr('fill', d => anomalyColors[d.type])
      .attr('stroke', d => (d.discovered ? '#fff' : 'none'))
      .attr('stroke-width', 1)
      .attr('opacity', d => (d.discovered ? 0.9 : 0.6))
      .style('cursor', 'pointer')
      .on('click', (event, d) => {
        event.stopPropagation()
        _onAnomalyClick?.(d)
      })
      .on('mouseover', function (event, d) {
        // Show tooltip
        d3.select(this).attr('r', d.discovered ? 10 : 8)

        // Create tooltip
        const tooltip = d3
          .select('body')
          .append('div')
          .attr('class', 'galaxy-tooltip')
          .style('position', 'absolute')
          .style('background', 'rgba(0, 0, 0, 0.8)')
          .style('color', 'white')
          .style('padding', '8px')
          .style('border-radius', '4px')
          .style('font-size', '12px')
          .style('pointer-events', 'none')
          .style('z-index', '1000').html(`
            <strong>${d.name}</strong><br/>
            Type: ${d.type}<br/>
            ${d.discovered ? 'Discovered' : 'Unknown'}
          `)

        tooltip.style('left', `${event.pageX + 10}px`).style('top', `${event.pageY - 10}px`)
      })
      .on('mouseout', function (event, d) {
        d3.select(this).attr('r', d.discovered ? 8 : 6)
        d3.selectAll('.galaxy-tooltip').remove()
      })

    // Draw star systems
    const systemGroup = mainGroup.append('g').attr('class', 'star-systems')

    const systemColors = {
      origin: '#ffd700',
      core: '#00bfff', // Same color for all non-origin systems
      rim: '#00bfff', // Same color for all non-origin systems
    }

    const systemSizes = {
      origin: 12,
      core: 6, // Same size for all non-origin systems
      rim: 6, // Same size for all non-origin systems
    }

    systemGroup
      .selectAll('circle')
      .data(galaxy.systems)
      .enter()
      .append('circle')
      .attr('cx', d => xScale(d.x))
      .attr('cy', d => yScale(d.y))
      .attr('r', d => systemSizes[d.type])
      .attr('fill', d => systemColors[d.type])
      .attr('stroke', d => (d.explored ? '#fff' : 'none'))
      .attr('stroke-width', 1)
      .attr('opacity', d => (d.explored ? 1.0 : 0.7))
      .style('cursor', 'pointer')
      .on('click', (event, d) => {
        event.stopPropagation()
        setSelectedSystem(d.id)
        onSystemClick?.(d)
        setContextMenu(null) // Close context menu on left click
      })
      .on('contextmenu', (event, d) => {
        event.preventDefault()
        event.stopPropagation()
        setContextMenu({
          system: d,
          position: { x: event.clientX, y: event.clientY },
        })
      })
      .on('mouseover', function (event, d) {
        // Highlight system
        d3.select(this)
          .attr('r', systemSizes[d.type] + 2)
          .attr('stroke', '#fff')
          .attr('stroke-width', 2)

        // Create tooltip
        const tooltip = d3
          .select('body')
          .append('div')
          .attr('class', 'galaxy-tooltip')
          .style('position', 'absolute')
          .style('background', 'rgba(0, 0, 0, 0.8)')
          .style('color', 'white')
          .style('padding', '8px')
          .style('border-radius', '4px')
          .style('font-size', '12px')
          .style('pointer-events', 'none')
          .style('z-index', '1000').html(`
            <strong>${d.name}</strong><br/>
            Type: ${d.type}<br/>
            Position: (${d.x.toFixed(1)}, ${d.y.toFixed(1)}) LY<br/>
            Connections: ${d.connections.length}<br/>
            ${d.explored ? 'Explored' : 'Unexplored'}<br/>
            ${d.population ? `Population: ${d.population.toLocaleString()}` : ''}
          `)

        tooltip.style('left', `${event.pageX + 10}px`).style('top', `${event.pageY - 10}px`)
      })
      .on('mouseout', function (event, d) {
        d3.select(this)
          .attr('r', systemSizes[d.type])
          .attr('stroke', d.explored ? '#fff' : 'none')
          .attr('stroke-width', 1)

        d3.selectAll('.galaxy-tooltip').remove()
      })

    // Add system labels for ALL systems
    systemGroup
      .selectAll('text')
      .data(galaxy.systems)
      .enter()
      .append('text')
      .attr('x', d => xScale(d.x))
      .attr('y', d => yScale(d.y) + systemSizes[d.type] + 15)
      .attr('text-anchor', 'middle')
      .attr('fill', d => {
        // Only distinguish origin system visually
        switch (d.type) {
          case 'origin':
            return '#ffdd00' // Gold for origin
          default:
            return '#fff' // White for all other systems
        }
      })
      .attr('font-size', d => {
        // Only distinguish origin system visually
        switch (d.type) {
          case 'origin':
            return '14px'
          default:
            return '10px' // Same size for all non-origin systems
        }
      })
      .attr('font-weight', d => (d.type === 'origin' ? 'bold' : 'normal'))
      .text(d => d.name)
      .style('pointer-events', 'none')
      .attr('opacity', 1.0) // Same opacity for all systems

    // Add legend
    const legend = svg
      .append('g')
      .attr('class', 'legend')
      .attr('transform', `translate(${width - 200}, 20)`)

    const legendData = [
      { type: 'Origin System', color: systemColors.origin, size: systemSizes.origin },
      { type: 'Star System', color: systemColors.core, size: systemSizes.core },
    ]

    legend
      .selectAll('g')
      .data(legendData)
      .enter()
      .append('g')
      .attr('transform', (d, i) => `translate(0, ${i * 25})`)
      .each(function (d) {
        const g = d3.select(this)
        g.append('circle').attr('cx', 10).attr('cy', 0).attr('r', d.size).attr('fill', d.color)
        g.append('text')
          .attr('x', 25)
          .attr('y', 5)
          .attr('fill', '#fff')
          .attr('font-size', '12px')
          .text(d.type)
      })

    // Add galaxy info
    const info = svg.append('g').attr('class', 'galaxy-info').attr('transform', 'translate(20, 20)')

    info
      .append('text')
      .attr('x', 0)
      .attr('y', 0)
      .attr('fill', '#fff')
      .attr('font-size', '14px')
      .attr('font-weight', 'bold')
      .text(`Galaxy Seed: ${galaxy.config.seed}`)

    info
      .append('text')
      .attr('x', 0)
      .attr('y', 20)
      .attr('fill', '#ccc')
      .attr('font-size', '12px')
      .text(`${galaxy.systems.length} systems, ${galaxy.anomalies.length} anomalies`)

    info
      .append('text')
      .attr('x', 0)
      .attr('y', 40)
      .attr('fill', '#ccc')
      .attr('font-size', '12px')
      .text(`${galaxy.warpLanes.length} warp lanes, ${galaxy.config.radius} LY radius`)
  }, [galaxy, dimensions])

  // Update selection highlight without rebuilding the scene
  useEffect(() => {
    if (!svgRef.current) return
    const svg = d3.select(svgRef.current)
    const circles = svg.selectAll<d3.BaseType, StarSystem>('.star-systems circle')
    circles
      .attr('stroke', (d: StarSystem) => {
        if (selectedSystem === d.id) return '#fff'
        return d.explored ? '#fff' : 'none'
      })
      .attr('stroke-width', (d: StarSystem) => (selectedSystem === d.id ? 3 : 1))
  }, [selectedSystem])

  // Handle centering on selected system from ledger (only when transform actually needs to change)
  useEffect(() => {
    if (!galaxy || !selectedSystemId || !svgRef.current || !zoomRef.current) return

    const system = galaxy.systems.find(s => s.id === selectedSystemId)
    if (!system) return

    const svg = d3.select(svgRef.current)
    const { width, height } = dimensions
    const scaledRadius = galaxy.config.visualization.scale * galaxy.config.radius

    // Create scales for coordinate conversion (same as main rendering)
    const xScale = d3
      .scaleLinear()
      .domain([-galaxy.config.radius, galaxy.config.radius])
      .range([width / 2 - scaledRadius / 2, width / 2 + scaledRadius / 2])

    const yScale = d3
      .scaleLinear()
      .domain([-galaxy.config.radius, galaxy.config.radius])
      .range([height / 2 + scaledRadius / 2, height / 2 - scaledRadius / 2])

    // Calculate the system's position in the unzoomed coordinate system
    const systemX = xScale(system.x)
    const systemY = yScale(system.y)

    // Calculate the center of the viewport
    const centerX = width / 2
    const centerY = height / 2

    // Set zoom scale and calculate transform (at least 2x, or keep current if higher)
    const zoomScale = Math.max(2, zoomTransformRef.current.k)

    // Debug info (can be removed in production)
    // console.log(`Centering on ${system.name} at (${system.x}, ${system.y})`)

    // Apply zoom with smooth transition using the stored zoom behavior
    // Use D3's transform.translate and scale methods for proper composition
    const targetTransform = d3.zoomIdentity
      .translate(centerX, centerY)
      .scale(zoomScale)
      .translate(-systemX, -systemY)

    // If we're already effectively at the target transform, skip
    const current = zoomTransformRef.current
    const approxEqual = (a: number, b: number) => Math.abs(a - b) < 0.5
    if (
      approxEqual(current.k, targetTransform.k) &&
      approxEqual(current.x, targetTransform.x) &&
      approxEqual(current.y, targetTransform.y)
    ) {
      lastCenteredIdRef.current = selectedSystemId
      return
    }

    svg
      .transition()
      .duration(750)
      .call(zoomRef.current.transform, targetTransform)
      .on('end', () => {
        zoomTransformRef.current = targetTransform
        lastCenteredIdRef.current = selectedSystemId
      })
  }, [selectedSystemId, galaxy, dimensions])

  if (loading) {
    return (
      <div className="loading-container">
        <div>🌌 Generating galaxy...</div>
      </div>
    )
  }

  if (error) {
    return (
      <div className="error-container">
        <div className="text-lg font-bold mb-2">Galaxy Generation Failed</div>
        <div className="text-sm">{error}</div>
      </div>
    )
  }

  if (!galaxy) {
    return (
      <div className="loading-container">
        <div>No galaxy data available</div>
      </div>
    )
  }

  return (
    <div className={`galaxy-map ${className}`} style={{ width: '100%', height: '100%' }}>
      <svg
        ref={svgRef}
        style={{
          background: 'radial-gradient(ellipse at center, #1a1a2e 0%, #0a0a0a 100%)',
          width: '100%',
          height: '100%',
        }}
      />

      {/* System Context Menu */}
      {contextMenu && (
        <SystemContextMenu
          system={contextMenu.system}
          position={contextMenu.position}
          onClose={() => setContextMenu(null)}
          onViewSystem={system => {
            onViewSystem?.(system)
            setContextMenu(null)
          }}
        />
      )}
    </div>
  )
}
