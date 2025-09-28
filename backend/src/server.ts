import express from 'express';
import cors from 'cors';
import dotenv from 'dotenv';
import pool from './config/database';
import galaxyRoutes from './routes/galaxy';

// Load environment variables
dotenv.config();

const app = express();
const PORT = process.env.PORT || 3001;

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Health check endpoint
app.get('/health', async (req, res) => {
  try {
    // Test database connection
    const client = await pool.connect();
    const result = await client.query('SELECT NOW()');
    client.release();
    
    res.json({
      status: 'healthy',
      timestamp: result.rows[0].now,
      database: 'connected'
    });
  } catch (error) {
    console.error('Health check failed:', error);
    res.status(500).json({
      status: 'unhealthy',
      database: 'disconnected',
      error: error instanceof Error ? error.message : 'Unknown error'
    });
  }
});

// API Routes
app.use('/api/galaxy', galaxyRoutes);

app.get('/api/test', (req, res) => {
  res.json({ message: 'Space 4X Backend API is running!' });
});

// Game state endpoints (placeholder)
app.get('/api/game/state', async (req, res) => {
  // TODO: Implement game state retrieval
  res.json({ 
    message: 'Game state endpoint - to be implemented',
    gameId: 'placeholder'
  });
});

app.post('/api/game/action', async (req, res) => {
  // TODO: Implement game action processing
  res.json({ 
    message: 'Game action endpoint - to be implemented',
    action: req.body
  });
});

// Error handling middleware
app.use((error: Error, req: express.Request, res: express.Response, next: express.NextFunction) => {
  console.error('Server error:', error);
  res.status(500).json({
    error: 'Internal server error',
    message: process.env.NODE_ENV === 'development' ? error.message : 'Something went wrong'
  });
});

// 404 handler
app.use((req, res) => {
  res.status(404).json({ error: 'Route not found' });
});

// Start server
app.listen(PORT, () => {
  console.log(`🚀 Space 4X Backend server running on port ${PORT}`);
  console.log(`📊 Health check available at http://localhost:${PORT}/health`);
});

// Graceful shutdown
process.on('SIGINT', async () => {
  console.log('🛑 Shutting down server...');
  await pool.end();
  process.exit(0);
});

export default app;
