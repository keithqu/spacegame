import fs from 'fs'
import path from 'path'
import dotenv from 'dotenv'
import pool from '../config/database'

dotenv.config()

async function runMigrations() {
  try {
    console.log('🔄 Starting database migrations...')

    // Create migrations table if it doesn't exist
    await pool.query(`
      CREATE TABLE IF NOT EXISTS migrations (
        id SERIAL PRIMARY KEY,
        filename VARCHAR(255) NOT NULL UNIQUE,
        executed_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
      )
    `)

    // Get list of executed migrations
    const executedMigrations = await pool.query('SELECT filename FROM migrations ORDER BY id')
    const executedFiles = executedMigrations.rows.map(row => row.filename)

    // Read migration files
    const migrationsDir = path.join(__dirname, '../migrations')
    const migrationFiles = fs
      .readdirSync(migrationsDir)
      .filter(file => file.endsWith('.sql'))
      .sort()

    console.log(`📁 Found ${migrationFiles.length} migration files`)

    let executedCount = 0

    for (const filename of migrationFiles) {
      if (executedFiles.includes(filename)) {
        console.log(`⏭️  Skipping ${filename} (already executed)`)
        continue
      }

      console.log(`🔄 Executing migration: ${filename}`)

      const filePath = path.join(migrationsDir, filename)
      const sql = fs.readFileSync(filePath, 'utf8')

      // Begin transaction
      const client = await pool.connect()
      try {
        await client.query('BEGIN')

        // Execute migration
        await client.query(sql)

        // Record migration as executed
        await client.query('INSERT INTO migrations (filename) VALUES ($1)', [filename])

        await client.query('COMMIT')
        console.log(`✅ Successfully executed: ${filename}`)
        executedCount++
      } catch (error) {
        await client.query('ROLLBACK')
        throw error
      } finally {
        client.release()
      }
    }

    if (executedCount === 0) {
      console.log('✅ All migrations are up to date')
    } else {
      console.log(`✅ Successfully executed ${executedCount} migrations`)
    }
  } catch (error) {
    console.error('❌ Migration failed:', error)
    process.exit(1)
  } finally {
    await pool.end()
  }
}

// Run migrations if this script is called directly
if (require.main === module) {
  runMigrations()
}

export { runMigrations }
