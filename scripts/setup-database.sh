#!/bin/bash

# Space 4X Game - Database Setup Script
echo "🚀 Setting up PostgreSQL database for Space 4X Game"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Database configuration
DB_NAME="space4x_game"
DB_USER="space4x_user"
DB_PASSWORD="space4x_dev_password"

# Check if PostgreSQL is installed
if ! command -v psql &> /dev/null && ! command -v /opt/homebrew/bin/psql &> /dev/null; then
    echo -e "${RED}❌ PostgreSQL is not installed.${NC}"
    echo "Please install PostgreSQL first:"
    echo "  macOS: brew install postgresql"
    echo "  Ubuntu: sudo apt-get install postgresql postgresql-contrib"
    exit 1
fi

# Set PostgreSQL path if using Homebrew
if [ -f /opt/homebrew/bin/psql ]; then
    export PATH="/opt/homebrew/bin:$PATH"
fi

# Check if PostgreSQL service is running
if ! pgrep -x "postgres" > /dev/null; then
    echo -e "${YELLOW}⚠️  PostgreSQL service is not running. Starting it...${NC}"
    if command -v brew &> /dev/null; then
        brew services start postgresql
    else
        sudo systemctl start postgresql
    fi
    sleep 2
fi

echo "🔄 Creating database and user..."

# Create database and user (use current user on macOS, postgres on Linux)
if [[ "$OSTYPE" == "darwin"* ]]; then
    POSTGRES_USER=$(whoami)
else
    POSTGRES_USER="postgres"
fi

psql -U $POSTGRES_USER -d postgres -c "CREATE USER $DB_USER WITH PASSWORD '$DB_PASSWORD';" 2>/dev/null || echo "User might already exist"
psql -U $POSTGRES_USER -d postgres -c "CREATE DATABASE $DB_NAME OWNER $DB_USER;" 2>/dev/null || echo "Database might already exist"
psql -U $POSTGRES_USER -d postgres -c "GRANT ALL PRIVILEGES ON DATABASE $DB_NAME TO $DB_USER;" 2>/dev/null

# Create .env file for backend
ENV_FILE="../backend/.env"
if [ ! -f "$ENV_FILE" ]; then
    echo "📝 Creating .env file..."
    cat > "$ENV_FILE" << EOF
# Database Configuration
DATABASE_URL=postgresql://$DB_USER:$DB_PASSWORD@localhost:5432/$DB_NAME
DB_HOST=localhost
DB_PORT=5432
DB_NAME=$DB_NAME
DB_USER=$DB_USER
DB_PASSWORD=$DB_PASSWORD

# Server Configuration
PORT=3001
NODE_ENV=development

# Game Engine Configuration
GAME_ENGINE_PORT=3002
EOF
    echo -e "${GREEN}✅ Created .env file${NC}"
else
    echo -e "${YELLOW}⚠️  .env file already exists, skipping creation${NC}"
fi

# Test connection
echo "🔍 Testing database connection..."
if psql -U $DB_USER -d $DB_NAME -c "SELECT version();" &> /dev/null; then
    echo -e "${GREEN}✅ Database connection successful!${NC}"
    
    # Run migrations
    echo "🔄 Running database migrations..."
    cd ../backend && pnpm run migrate
    
    echo -e "${GREEN}🎉 Database setup complete!${NC}"
    echo ""
    echo "Next steps:"
    echo "1. Start the backend server: cd backend && pnpm run dev"
    echo "2. The backend will be available at http://localhost:3001"
    echo "3. Check health: curl http://localhost:3001/health"
else
    echo -e "${RED}❌ Database connection failed${NC}"
    echo "Please check your PostgreSQL installation and try again"
    exit 1
fi
