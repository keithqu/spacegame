export type User = {
  id: string
  username: string
  email: string
  membership: string | null
}

// For now, always return the seeded user. Later, replace with real API/auth.
export async function getCurrentUser(): Promise<User> {
  return {
    id: '00000000-0000-0000-0000-000000000000',
    username: 'keith',
    email: 'kqu123@gmail.com',
    membership: null,
  }
}
