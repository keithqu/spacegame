/* eslint @typescript-eslint/no-unused-vars: ["error", { "argsIgnorePattern": "^_" }] */
/* eslint no-unused-vars: off */

import React, { useState, useMemo } from 'react'
import { StarSystem, Galaxy } from '../types/galaxy'

interface GalaxyLedgerProps {
  galaxy: Galaxy | null
  onSystemSelect?: (system: StarSystem) => void
  selectedSystemId?: string | null
}

export const GalaxyLedger: React.FC<GalaxyLedgerProps> = ({
  galaxy,
  onSystemSelect,
  selectedSystemId,
}) => {
  const [searchQuery, setSearchQuery] = useState('')
  const [activeTab, setActiveTab] = useState<'search' | 'controlled'>('search')

  // Filter systems based on search query
  const filteredSystems = useMemo(() => {
    if (!galaxy?.systems || !searchQuery.trim()) {
      return galaxy?.systems || []
    }

    const query = searchQuery.toLowerCase()
    return galaxy.systems.filter(
      system =>
        system.name.toLowerCase().includes(query) ||
        system.type.toLowerCase().includes(query) ||
        system.id.toLowerCase().includes(query)
    )
  }, [galaxy?.systems, searchQuery])

  // Controlled systems (empty for now - placeholder for future implementation)
  const controlledSystems = useMemo((): StarSystem[] => {
    // TODO: Filter systems controlled by current player
    // For now, return empty array since we haven't implemented player control
    return []
  }, [])

  const handleSystemClick = (system: StarSystem) => {
    onSystemSelect?.(system)
  }

  const getSystemIcon = (system: StarSystem) => {
    switch (system.type) {
      case 'origin':
        return '🌟'
      case 'core':
        return '⭐'
      case 'rim':
        return '✦'
      default:
        return '○'
    }
  }

  const getSystemDistance = (system: StarSystem) => {
    const distance = Math.sqrt(system.x ** 2 + system.y ** 2)
    return distance.toFixed(1)
  }

  if (!galaxy) {
    return (
      <div className="galaxy-ledger">
        <div className="ledger-loading">
          <div>📊 Loading galaxy data...</div>
        </div>
      </div>
    )
  }

  return (
    <div className="galaxy-ledger">
      {/* Header */}
      <div className="ledger-header">
        <h3>Galaxy Ledger</h3>
        <div className="ledger-stats">
          <div className="stat">
            <span className="stat-value">{galaxy.systems.length}</span>
            <span className="stat-label">Systems</span>
          </div>
          <div className="stat">
            <span className="stat-value">{galaxy.warpLanes.length}</span>
            <span className="stat-label">Lanes</span>
          </div>
        </div>
      </div>

      {/* Tab Navigation */}
      <div className="ledger-tabs">
        <button
          className={`tab ${activeTab === 'search' ? 'active' : ''}`}
          onClick={() => setActiveTab('search')}
        >
          🔍 Search
        </button>
        <button
          className={`tab ${activeTab === 'controlled' ? 'active' : ''}`}
          onClick={() => setActiveTab('controlled')}
        >
          🏛️ Controlled
        </button>
      </div>

      {/* Tab Content */}
      <div className="ledger-content">
        {activeTab === 'search' && (
          <div className="search-tab">
            {/* Search Input */}
            <div className="search-section">
              <input
                type="text"
                placeholder="Search systems..."
                value={searchQuery}
                onChange={e => setSearchQuery(e.target.value)}
                className="search-input"
              />
              {searchQuery && (
                <button
                  onClick={() => setSearchQuery('')}
                  className="clear-search"
                  title="Clear search"
                >
                  ✕
                </button>
              )}
            </div>

            {/* Search Results */}
            <div className="systems-list">
              <div className="list-header">
                <span>
                  {searchQuery
                    ? `${filteredSystems.length} results`
                    : `${galaxy.systems.length} systems`}
                </span>
              </div>
              <div className="systems-scroll">
                {filteredSystems.map(system => (
                  <div
                    key={system.id}
                    className={`system-item ${selectedSystemId === system.id ? 'selected' : ''}`}
                    onClick={() => handleSystemClick(system)}
                    onKeyDown={e => {
                      if (e.key === 'Enter' || e.key === ' ') {
                        e.preventDefault()
                        handleSystemClick(system)
                      }
                    }}
                    role="button"
                    tabIndex={0}
                  >
                    <div className="system-icon">{getSystemIcon(system)}</div>
                    <div className="system-info">
                      <div className="system-name">{system.name}</div>
                      <div className="system-details">
                        <span className={`system-type ${system.type}`}>{system.type}</span>
                        <span className="system-distance">{getSystemDistance(system)} LY</span>
                      </div>
                      {system.explored && <div className="system-status">✅ Explored</div>}
                    </div>
                    <div className="system-connections">{system.connections.length}</div>
                  </div>
                ))}
                {filteredSystems.length === 0 && searchQuery && (
                  <div className="no-results">
                    <div>No systems found</div>
                    <div className="no-results-hint">Try searching by name, type, or ID</div>
                  </div>
                )}
              </div>
            </div>
          </div>
        )}

        {activeTab === 'controlled' && (
          <div className="controlled-tab">
            <div className="controlled-section">
              <div className="controlled-header">
                <span>Controlled Systems</span>
                <span className="controlled-count">{controlledSystems.length}</span>
              </div>
              <div className="controlled-list">
                {controlledSystems.length === 0 ? (
                  <div className="no-controlled">
                    <div className="no-controlled-icon">🏛️</div>
                    <div className="no-controlled-title">No Controlled Systems</div>
                    <div className="no-controlled-desc">
                      Player control system not yet implemented.
                      <br />
                      Future features will include:
                    </div>
                    <ul className="future-features">
                      <li>Colony management</li>
                      <li>Fleet deployment</li>
                      <li>Resource production</li>
                      <li>System upgrades</li>
                    </ul>
                  </div>
                ) : (
                  // Future: Render controlled systems
                  controlledSystems.map(system => (
                    <div key={system.id} className="controlled-system">
                      {/* Controlled system UI will go here */}
                    </div>
                  ))
                )}
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  )
}
