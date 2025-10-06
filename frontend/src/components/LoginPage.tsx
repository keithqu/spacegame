import React from 'react'
import { useUser } from '../contexts/UserContext'

export const LoginPage: React.FC = () => {
  const { login, loading, error } = useUser()

  const handleLogin = async () => {
    await login()
  }

  return (
    <div className="login-container">
      <div className="login-card">
        <h1>Space 4X Game</h1>
        <p>Welcome to the Space 4X strategy game!</p>

        {error && <div className="error-message">{error}</div>}

        <button onClick={handleLogin} disabled={loading} className="login-button">
          {loading ? 'Authenticating...' : 'Login as Keith'}
        </button>

        <p className="login-note">Currently using default user "keith" for development</p>
      </div>
    </div>
  )
}
