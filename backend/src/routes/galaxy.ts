import { Router } from 'express';
import axios from 'axios';

const router = Router();

// Configuration for C++ engine
const ENGINE_URL = process.env.GAME_ENGINE_URL || 'http://localhost:3002';

// POST /api/galaxy/generate - Generate a new galaxy
router.post('/generate', async (req, res) => {
  try {
    console.log('🌌 Received galaxy generation request:', req.body);
    
    // Forward request to C++ engine
    const response = await axios.post(`${ENGINE_URL}/generate-galaxy`, req.body, {
      headers: {
        'Content-Type': 'application/json'
      },
      timeout: 30000 // 30 second timeout
    });
    
    console.log('✅ Galaxy generated successfully by C++ engine');
    res.json(response.data);
    
  } catch (error) {
    console.error('❌ Galaxy generation failed:', error);
    
    if (axios.isAxiosError(error)) {
      if (error.code === 'ECONNREFUSED') {
        res.status(503).json({
          error: 'Game engine unavailable',
          message: 'The C++ game engine is not running or not accessible'
        });
      } else if (error.response) {
        res.status(error.response.status).json(error.response.data);
      } else {
        res.status(500).json({
          error: 'Network error',
          message: 'Failed to communicate with game engine'
        });
      }
    } else {
      res.status(500).json({
        error: 'Internal server error',
        message: error instanceof Error ? error.message : 'Unknown error'
      });
    }
  }
});

// GET /api/galaxy/health - Check if galaxy generation service is available
router.get('/health', async (req, res) => {
  try {
    const response = await axios.get(`${ENGINE_URL}/health`, {
      timeout: 5000 // 5 second timeout
    });
    
    res.json({
      status: 'healthy',
      engine: response.data,
      proxy: 'operational'
    });
    
  } catch (error) {
    console.error('❌ Engine health check failed:', error);
    
    res.status(503).json({
      status: 'unhealthy',
      engine: 'unavailable',
      proxy: 'operational',
      error: axios.isAxiosError(error) ? error.message : 'Unknown error'
    });
  }
});

export default router;
