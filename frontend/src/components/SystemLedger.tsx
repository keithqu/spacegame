import React, { useState, useEffect } from 'react'
import { StarSystem, Galaxy, DetailedSystemData } from '../types/galaxy'
import { GalaxyApiService } from '../services/galaxyApi'

interface SystemLedgerProps {
  system: StarSystem
  galaxy: Galaxy
  onPlanetSelect?: (planetId: string) => void
  selectedPlanetId?: string | null
}

export const SystemLedger: React.FC<SystemLedgerProps> = ({
  system,
  galaxy,
  onPlanetSelect,
  selectedPlanetId,
}) => {
  const [activeTab, setActiveTab] = useState<'overview' | 'planets' | 'resources'>('overview')
  const [detailedSystemData, setDetailedSystemData] = useState<DetailedSystemData | null>(null)

  const connectedSystems = galaxy.systems.filter(s => system.connections.includes(s.id))

  // Fetch detailed system data if available
  useEffect(() => {
    const fetchDetailedData = async () => {
      if (system.hasDetailedData) {
        try {
          const detailedData = await GalaxyApiService.getSystemDetails(system.id)
          setDetailedSystemData(detailedData)
        } catch (error) {
          console.error('Failed to fetch detailed system data:', error)
          setDetailedSystemData(null)
        }
      } else {
        setDetailedSystemData(null)
      }
    }

    fetchDetailedData()
  }, [system.id, system.hasDetailedData])

  const formatNumber = (num: number) => {
    if (num >= 1000000) {
      return (num / 1000000).toFixed(1) + 'M'
    } else if (num >= 1000) {
      return (num / 1000).toFixed(1) + 'K'
    }
    return num.toString()
  }

  return (
    <div className="system-ledger">
      {/* Header */}
      <div className="ledger-header">
        <h3>{system.name}</h3>
        <div className="system-stats">
          <div className="stat">
            <div className="stat-value">{system.systemInfo?.planetCount || 0}</div>
            <div className="stat-label">Planets</div>
          </div>
          <div className="stat">
            <div className="stat-value">{system.systemInfo?.moonCount || 0}</div>
            <div className="stat-label">Moons</div>
          </div>
          <div className="stat">
            <div className="stat-value">{system.connections.length}</div>
            <div className="stat-label">Lanes</div>
          </div>
        </div>
      </div>

      {/* Tabs */}
      <div className="ledger-tabs">
        <button
          className={`tab ${activeTab === 'overview' ? 'active' : ''}`}
          onClick={() => setActiveTab('overview')}
        >
          Overview
        </button>
        <button
          className={`tab ${activeTab === 'planets' ? 'active' : ''}`}
          onClick={() => setActiveTab('planets')}
        >
          Planets
        </button>
        <button
          className={`tab ${activeTab === 'resources' ? 'active' : ''}`}
          onClick={() => setActiveTab('resources')}
        >
          Resources
        </button>
      </div>

      {/* Content */}
      <div className="ledger-content">
        {activeTab === 'overview' && (
          <div className="overview-tab">
            <div className="section">
              <h4>System Information</h4>
              <div className="info-grid">
                <div className="info-item">
                  <span className="label">Star Type:</span>
                  <span className="value">{system.systemInfo?.starType || 'Unknown'}</span>
                </div>
                <div className="info-item">
                  <span className="label">Population:</span>
                  <span className="value">
                    {system.population ? formatNumber(system.population) : 'Uninhabited'}
                  </span>
                </div>
                <div className="info-item">
                  <span className="label">GDP:</span>
                  <span className="value">
                    {system.gdp ? `${formatNumber(system.gdp)} credits` : 'No economy'}
                  </span>
                </div>
                <div className="info-item">
                  <span className="label">Status:</span>
                  <span className="value">{system.explored ? '✅ Explored' : '❓ Unexplored'}</span>
                </div>
              </div>
            </div>

            <div className="section">
              <h4>Connected Systems ({connectedSystems.length})</h4>
              <div className="connected-systems">
                {connectedSystems.map(connectedSystem => (
                  <div key={connectedSystem.id} className="connected-system">
                    <span className="system-name">{connectedSystem.name}</span>
                    <span className="system-distance">
                      {Math.sqrt(
                        Math.pow(system.x - connectedSystem.x, 2) +
                          Math.pow(system.y - connectedSystem.y, 2)
                      ).toFixed(1)}{' '}
                      LY
                    </span>
                  </div>
                ))}
              </div>
            </div>
          </div>
        )}

        {activeTab === 'planets' && (
          <div className="planets-tab">
            <div className="section">
              <h4>Planetary Bodies</h4>
              <div className="planets-list">
                {detailedSystemData?.planets.flatMap(planet => [
                  // Planet entry
                  <div
                    key={planet.id}
                    className={`planet-item ${selectedPlanetId === planet.id ? 'selected' : ''}`}
                    onClick={() => onPlanetSelect?.(planet.id)}
                  >
                    <div className="planet-info">
                      <div className="planet-name">{planet.name}</div>
                      <div className="planet-type">
                        {planet.habitability > 50
                          ? 'Habitable'
                          : planet.composition.includes('gas')
                            ? 'Gas Giant'
                            : planet.composition.includes('ice')
                              ? 'Ice World'
                              : 'Terrestrial'}
                      </div>
                    </div>
                    <div className="planet-stats">
                      <div className="planet-population">
                        Diameter: {planet.diameter.toLocaleString()} km | Gravity: {planet.gravity}%
                      </div>
                      <div className="planet-population">
                        Habitability: {planet.habitability}% | Mass: {planet.mass.toFixed(2)} Earth
                        masses
                      </div>
                      {planet.moons.length > 0 && (
                        <div className="planet-moons">
                          {planet.moons.length} moon{planet.moons.length > 1 ? 's' : ''}
                        </div>
                      )}
                    </div>
                  </div>,
                  // Moon entries
                  ...planet.moons.map(moon => (
                    <div
                      key={moon.id}
                      className={`planet-item moon-item ${selectedPlanetId === moon.id ? 'selected' : ''}`}
                      onClick={() => onPlanetSelect?.(moon.id)}
                    >
                      <div className="planet-info">
                        <div className="planet-name"> └ {moon.name}</div>
                        <div className="planet-type">Moon of {planet.name}</div>
                      </div>
                      <div className="planet-stats">
                        <div className="planet-population">
                          Diameter: {moon.diameter.toLocaleString()} km | Gravity: {moon.gravity}%
                        </div>
                        <div className="planet-population">
                          Habitability: {moon.habitability}% | Mass: {moon.mass.toFixed(4)} Earth
                          masses
                        </div>
                      </div>
                    </div>
                  )),
                ]) || []}

                {detailedSystemData?.asteroids.map(asteroid => (
                  <div
                    key={asteroid.id}
                    className={`planet-item ${selectedPlanetId === asteroid.id ? 'selected' : ''}`}
                    onClick={() => onPlanetSelect?.(asteroid.id)}
                  >
                    <div className="planet-info">
                      <div className="planet-name">{asteroid.name}</div>
                      <div className="planet-type">Asteroid</div>
                    </div>
                    <div className="planet-stats">
                      <div className="planet-population">
                        Diameter: {asteroid.diameter.toFixed(0)} km | Mass:{' '}
                        {asteroid.mass.toFixed(6)} Earth masses
                      </div>
                    </div>
                  </div>
                )) || []}

                {!detailedSystemData && (
                  <div className="planet-item">
                    <div className="planet-info">
                      <div className="planet-name">Procedural System</div>
                      <div className="planet-type">Generated Content</div>
                    </div>
                    <div className="planet-stats">
                      <div className="planet-population">
                        {system.systemInfo?.planetCount || 0} planets,{' '}
                        {system.systemInfo?.moonCount || 0} moons
                      </div>
                    </div>
                  </div>
                )}
              </div>
            </div>
          </div>
        )}

        {activeTab === 'resources' && (
          <div className="resources-tab">
            <div className="section">
              <h4>System Resources</h4>
              {system.resources && (
                <div className="resource-grid">
                  <div className="resource-item">
                    <div className="resource-icon">⛏️</div>
                    <div className="resource-info">
                      <div className="resource-name">Minerals</div>
                      <div className="resource-amount">{system.resources.minerals}</div>
                    </div>
                  </div>
                  <div className="resource-item">
                    <div className="resource-icon">⚡</div>
                    <div className="resource-info">
                      <div className="resource-name">Energy</div>
                      <div className="resource-amount">{system.resources.energy}</div>
                    </div>
                  </div>
                  <div className="resource-item">
                    <div className="resource-icon">🔬</div>
                    <div className="resource-info">
                      <div className="resource-name">Research</div>
                      <div className="resource-amount">{system.resources.research}</div>
                    </div>
                  </div>
                </div>
              )}
            </div>
          </div>
        )}
      </div>
    </div>
  )
}
