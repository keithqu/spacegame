import pool from '../config/database'
import { Player } from '../types/game'

export class PlayerModel {
  static async create(username: string, email: string): Promise<Player> {
    const query = `
      INSERT INTO players (username, email, created_at, updated_at)
      VALUES ($1, $2, NOW(), NOW())
      RETURNING *
    `

    const result = await pool.query(query, [username, email])
    return result.rows[0]
  }

  static async findById(id: string): Promise<Player | null> {
    const query = 'SELECT * FROM players WHERE id = $1'
    const result = await pool.query(query, [id])
    return result.rows[0] || null
  }

  static async findByEmail(email: string): Promise<Player | null> {
    const query = 'SELECT * FROM players WHERE email = $1'
    const result = await pool.query(query, [email])
    return result.rows[0] || null
  }

  static async findByUsername(username: string): Promise<Player | null> {
    const query = 'SELECT * FROM players WHERE username = $1'
    const result = await pool.query(query, [username])
    return result.rows[0] || null
  }

  static async update(
    id: string,
    updates: Partial<Pick<Player, 'username' | 'email'>>
  ): Promise<Player | null> {
    const setClause = Object.keys(updates)
      .map((key, index) => `${key} = $${index + 2}`)
      .join(', ')
    const query = `
      UPDATE players 
      SET ${setClause}, updated_at = NOW()
      WHERE id = $1
      RETURNING *
    `

    const values = [id, ...Object.values(updates)]
    const result = await pool.query(query, values)
    return result.rows[0] || null
  }

  static async delete(id: string): Promise<boolean> {
    const query = 'DELETE FROM players WHERE id = $1'
    const result = await pool.query(query, [id])
    return (result.rowCount || 0) > 0
  }

  static async list(limit: number = 50, offset: number = 0): Promise<Player[]> {
    const query = 'SELECT * FROM players ORDER BY created_at DESC LIMIT $1 OFFSET $2'
    const result = await pool.query(query, [limit, offset])
    return result.rows
  }
}
