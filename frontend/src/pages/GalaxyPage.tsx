import React, { useState, useCallback } from 'react'
import { GalaxyMap } from '../components/GalaxyMap'
import { GalaxyLedger } from '../components/GalaxyLedger'
import { SystemPage } from './SystemPage'
import { StarSystem, Anomaly, GalaxyConfig, DEFAULT_GALAXY_CONFIG, Galaxy } from '../types/galaxy'

export const GalaxyPage: React.FC = () => {
  const [config, setConfig] = useState<Partial<GalaxyConfig>>(DEFAULT_GALAXY_CONFIG)
  const [selectedSystem, setSelectedSystem] = useState<StarSystem | null>(null)
  const [selectedAnomaly, setSelectedAnomaly] = useState<Anomaly | null>(null)
  const [galaxy, setGalaxy] = useState<Galaxy | null>(null)
  const [viewingSystem, setViewingSystem] = useState<StarSystem | null>(null)

  const handleSystemClick = (system: StarSystem) => {
    setSelectedSystem(system)
    setSelectedAnomaly(null)
    console.log('Selected system:', system)
  }

  const handleAnomalyClick = (anomaly: Anomaly) => {
    setSelectedAnomaly(anomaly)
    setSelectedSystem(null)
    console.log('Selected anomaly:', anomaly)
  }

  const handleSystemSelect = (system: StarSystem) => {
    setSelectedSystem(system)
    setSelectedAnomaly(null)
  }

  const handleViewSystem = (system: StarSystem) => {
    setViewingSystem(system)
  }

  const handleBackToGalaxy = () => {
    setViewingSystem(null)
  }

  const handleGalaxyLoad = useCallback((loadedGalaxy: Galaxy) => {
    setGalaxy(loadedGalaxy)
  }, [])

  const handleConfigChange = (newConfig: Partial<GalaxyConfig>) => {
    setConfig({ ...config, ...newConfig })
  }

  const regenerateGalaxy = () => {
    const newSeed = Math.floor(Math.random() * 1000000)
    handleConfigChange({ seed: newSeed })
  }

  // If viewing a system, show the system page
  if (viewingSystem && galaxy) {
    return (
      <SystemPage
        system={viewingSystem}
        galaxy={galaxy}
        onBackToGalaxy={handleBackToGalaxy}
      />
    )
  }

  return (
    <div className="game-container">
      {/* Debug Controls (Top-Left) */}
      <div className="debug-controls">
        <h3>Debug Controls</h3>

        <div className="control-group">
          <label>Seed</label>
          <input
            type="number"
            value={config.seed || 42}
            onChange={e => handleConfigChange({ seed: parseInt(e.target.value) || 42 })}
          />
        </div>

        <div className="control-group">
          <label>Radius (LY)</label>
          <input
            type="number"
            value={config.radius || 500}
            onChange={e => handleConfigChange({ radius: parseInt(e.target.value) || 500 })}
          />
        </div>

        <div className="control-group">
          <label>Systems</label>
          <input
            type="number"
            value={config.starSystemCount || 400}
            onChange={e => handleConfigChange({ starSystemCount: parseInt(e.target.value) || 400 })}
          />
        </div>

        <div className="control-group">
          <label>Anomalies</label>
          <input
            type="number"
            value={config.anomalyCount || 25}
            onChange={e => handleConfigChange({ anomalyCount: parseInt(e.target.value) || 25 })}
          />
        </div>

        <button onClick={regenerateGalaxy}>🎲 Random</button>

        <button onClick={() => handleConfigChange({ seed: 42 })}>🏠 Default</button>
      </div>

      {/* Main Game Area */}
      <div className="game-main">
        <div className="galaxy-map-container">
          <GalaxyMap
            config={config}
            onSystemClick={handleSystemClick}
            onAnomalyClick={handleAnomalyClick}
            onGalaxyLoad={handleGalaxyLoad}
            onViewSystem={handleViewSystem}
            selectedSystemId={selectedSystem?.id || null}
          />
        </div>
        <div className="galaxy-ledger-container">
          <GalaxyLedger
            galaxy={galaxy}
            onSystemSelect={handleSystemSelect}
            selectedSystemId={selectedSystem?.id || null}
          />
        </div>
      </div>

      {/* Game Info UI (Bottom) */}
      <div className="game-info-ui">
        <div className="game-info-placeholder">🌌 Galaxy Command Interface - Coming Soon</div>

        {/* Selection Info (if something is selected) */}
        {(selectedSystem || selectedAnomaly) && (
          <div
            style={{
              position: 'absolute',
              left: '20px',
              top: '20px',
              background: 'rgba(0, 0, 0, 0.8)',
              padding: '15px',
              borderRadius: '8px',
              border: '1px solid #3b82f6',
              maxWidth: '300px',
            }}
          >
            {selectedSystem && (
              <div>
                <h3 style={{ color: '#60a5fa', marginBottom: '10px' }}>⭐ {selectedSystem.name}</h3>
                <div style={{ fontSize: '12px', lineHeight: '1.4' }}>
                  <div>
                    <strong>Type:</strong> {selectedSystem.type}
                  </div>
                  <div>
                    <strong>Position:</strong> ({selectedSystem.x.toFixed(1)},{' '}
                    {selectedSystem.y.toFixed(1)}) LY
                  </div>
                  <div>
                    <strong>Connections:</strong> {selectedSystem.connections.length}
                  </div>
                  <div>
                    <strong>Status:</strong>{' '}
                    {selectedSystem.explored ? '✅ Explored' : '❓ Unexplored'}
                  </div>
                </div>
              </div>
            )}

            {selectedAnomaly && (
              <div>
                <h3 style={{ color: '#c084fc', marginBottom: '10px' }}>
                  🌟 {selectedAnomaly.name}
                </h3>
                <div style={{ fontSize: '12px', lineHeight: '1.4' }}>
                  <div>
                    <strong>Type:</strong> {selectedAnomaly.type}
                  </div>
                  <div>
                    <strong>Position:</strong> ({selectedAnomaly.x.toFixed(1)},{' '}
                    {selectedAnomaly.y.toFixed(1)}) LY
                  </div>
                  <div>
                    <strong>Status:</strong>{' '}
                    {selectedAnomaly.discovered ? '✅ Discovered' : '❓ Unknown'}
                  </div>
                </div>
              </div>
            )}
          </div>
        )}
      </div>
    </div>
  )
}
