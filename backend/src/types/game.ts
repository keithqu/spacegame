// Game-related type definitions

export interface Player {
  id: string;
  username: string;
  email: string;
  created_at: Date;
  updated_at: Date;
}

export interface Game {
  id: string;
  name: string;
  creator_id: string;
  status: 'waiting' | 'active' | 'paused' | 'finished';
  max_players: number;
  current_turn: number;
  created_at: Date;
  updated_at: Date;
}

export interface GamePlayer {
  game_id: string;
  player_id: string;
  faction: string;
  status: 'active' | 'eliminated' | 'ai';
  joined_at: Date;
}

export interface Planet {
  id: string;
  game_id: string;
  name: string;
  owner_id?: string;
  x: number;
  y: number;
  z: number;
  size: number;
  population: number;
  resources: {
    minerals: number;
    energy: number;
    research: number;
  };
  created_at: Date;
  updated_at: Date;
}

export interface Fleet {
  id: string;
  game_id: string;
  owner_id: string;
  name: string;
  x: number;
  y: number;
  z: number;
  destination_x?: number;
  destination_y?: number;
  destination_z?: number;
  ships: {
    fighters: number;
    cruisers: number;
    battleships: number;
  };
  created_at: Date;
  updated_at: Date;
}

export interface GameAction {
  id: string;
  game_id: string;
  player_id: string;
  turn: number;
  action_type: 'move_fleet' | 'build_ship' | 'colonize_planet' | 'research_tech';
  action_data: Record<string, any>;
  processed: boolean;
  created_at: Date;
}
