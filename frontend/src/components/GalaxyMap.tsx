import React, { useEffect, useRef, useState } from 'react';
import * as d3 from 'd3';
import { Galaxy, StarSystem, Anomaly, GalaxyConfig } from '../types/galaxy';
import { GalaxyApiService } from '../services/galaxyApi';

interface GalaxyMapProps {
  config?: Partial<GalaxyConfig>;
  onSystemClick?: (system: StarSystem) => void;
  onAnomalyClick?: (anomaly: Anomaly) => void;
  className?: string;
}

export const GalaxyMap: React.FC<GalaxyMapProps> = ({
  config = {},
  onSystemClick,
  onAnomalyClick,
  className = ''
}) => {
  const svgRef = useRef<SVGSVGElement>(null);
  const [galaxy, setGalaxy] = useState<Galaxy | null>(null);
  const [selectedSystem, setSelectedSystem] = useState<string | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  // Generate galaxy when config changes
  useEffect(() => {
    const generateGalaxy = async () => {
      try {
        setLoading(true);
        setError(null);
        
        // Use API service to generate galaxy
        const newGalaxy = await GalaxyApiService.generateGalaxy(config);
        
        setGalaxy(newGalaxy);
      } catch (err) {
        setError(err instanceof Error ? err.message : 'Failed to generate galaxy');
        console.error('Galaxy generation error:', err);
      } finally {
        setLoading(false);
      }
    };

    generateGalaxy();
  }, [config]);

  // Render D3 visualization when galaxy changes
  useEffect(() => {
    if (!galaxy || !svgRef.current || !galaxy.config?.visualization) return;

    const svg = d3.select(svgRef.current);
    const width = galaxy.config.visualization.width || 1200;
    const height = galaxy.config.visualization.height || 800;

    // Clear previous content
    svg.selectAll('*').remove();

    // Set up scales with the configured scale factor
    const scale = galaxy.config.visualization?.scale || 3.0;
    const scaledRadius = (galaxy.config.radius || 500) * scale;
    
    const radius = galaxy.config.radius || 500;
    
    const xScale = d3.scaleLinear()
      .domain([-radius, radius])
      .range([width/2 - scaledRadius/2, width/2 + scaledRadius/2]);

    const yScale = d3.scaleLinear()
      .domain([-radius, radius])
      .range([height/2 + scaledRadius/2, height/2 - scaledRadius/2]);

    // Create main group for zoom/pan
    const mainGroup = svg.append('g').attr('class', 'main-group');

    // Add zoom behavior
    const zoom = d3.zoom<SVGSVGElement, unknown>()
      .scaleExtent([0.1, 10])
      .on('zoom', (event) => {
        mainGroup.attr('transform', event.transform);
      });

    svg.call(zoom);

    // Draw galaxy boundary circle
    mainGroup.append('circle')
      .attr('cx', xScale(0))
      .attr('cy', yScale(0))
      .attr('r', scaledRadius / 2)
      .attr('fill', 'none')
      .attr('stroke', '#444')
      .attr('stroke-width', 2)
      .attr('stroke-dasharray', '5,5')
      .attr('opacity', 0.7);

    // Draw warp lanes first (so they appear behind systems)
    const warpLaneGroup = mainGroup.append('g').attr('class', 'warp-lanes');
    
    // Create warp lane lines with enhanced visibility
    const warpLanes = warpLaneGroup.selectAll('g.warp-lane')
      .data(galaxy.warpLanes)
      .enter()
      .append('g')
      .attr('class', 'warp-lane');
    
    // Add glow effect for discovered lanes
    warpLanes.filter(d => d.discovered)
      .append('line')
      .attr('x1', d => {
        const fromSystem = galaxy.systems.find(s => s.id === d.from);
        return fromSystem ? xScale(fromSystem.x) : 0;
      })
      .attr('y1', d => {
        const fromSystem = galaxy.systems.find(s => s.id === d.from);
        return fromSystem ? yScale(fromSystem.y) : 0;
      })
      .attr('x2', d => {
        const toSystem = galaxy.systems.find(s => s.id === d.to);
        return toSystem ? xScale(toSystem.x) : 0;
      })
      .attr('y2', d => {
        const toSystem = galaxy.systems.find(s => s.id === d.to);
        return toSystem ? yScale(toSystem.y) : 0;
      })
      .attr('stroke', '#ffdd00')
      .attr('stroke-width', 6)
      .attr('opacity', 0.3)
      .attr('filter', 'blur(2px)');
    
    // Main warp lane lines
    warpLanes.append('line')
      .attr('x1', d => {
        const fromSystem = galaxy.systems.find(s => s.id === d.from);
        return fromSystem ? xScale(fromSystem.x) : 0;
      })
      .attr('y1', d => {
        const fromSystem = galaxy.systems.find(s => s.id === d.from);
        return fromSystem ? yScale(fromSystem.y) : 0;
      })
      .attr('x2', d => {
        const toSystem = galaxy.systems.find(s => s.id === d.to);
        return toSystem ? xScale(toSystem.x) : 0;
      })
      .attr('y2', d => {
        const toSystem = galaxy.systems.find(s => s.id === d.to);
        return toSystem ? yScale(toSystem.y) : 0;
      })
      .attr('stroke', d => {
        if (d.discovered) return '#ffdd00';
        return d.distance <= 6 ? '#888' : '#555';
      })
      .attr('stroke-width', d => {
        if (d.discovered) return 2.5;
        return d.distance <= 6 ? 2 : 1.5;
      })
      .attr('opacity', d => d.discovered ? 0.9 : 0.6)
      .attr('stroke-dasharray', d => d.discovered ? 'none' : '3,3')
      .style('cursor', 'pointer')
      .on('mouseover', function(event, d) {
        d3.select(this)
          .attr('stroke-width', d.discovered ? 4 : 3)
          .attr('opacity', 1);
        
        // Show tooltip
        const fromSystem = galaxy.systems.find(s => s.id === d.from);
        const toSystem = galaxy.systems.find(s => s.id === d.to);
        if (fromSystem && toSystem) {
          console.log(`Warp Lane: ${fromSystem.name} ↔ ${toSystem.name} (${d.distance.toFixed(1)} LY)`);
        }
      })
      .on('mouseout', function(event, d) {
        d3.select(this)
          .attr('stroke-width', d.discovered ? 2.5 : (d.distance <= 6 ? 2 : 1.5))
          .attr('opacity', d.discovered ? 0.9 : 0.6);
      });

    // Draw anomalies
    const anomalyGroup = mainGroup.append('g').attr('class', 'anomalies');
    
    const anomalyColors = {
      nebula: '#ff6b9d',
      blackhole: '#1a1a1a',
      wormhole: '#9b59b6',
      artifact: '#f39c12',
      resource: '#27ae60'
    };

    anomalyGroup.selectAll('circle')
      .data(galaxy.anomalies)
      .enter()
      .append('circle')
      .attr('cx', d => xScale(d.x))
      .attr('cy', d => yScale(d.y))
      .attr('r', d => d.discovered ? 8 : 6)
      .attr('fill', d => anomalyColors[d.type])
      .attr('stroke', d => d.discovered ? '#fff' : 'none')
      .attr('stroke-width', 1)
      .attr('opacity', d => d.discovered ? 0.9 : 0.6)
      .style('cursor', 'pointer')
      .on('click', (event, d) => {
        event.stopPropagation();
        onAnomalyClick?.(d);
      })
      .on('mouseover', function(event, d) {
        // Show tooltip
        d3.select(this).attr('r', d.discovered ? 10 : 8);
        
        // Create tooltip
        const tooltip = d3.select('body').append('div')
          .attr('class', 'galaxy-tooltip')
          .style('position', 'absolute')
          .style('background', 'rgba(0, 0, 0, 0.8)')
          .style('color', 'white')
          .style('padding', '8px')
          .style('border-radius', '4px')
          .style('font-size', '12px')
          .style('pointer-events', 'none')
          .style('z-index', '1000')
          .html(`
            <strong>${d.name}</strong><br/>
            Type: ${d.type}<br/>
            ${d.discovered ? 'Discovered' : 'Unknown'}
          `);

        tooltip
          .style('left', (event.pageX + 10) + 'px')
          .style('top', (event.pageY - 10) + 'px');
      })
      .on('mouseout', function(event, d) {
        d3.select(this).attr('r', d.discovered ? 8 : 6);
        d3.selectAll('.galaxy-tooltip').remove();
      });

    // Draw star systems
    const systemGroup = mainGroup.append('g').attr('class', 'star-systems');
    
    const systemColors = {
      origin: '#ffd700',
      major: '#00bfff',
      minor: '#87ceeb',
      frontier: '#ddd'
    };

    const systemSizes = {
      origin: 12,
      major: 8,
      minor: 6,
      frontier: 4
    };

    systemGroup.selectAll('circle')
      .data(galaxy.systems)
      .enter()
      .append('circle')
      .attr('cx', d => xScale(d.x))
      .attr('cy', d => yScale(d.y))
      .attr('r', d => systemSizes[d.type])
      .attr('fill', d => systemColors[d.type])
      .attr('stroke', d => {
        if (selectedSystem === d.id) return '#fff';
        if (d.explored) return '#fff';
        return 'none';
      })
      .attr('stroke-width', d => selectedSystem === d.id ? 3 : 1)
      .attr('opacity', d => d.explored ? 1.0 : 0.7)
      .style('cursor', 'pointer')
      .on('click', (event, d) => {
        event.stopPropagation();
        setSelectedSystem(d.id);
        onSystemClick?.(d);
      })
      .on('mouseover', function(event, d) {
        // Highlight system
        d3.select(this)
          .attr('r', systemSizes[d.type] + 2)
          .attr('stroke', '#fff')
          .attr('stroke-width', 2);
        
        // Create tooltip
        const tooltip = d3.select('body').append('div')
          .attr('class', 'galaxy-tooltip')
          .style('position', 'absolute')
          .style('background', 'rgba(0, 0, 0, 0.8)')
          .style('color', 'white')
          .style('padding', '8px')
          .style('border-radius', '4px')
          .style('font-size', '12px')
          .style('pointer-events', 'none')
          .style('z-index', '1000')
          .html(`
            <strong>${d.name}</strong><br/>
            Type: ${d.type}<br/>
            Position: (${d.x.toFixed(1)}, ${d.y.toFixed(1)}) LY<br/>
            Connections: ${d.connections.length}<br/>
            ${d.explored ? 'Explored' : 'Unexplored'}<br/>
            ${d.population ? `Population: ${d.population.toLocaleString()}` : ''}
          `);

        tooltip
          .style('left', (event.pageX + 10) + 'px')
          .style('top', (event.pageY - 10) + 'px');
      })
      .on('mouseout', function(event, d) {
        d3.select(this)
          .attr('r', systemSizes[d.type])
          .attr('stroke', selectedSystem === d.id ? '#fff' : (d.explored ? '#fff' : 'none'))
          .attr('stroke-width', selectedSystem === d.id ? 3 : 1);
        
        d3.selectAll('.galaxy-tooltip').remove();
      });

    // Add system labels for major systems
    systemGroup.selectAll('text')
      .data(galaxy.systems.filter(s => s.type === 'origin' || s.type === 'major'))
      .enter()
      .append('text')
      .attr('x', d => xScale(d.x))
      .attr('y', d => yScale(d.y) + systemSizes[d.type] + 15)
      .attr('text-anchor', 'middle')
      .attr('fill', '#fff')
      .attr('font-size', '12px')
      .attr('font-weight', d => d.type === 'origin' ? 'bold' : 'normal')
      .text(d => d.name)
      .style('pointer-events', 'none');

    // Add legend
    const legend = svg.append('g')
      .attr('class', 'legend')
      .attr('transform', `translate(${width - 200}, 20)`);

    const legendData = [
      { type: 'Origin System', color: systemColors.origin, size: systemSizes.origin },
      { type: 'Major System', color: systemColors.major, size: systemSizes.major },
      { type: 'Minor System', color: systemColors.minor, size: systemSizes.minor },
      { type: 'Frontier System', color: systemColors.frontier, size: systemSizes.frontier }
    ];

    legend.selectAll('g')
      .data(legendData)
      .enter()
      .append('g')
      .attr('transform', (d, i) => `translate(0, ${i * 25})`)
      .each(function(d) {
        const g = d3.select(this);
        g.append('circle')
          .attr('cx', 10)
          .attr('cy', 0)
          .attr('r', d.size)
          .attr('fill', d.color);
        g.append('text')
          .attr('x', 25)
          .attr('y', 5)
          .attr('fill', '#fff')
          .attr('font-size', '12px')
          .text(d.type);
      });

    // Add galaxy info
    const info = svg.append('g')
      .attr('class', 'galaxy-info')
      .attr('transform', 'translate(20, 20)');

    info.append('text')
      .attr('x', 0)
      .attr('y', 0)
      .attr('fill', '#fff')
      .attr('font-size', '14px')
      .attr('font-weight', 'bold')
      .text(`Galaxy Seed: ${galaxy.config.seed}`);

    info.append('text')
      .attr('x', 0)
      .attr('y', 20)
      .attr('fill', '#ccc')
      .attr('font-size', '12px')
      .text(`${galaxy.systems.length} systems, ${galaxy.anomalies.length} anomalies`);

    info.append('text')
      .attr('x', 0)
      .attr('y', 40)
      .attr('fill', '#ccc')
      .attr('font-size', '12px')
      .text(`${galaxy.warpLanes.length} warp lanes, ${galaxy.config.radius} LY radius`);

  }, [galaxy, selectedSystem, onSystemClick, onAnomalyClick]);

  if (loading) {
    return (
      <div className={`flex items-center justify-center h-96 ${className}`}>
        <div className="text-white">Generating galaxy...</div>
      </div>
    );
  }

  if (error) {
    return (
      <div className={`flex items-center justify-center h-96 ${className}`}>
        <div className="text-red-400">
          <div className="text-lg font-bold mb-2">Galaxy Generation Failed</div>
          <div className="text-sm">{error}</div>
        </div>
      </div>
    );
  }

  if (!galaxy) {
    return (
      <div className={`flex items-center justify-center h-96 ${className}`}>
        <div className="text-white">No galaxy data available</div>
      </div>
    );
  }

  return (
    <div className={`galaxy-map ${className}`}>
      <svg
        ref={svgRef}
        width={galaxy.config.visualization.width}
        height={galaxy.config.visualization.height}
        style={{ background: '#0a0a0a', border: '1px solid #333' }}
      />
    </div>
  );
};
