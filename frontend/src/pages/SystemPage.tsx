import React, { useState, useCallback } from 'react'
import { StarSystem, Galaxy } from '../types/galaxy'
import { SystemOrbitMap } from '../components/SystemOrbitMap'
import { SystemLedger } from '../components/SystemLedger'

interface SystemPageProps {
  system: StarSystem
  galaxy: Galaxy
  onBackToGalaxy: () => void
}

export const SystemPage: React.FC<SystemPageProps> = ({ system, galaxy, onBackToGalaxy }) => {
  const [selectedPlanet, setSelectedPlanet] = useState<string | null>(null)

  const handlePlanetClick = useCallback((planetId: string) => {
    setSelectedPlanet(planetId)
  }, [])

  return (
    <div className="system-container">
      {/* System Orbital Map */}
      <div className="system-main">
        <div className="orbital-map-container">
          <SystemOrbitMap
            system={system}
            onPlanetClick={handlePlanetClick}
            selectedPlanetId={selectedPlanet}
          />
        </div>
        <div className="system-ledger-container">
          <SystemLedger
            system={system}
            galaxy={galaxy}
            onPlanetSelect={planetId => setSelectedPlanet(planetId)}
            selectedPlanetId={selectedPlanet}
          />
        </div>
      </div>

      {/* System Info UI (Bottom) */}
      <div className="system-info-ui">
        <div className="system-info-placeholder">🌟 System Command Interface - {system.name}</div>

        {/* Back to Galaxy Button */}
        <button className="back-to-galaxy-button" onClick={onBackToGalaxy}>
          🌌 Back to Galaxy
        </button>
      </div>
    </div>
  )
}
