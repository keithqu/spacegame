import React, { useEffect, useState } from 'react'
import { UserProvider, useUser } from './contexts/UserContext'
import { GalaxyPage } from './pages/GalaxyPage'
import { MainMenu } from './components/MainMenu'
import { LoadGameModal } from './components/LoadGameModal'
import { LoginPage } from './components/LoginPage'
import './App.css'

const AppContent: React.FC = () => {
  const { user, loading } = useUser()
  const [showMenu, setShowMenu] = useState(true)
  const [showLoadModal, setShowLoadModal] = useState(false)
  const [initialGalaxyJson, setInitialGalaxyJson] = useState<any | null>(null)
  const [startNew, setStartNew] = useState<boolean>(false)

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

  if (loading) {
    return (
      <div className="App">
        <div className="loading-container">
          <div className="loading-spinner"></div>
          <p>Authenticating user...</p>
        </div>
      </div>
    )
  }

  if (!user) {
    return (
      <div className="App">
        <LoginPage />
      </div>
    )
  }

  if (showMenu) {
    return (
      <div className="App">
        <div className="user-header">
          <span>Welcome, {user.username}!</span>
        </div>
        <MainMenu
          onNewGame={() => {
            setInitialGalaxyJson(null)
            setStartNew(true)
            setShowMenu(false)
          }}
          onLoadGame={() => setShowLoadModal(true)}
        />
        {showLoadModal && (
          <LoadGameModal
            onClose={() => setShowLoadModal(false)}
            onLoaded={galaxyJson => {
              setInitialGalaxyJson(galaxyJson)
              setStartNew(false)
              setShowLoadModal(false)
              setShowMenu(false)
            }}
          />
        )}
      </div>
    )
  }

  return (
    <div className="App">
      <div className="user-header">
        <span>Welcome, {user.username}!</span>
      </div>
      <GalaxyPage
        key={initialGalaxyJson ? 'loaded' : 'new'}
        initialGalaxyJson={initialGalaxyJson}
        startNew={startNew}
      />
    </div>
  )
}

function App() {
  return (
    <UserProvider>
      <AppContent />
    </UserProvider>
  )
}

export default App
