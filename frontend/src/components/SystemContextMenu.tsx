import React from 'react'
import { StarSystem } from '../types/galaxy'

interface SystemContextMenuProps {
  system: StarSystem
  position: { x: number; y: number }
  onClose: () => void
  onViewSystem: (system: StarSystem) => void
}

export const SystemContextMenu: React.FC<SystemContextMenuProps> = ({
  system,
  position,
  onClose,
  onViewSystem,
}) => {
  const handleViewSystem = () => {
    onViewSystem(system)
    onClose()
  }

  const formatNumber = (num: number) => {
    if (num >= 1000000) {
      return (num / 1000000).toFixed(1) + 'M'
    } else if (num >= 1000) {
      return (num / 1000).toFixed(1) + 'K'
    }
    return num.toString()
  }

  return (
    <>
      {/* Backdrop to close menu when clicking outside */}
      <div className="context-menu-backdrop" onClick={onClose} />

      {/* Context menu */}
      <div
        className="system-context-menu"
        style={{
          left: `${position.x + 10}px`,
          top: `${position.y - 10}px`,
        }}
      >
        <div className="context-menu-header">
          <h3>{system.name}</h3>
          <div className="system-type-badge">{system.type}</div>
        </div>

        <div className="context-menu-content">
          <div className="system-stat">
            <span className="stat-label">Population:</span>
            <span className="stat-value">
              {system.population ? formatNumber(system.population) : 'Uninhabited'}
            </span>
          </div>

          <div className="system-stat">
            <span className="stat-label">GDP:</span>
            <span className="stat-value">
              {system.gdp ? `${formatNumber(system.gdp)} credits` : 'No economy'}
            </span>
          </div>

          {system.systemInfo && (
            <>
              <div className="system-stat">
                <span className="stat-label">Star Type:</span>
                <span className="stat-value">{system.systemInfo.starType}</span>
              </div>

              <div className="system-stat">
                <span className="stat-label">Planets:</span>
                <span className="stat-value">{system.systemInfo.planetCount}</span>
              </div>

              <div className="system-stat">
                <span className="stat-label">Moons:</span>
                <span className="stat-value">{system.systemInfo.moonCount}</span>
              </div>
            </>
          )}

          <div className="system-stat">
            <span className="stat-label">Status:</span>
            <span className="stat-value">{system.explored ? '✅ Explored' : '❓ Unexplored'}</span>
          </div>

          <div className="system-stat">
            <span className="stat-label">Connections:</span>
            <span className="stat-value">{system.connections.length} lanes</span>
          </div>
        </div>

        <div className="context-menu-actions">
          <button className="view-system-button" onClick={handleViewSystem}>
            🔍 View System
          </button>
        </div>
      </div>
    </>
  )
}
