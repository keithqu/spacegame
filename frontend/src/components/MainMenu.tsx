import React from 'react'

interface MainMenuProps {
  onNewGame: () => void
  onLoadGame: () => void
}

export const MainMenu: React.FC<MainMenuProps> = ({ onNewGame, onLoadGame }) => {
  return (
    <div
      style={{
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        height: '100vh',
        background: 'radial-gradient(ellipse at center, #0a0a2e 0%, #000000 100%)',
        color: '#fff',
        gap: 16,
      }}
    >
      <div style={{ textAlign: 'center', marginBottom: 24 }}>
        <h1 style={{ margin: 0, fontSize: 32 }}>Space 4X</h1>
        <div style={{ opacity: 0.7, marginTop: 8 }}>Auto-logged in as keith</div>
      </div>

      <button className="menu-button" onClick={onNewGame}>
        New Game
      </button>
      <button className="menu-button" onClick={onLoadGame}>
        Load Game
      </button>
      <button className="menu-button" disabled>
        Options (coming soon)
      </button>
      <style>
        {`
        .menu-button {
          background: #1f2937;
          color: #fff;
          border: 1px solid #3b82f6;
          padding: 12px 24px;
          border-radius: 8px;
          width: 220px;
          cursor: pointer;
          font-size: 16px;
        }
        .menu-button:hover:not([disabled]) {
          background: #111827;
        }
        .menu-button[disabled] {
          opacity: 0.5;
          cursor: not-allowed;
        }
      `}
      </style>
    </div>
  )
}
