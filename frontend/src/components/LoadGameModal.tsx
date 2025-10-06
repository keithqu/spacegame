import React, { useEffect, useState } from 'react'
import { GalaxyApiService } from '../services/galaxyApi'

interface LoadGameModalProps {
  onClose: () => void
  onLoaded: (galaxyJson: any) => void
}

interface SaveMeta {
  id: string
  save_slot: number
  created_at: string
  updated_at: string
}

export const LoadGameModal: React.FC<LoadGameModalProps> = ({ onClose, onLoaded }) => {
  const [saves, setSaves] = useState<SaveMeta[]>([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    const fetchSaves = async () => {
      try {
        setLoading(true)
        setError(null)
        const list = await GalaxyApiService.listSaves()
        setSaves(list)
      } catch (e) {
        setError(e instanceof Error ? e.message : 'Failed to load saves')
      } finally {
        setLoading(false)
      }
    }
    fetchSaves()
  }, [])

  const handleLoad = async (id: string) => {
    try {
      const data = await GalaxyApiService.loadSave(id)
      onLoaded(data)
      onClose()
    } catch (e) {
      setError(e instanceof Error ? e.message : 'Failed to load save')
    }
  }

  return (
    <div className="modal-backdrop">
      <div className="modal">
        <div className="modal-header">
          <h3>Load Game</h3>
          <button onClick={onClose}>✖</button>
        </div>
        <div className="modal-body">
          {loading && <div>Loading saves...</div>}
          {error && <div className="error">{error}</div>}
          {!loading && !error && (
            <div className="save-list">
              {saves.length === 0 && <div>No saves found.</div>}
              {saves.map(save => (
                <div key={save.id} className="save-item">
                  <div>
                    <div>Slot {save.save_slot}</div>
                    <div className="muted">
                      Updated {new Date(save.updated_at).toLocaleString()}
                    </div>
                  </div>
                  <button onClick={() => handleLoad(save.id)}>Load</button>
                </div>
              ))}
            </div>
          )}
        </div>
      </div>
      <style>
        {`
        .modal-backdrop { position: fixed; inset: 0; background: rgba(0,0,0,0.6); display: flex; align-items: center; justify-content: center; }
        .modal { background: #0b1020; color: #fff; border: 1px solid #3b82f6; border-radius: 8px; width: 480px; max-width: 90vw; }
        .modal-header { display: flex; align-items: center; justify-content: space-between; padding: 12px 16px; border-bottom: 1px solid #1f2937; }
        .modal-body { padding: 12px 16px; }
        .save-list { display: flex; flex-direction: column; gap: 8px; }
        .save-item { display: flex; align-items: center; justify-content: space-between; border: 1px solid #1f2937; padding: 10px 12px; border-radius: 6px; }
        .muted { color: #9ca3af; font-size: 12px; }
        .error { color: #f87171; margin-bottom: 8px; }
        button { background: #1f2937; color: #fff; border: 1px solid #3b82f6; padding: 6px 10px; border-radius: 6px; cursor: pointer; }
        button:hover { background: #111827; }
      `}
      </style>
    </div>
  )
}
