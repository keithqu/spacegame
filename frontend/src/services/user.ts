export type User = {
  id: string
  username: string
  email: string
  membership: string | null
}

const API_BASE_URL = process.env.REACT_APP_API_URL || 'http://localhost:3001'

// Get the current user from the backend API
export async function getCurrentUser(): Promise<User> {
  try {
    const response = await fetch(`${API_BASE_URL}/api/user/current`, {
      method: 'GET',
      headers: {
        'Content-Type': 'application/json',
      },
    })

    if (!response.ok) {
      throw new Error(`Failed to get current user: ${response.statusText}`)
    }

    const user = await response.json()
    return user
  } catch (error) {
    console.error('Error getting current user:', error)
    // Fallback to default user for development
    return {
      id: '00000000-0000-0000-0000-000000000000',
      username: 'keith',
      email: 'kqu123@gmail.com',
      membership: null,
    }
  }
}
