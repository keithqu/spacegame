import React, { useState } from 'react';
import { GalaxyMap } from '../components/GalaxyMap';
import { StarSystem, Anomaly, GalaxyConfig, DEFAULT_GALAXY_CONFIG } from '../types/galaxy';

export const GalaxyPage: React.FC = () => {
  const [config, setConfig] = useState<Partial<GalaxyConfig>>(DEFAULT_GALAXY_CONFIG);
  const [selectedSystem, setSelectedSystem] = useState<StarSystem | null>(null);
  const [selectedAnomaly, setSelectedAnomaly] = useState<Anomaly | null>(null);

  const handleSystemClick = (system: StarSystem) => {
    setSelectedSystem(system);
    setSelectedAnomaly(null);
    console.log('Selected system:', system);
  };

  const handleAnomalyClick = (anomaly: Anomaly) => {
    setSelectedAnomaly(anomaly);
    setSelectedSystem(null);
    console.log('Selected anomaly:', anomaly);
  };

  const handleConfigChange = (newConfig: Partial<GalaxyConfig>) => {
    setConfig({ ...config, ...newConfig });
  };

  const regenerateGalaxy = () => {
    const newSeed = Math.floor(Math.random() * 1000000);
    handleConfigChange({ seed: newSeed });
  };

  return (
    <div className="min-h-screen bg-gray-900 text-white">
      <div className="container mx-auto px-4 py-8">
        <h1 className="text-4xl font-bold mb-8 text-center">
          🌌 Space 4X Galaxy Map
        </h1>
        
        {/* Controls */}
        <div className="mb-6 bg-gray-800 rounded-lg p-6">
          <h2 className="text-xl font-semibold mb-4">Galaxy Configuration</h2>
          
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
            <div>
              <label className="block text-sm font-medium mb-2">
                Seed
              </label>
              <input
                type="number"
                value={config.seed || 42}
                onChange={(e) => handleConfigChange({ seed: parseInt(e.target.value) || 42 })}
                className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded-md text-white"
              />
            </div>
            
            <div>
              <label className="block text-sm font-medium mb-2">
                Radius (Light Years)
              </label>
              <input
                type="number"
                value={config.radius || 500}
                onChange={(e) => handleConfigChange({ radius: parseInt(e.target.value) || 500 })}
                className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded-md text-white"
              />
            </div>
            
            <div>
              <label className="block text-sm font-medium mb-2">
                Star Systems
              </label>
              <input
                type="number"
                value={config.starSystemCount || 350}
                onChange={(e) => handleConfigChange({ starSystemCount: parseInt(e.target.value) || 350 })}
                className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded-md text-white"
              />
            </div>
            
            <div>
              <label className="block text-sm font-medium mb-2">
                Anomalies
              </label>
              <input
                type="number"
                value={config.anomalyCount || 50}
                onChange={(e) => handleConfigChange({ anomalyCount: parseInt(e.target.value) || 50 })}
                className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded-md text-white"
              />
            </div>
          </div>
          
          <div className="mt-4 flex gap-4">
            <button
              onClick={regenerateGalaxy}
              className="px-4 py-2 bg-blue-600 hover:bg-blue-700 rounded-md font-medium transition-colors"
            >
              🎲 Random Galaxy
            </button>
            
            <button
              onClick={() => handleConfigChange({ seed: 42 })}
              className="px-4 py-2 bg-green-600 hover:bg-green-700 rounded-md font-medium transition-colors"
            >
              🏠 Default Galaxy
            </button>
          </div>
        </div>

        {/* Galaxy Map */}
        <div className="flex flex-col lg:flex-row gap-6">
          <div className="flex-1">
            <GalaxyMap
              config={config}
              onSystemClick={handleSystemClick}
              onAnomalyClick={handleAnomalyClick}
              className="w-full"
            />
          </div>
          
          {/* Selection Panel */}
          <div className="lg:w-80 bg-gray-800 rounded-lg p-6">
            <h2 className="text-xl font-semibold mb-4">Selection Details</h2>
            
            {selectedSystem && (
              <div className="space-y-3">
                <h3 className="text-lg font-medium text-blue-400">
                  ⭐ {selectedSystem.name}
                </h3>
                
                <div className="space-y-2 text-sm">
                  <div>
                    <span className="font-medium">Type:</span> {selectedSystem.type}
                  </div>
                  <div>
                    <span className="font-medium">Position:</span> ({selectedSystem.x.toFixed(1)}, {selectedSystem.y.toFixed(1)}) LY
                  </div>
                  <div>
                    <span className="font-medium">Distance from Sol:</span> {
                      Math.sqrt(selectedSystem.x * selectedSystem.x + selectedSystem.y * selectedSystem.y).toFixed(1)
                    } LY
                  </div>
                  <div>
                    <span className="font-medium">Connections:</span> {selectedSystem.connections.length}
                  </div>
                  <div>
                    <span className="font-medium">Status:</span> {selectedSystem.explored ? '✅ Explored' : '❓ Unexplored'}
                  </div>
                  {selectedSystem.population && selectedSystem.population > 0 && (
                    <div>
                      <span className="font-medium">Population:</span> {selectedSystem.population.toLocaleString()}
                    </div>
                  )}
                  
                  {selectedSystem.resources && (
                    <div className="mt-3">
                      <div className="font-medium mb-1">Resources:</div>
                      <div className="pl-2 space-y-1 text-xs">
                        <div>🔩 Minerals: {selectedSystem.resources.minerals}</div>
                        <div>⚡ Energy: {selectedSystem.resources.energy}</div>
                        <div>🧪 Research: {selectedSystem.resources.research}</div>
                      </div>
                    </div>
                  )}
                </div>
              </div>
            )}
            
            {selectedAnomaly && (
              <div className="space-y-3">
                <h3 className="text-lg font-medium text-purple-400">
                  🌟 {selectedAnomaly.name}
                </h3>
                
                <div className="space-y-2 text-sm">
                  <div>
                    <span className="font-medium">Type:</span> {selectedAnomaly.type}
                  </div>
                  <div>
                    <span className="font-medium">Position:</span> ({selectedAnomaly.x.toFixed(1)}, {selectedAnomaly.y.toFixed(1)}) LY
                  </div>
                  <div>
                    <span className="font-medium">Distance from Sol:</span> {
                      Math.sqrt(selectedAnomaly.x * selectedAnomaly.x + selectedAnomaly.y * selectedAnomaly.y).toFixed(1)
                    } LY
                  </div>
                  <div>
                    <span className="font-medium">Status:</span> {selectedAnomaly.discovered ? '✅ Discovered' : '❓ Unknown'}
                  </div>
                  
                  {selectedAnomaly.effect && (
                    <div className="mt-3">
                      <div className="font-medium mb-1">Effect:</div>
                      <div className="pl-2 text-xs">
                        {selectedAnomaly.effect.type.replace('_', ' ')}: {selectedAnomaly.effect.value > 0 ? '+' : ''}{selectedAnomaly.effect.value}
                      </div>
                    </div>
                  )}
                </div>
              </div>
            )}
            
            {!selectedSystem && !selectedAnomaly && (
              <div className="text-gray-400 text-center py-8">
                Click on a star system or anomaly to view details
              </div>
            )}
            
            {/* Instructions */}
            <div className="mt-6 pt-6 border-t border-gray-700">
              <h3 className="font-medium mb-2">Controls:</h3>
              <ul className="text-sm text-gray-400 space-y-1">
                <li>• Click and drag to pan</li>
                <li>• Scroll to zoom in/out</li>
                <li>• Click systems/anomalies for details</li>
                <li>• Hover for quick info</li>
              </ul>
            </div>
          </div>
        </div>
        
        {/* Stats */}
        <div className="mt-6 bg-gray-800 rounded-lg p-6">
          <h2 className="text-xl font-semibold mb-4">Galaxy Statistics</h2>
          <div className="grid grid-cols-2 md:grid-cols-4 gap-4 text-center">
            <div>
              <div className="text-2xl font-bold text-blue-400">{config.starSystemCount || 350}</div>
              <div className="text-sm text-gray-400">Star Systems</div>
            </div>
            <div>
              <div className="text-2xl font-bold text-purple-400">{config.anomalyCount || 50}</div>
              <div className="text-sm text-gray-400">Anomalies</div>
            </div>
            <div>
              <div className="text-2xl font-bold text-green-400">{config.radius || 500}</div>
              <div className="text-sm text-gray-400">Light Years Radius</div>
            </div>
            <div>
              <div className="text-2xl font-bold text-yellow-400">{config.seed || 42}</div>
              <div className="text-sm text-gray-400">Galaxy Seed</div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
