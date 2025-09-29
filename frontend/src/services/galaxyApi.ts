import { Galaxy, GalaxyConfig, DEFAULT_GALAXY_CONFIG } from '../types/galaxy';

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
      
      // Transform the response to match our frontend types and ensure all required properties exist
      const mergedConfig = {
        ...DEFAULT_GALAXY_CONFIG,
        ...galaxyData.config,
        visualization: {
          ...DEFAULT_GALAXY_CONFIG.visualization,
          ...(galaxyData.config?.visualization || {})
        }
      };
      
      return {
        config: mergedConfig,
        systems: galaxyData.systems || [],
        anomalies: galaxyData.anomalies || [],
        warpLanes: galaxyData.warpLanes || [],
        bounds: galaxyData.bounds || {
          minX: -mergedConfig.radius,
          maxX: mergedConfig.radius,
          minY: -mergedConfig.radius,
          maxY: mergedConfig.radius,
          radius: mergedConfig.radius
        }
      };
      
    } catch (error) {
      console.error('❌ Galaxy generation failed:', error);
      
      // If backend is unavailable, provide a fallback response
      if (error instanceof Error && (error.message.includes('fetch') || error.message.includes('ECONNREFUSED'))) {
        console.warn('⚠️ Backend unavailable, using fallback galaxy data');
        
        const fallbackConfig = { ...DEFAULT_GALAXY_CONFIG, ...config };
        
        return {
          config: fallbackConfig,
          systems: [
            {
              id: 'sol',
              name: 'Sol System',
              x: 0,
              y: 0,
              type: 'origin',
              isFixed: true,
              explored: true,
              population: 1000000,
              connections: ['alpha-centauri'],
              resources: { minerals: 100, energy: 100, research: 100 }
            },
            {
              id: 'alpha-centauri',
              name: 'Alpha Centauri',
              x: 4.37,
              y: 0,
              type: 'major',
              isFixed: true,
              explored: false,
              population: 0,
              connections: ['sol'],
              resources: { minerals: 150, energy: 80, research: 120 }
            }
          ],
          anomalies: [
            {
              id: 'nebula-1',
              name: 'Crimson Nebula',
              x: 10,
              y: 15,
              type: 'nebula',
              discovered: false,
              effect: { type: 'sensor_interference', value: -0.5 }
            }
          ],
          warpLanes: [
            {
              id: 'sol-alpha-centauri',
              from: 'sol',
              to: 'alpha-centauri',
              distance: 4.37,
              travelTime: 1,
              discovered: true
            }
          ],
          bounds: {
            minX: -fallbackConfig.radius,
            maxX: fallbackConfig.radius,
            minY: -fallbackConfig.radius,
            maxY: fallbackConfig.radius,
            radius: fallbackConfig.radius
          }
        };
      }
      
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
