import { Galaxy, GalaxyConfig } from '../types/galaxy';

const API_BASE_URL = process.env.REACT_APP_API_URL || 'http://localhost:3001';

export class GalaxyApiService {
  static async generateGalaxy(config: Partial<GalaxyConfig>): Promise<Galaxy> {
    try {
      console.log('🌌 Requesting galaxy generation from backend...', config);
      
      const response = await fetch(`${API_BASE_URL}/api/galaxy/generate`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(config),
      });

      if (!response.ok) {
        const errorData = await response.json().catch(() => ({}));
        throw new Error(
          errorData.message || 
          errorData.error || 
          `HTTP ${response.status}: ${response.statusText}`
        );
      }

      const galaxyData = await response.json();
      console.log('✅ Galaxy generated successfully by backend');
      
      // Transform the response to match our frontend types
      return {
        config: galaxyData.config,
        systems: galaxyData.systems,
        anomalies: galaxyData.anomalies,
        warpLanes: galaxyData.warpLanes,
        bounds: galaxyData.bounds
      };
      
    } catch (error) {
      console.error('❌ Galaxy generation failed:', error);
      throw error;
    }
  }

  static async checkHealth(): Promise<{ status: string; engine: any; proxy: string }> {
    try {
      const response = await fetch(`${API_BASE_URL}/api/galaxy/health`);
      
      if (!response.ok) {
        throw new Error(`Health check failed: ${response.status}`);
      }
      
      return await response.json();
    } catch (error) {
      console.error('❌ Galaxy service health check failed:', error);
      throw error;
    }
  }
}
