# Space 4X Game

A modern space strategy game with hybrid JavaScript/C++ architecture featuring real-time galaxy generation and interactive visualization.

## 🌌 Features

- **Dynamic Galaxy Generation**: Procedurally generated galaxies with customizable parameters
- **Interactive Visualization**: Beautiful D3.js-powered galaxy maps with zoom, pan, and tooltips
- **Hybrid Architecture**: High-performance C++ game engine with modern web frontend
- **Real-time Communication**: Frontend ↔ Backend ↔ C++ Engine pipeline
- **Configurable Systems**: Customizable star systems, anomalies, and warp lane networks

## 🏗️ Architecture

```
React Frontend (port 3000)     Node.js Backend (port 3001)     C++ Game Engine (port 3002)
     │                               │                               │
     │ ──── Galaxy Generation ────→  │ ──── HTTP Proxy ────→        │
     │                               │                               │ Galaxy
     │                               │                               │ Algorithm
     │ ←──── JSON Response ────────  │ ←──── JSON Data ────────     │
     │                               │                               │
     └── D3.js Visualization         └── Express API                 └── HTTP Server
```

## 🚀 Quick Start

### Prerequisites

- **Node.js** 18+ and **pnpm** 8+
- **C++ compiler** with C++17 support (GCC/Clang)
- **Make** build system
- **PostgreSQL** (for backend database)

### Installation

1. **Clone the repository**

   ```bash
   git clone <repository-url>
   cd spacegame
   ```

2. **Install dependencies**

   ```bash
   pnpm install
   ```

3. **Set up the database**

   ```bash
   cd scripts && ./setup-database.sh
   ```

4. **Build the C++ engine**
   ```bash
   cd game-engine && make dev
   ```

### Running the Application

**Option 1: Run all services together**

```bash
pnpm run dev
```

**Option 2: Run services individually**

```bash
# Terminal 1: C++ Game Engine
cd game-engine && ./build/space4x-engine --mode service

# Terminal 2: Backend API
cd backend && pnpm run dev

# Terminal 3: Frontend
cd frontend && pnpm start
```

### Access Points

- **Frontend**: http://localhost:3000
- **Backend API**: http://localhost:3001
- **C++ Engine**: http://localhost:3002
- **Health Checks**:
  - Backend: http://localhost:3001/health
  - Engine: http://localhost:3002/health
  - Galaxy Service: http://localhost:3001/api/galaxy/health

## 📁 Project Structure

```
spacegame/
├── frontend/                 # React TypeScript frontend
│   ├── src/
│   │   ├── components/      # React components
│   │   ├── services/        # API services
│   │   ├── types/           # TypeScript definitions
│   │   └── utils/           # Utility functions
│   └── package.json
├── backend/                  # Node.js Express backend
│   ├── src/
│   │   ├── config/          # Database configuration
│   │   ├── models/          # Data models
│   │   ├── routes/          # API routes
│   │   └── scripts/         # Database migrations
│   └── package.json
├── game-engine/             # C++ game engine
│   ├── src/                 # C++ source files
│   ├── include/             # Header files
│   ├── Makefile             # Build configuration
│   └── build/               # Compiled binaries
└── scripts/                 # Setup and utility scripts
```

## 🎮 Galaxy Generation

### Configuration Options

```json
{
  "seed": 42,
  "radius": 500,
  "starSystemCount": 350,
  "anomalyCount": 50,
  "fixedSystems": [
    {
      "id": "sol",
      "name": "Sol System",
      "x": 0,
      "y": 0,
      "type": "origin"
    }
  ],
  "connectivity": {
    "minConnections": 1,
    "maxConnections": 3,
    "maxDistance": 8,
    "distanceDecayFactor": 0.5
  }
}
```

### Generation Rules

- **Minimum Distance**: 2 light years between any objects
- **Maximum Warp Distance**: 8 light years for standard connections
- **Emergency Connections**: Up to 15 light years for isolated systems
- **Fixed Systems**: Sol, Alpha Centauri, and Tau Ceti always present
- **Network Connectivity**: All star systems connected, anomalies isolated

## 🔧 Development

### Building Components

```bash
# Build C++ engine
cd game-engine && make dev        # Development build
cd game-engine && make release    # Production build

# Build backend
cd backend && pnpm run build

# Build frontend
cd frontend && pnpm run build
```

### Testing

```bash
# Test individual services
curl http://localhost:3002/health                    # C++ engine
curl http://localhost:3001/health                    # Backend
curl http://localhost:3001/api/galaxy/health         # Galaxy service

# Test galaxy generation
curl -X POST http://localhost:3001/api/galaxy/generate \
  -H "Content-Type: application/json" \
  -d '{"seed":123,"radius":200,"starSystemCount":50}'
```

### Database Management

```bash
# Run migrations
cd backend && pnpm run migrate

# Reset database
cd scripts && ./setup-database.sh
```

## 🛠️ Technology Stack

### Frontend

- **React** 18 with TypeScript
- **D3.js** for data visualization
- **Create React App** for development setup

### Backend

- **Node.js** with Express
- **TypeScript** for type safety
- **PostgreSQL** with pg driver
- **Axios** for HTTP client

### Game Engine

- **C++17** with STL
- **Custom HTTP server** (lightweight)
- **JSON serialization** (manual implementation)
- **Cross-platform** socket programming

### Development Tools

- **pnpm** for package management
- **pnpm workspaces** for monorepo structure
- **Concurrently** for running multiple services
- **Nodemon** for backend hot reload
- **Make** for C++ build system

## 📊 Performance

- **Galaxy Generation**: ~50ms for 350 systems (C++)
- **Network Latency**: <10ms local pipeline
- **Memory Usage**: ~50MB C++ engine, ~100MB Node.js backend
- **Visualization**: 60fps D3.js rendering with zoom/pan

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🎯 Roadmap

- [ ] **Game Logic**: Turn-based gameplay mechanics
- [ ] **Multiplayer**: Real-time multiplayer support
- [ ] **AI Players**: Computer-controlled factions
- [ ] **Ship Combat**: Tactical battle system
- [ ] **Research Trees**: Technology progression
- [ ] **Diplomacy**: Inter-faction relations
- [ ] **Resource Management**: Economic systems
- [ ] **Save/Load**: Game state persistence

## 🐛 Known Issues

- C++ engine runs in demo mode (100 ticks limit)
- Database migrations need manual execution
- Frontend error handling could be improved

## 📞 Support

For questions, issues, or contributions, please open an issue on GitHub or contact the development team.

---

**Built with ❤️ for space strategy enthusiasts**
