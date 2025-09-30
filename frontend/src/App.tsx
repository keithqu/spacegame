import React, { useEffect } from 'react'
import { GalaxyPage } from './pages/GalaxyPage'
import './App.css'

function App() {
  // Disable default browser context menu globally
  useEffect(() => {
    const handleContextMenu = (e: MouseEvent) => {
      e.preventDefault()
    }

    document.addEventListener('contextmenu', handleContextMenu)

    return () => {
      document.removeEventListener('contextmenu', handleContextMenu)
    }
  }, [])

  return (
    <div className="App">
      <GalaxyPage />
    </div>
  )
}

export default App
