#!/bin/bash

# Space 4X Game - Full Stack Status Check
echo "🚀 Space 4X Game - Full Stack Status"
echo "===================================="
echo

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check Frontend (React)
echo "🌐 Frontend (React):"
if curl -s http://localhost:3000 > /dev/null 2>&1; then
    echo -e "   ${GREEN}✅ RUNNING${NC} - http://localhost:3000"
    TITLE=$(curl -s http://localhost:3000 | grep -o '<title>.*</title>')
    echo "   📄 $TITLE"
else
    echo -e "   ${RED}❌ NOT RUNNING${NC}"
fi
echo

# Check Backend (Node.js)
echo "🔧 Backend (Node.js):"
if curl -s http://localhost:3001/health > /dev/null 2>&1; then
    echo -e "   ${GREEN}✅ RUNNING${NC} - http://localhost:3001"
    HEALTH=$(curl -s http://localhost:3001/health | python3 -c "import sys, json; data=json.load(sys.stdin); print(f'Status: {data[\"status\"]}, DB: {data[\"database\"]}')")
    echo "   💚 $HEALTH"
else
    echo -e "   ${RED}❌ NOT RUNNING${NC}"
fi
echo

# Check Game Engine (C++)
echo "🎮 Game Engine (C++):"
ENGINE_COUNT=$(ps aux | grep "space4x-engine --mode service" | grep -v grep | wc -l | tr -d ' ')
if [ "$ENGINE_COUNT" -gt 0 ]; then
    echo -e "   ${GREEN}✅ RUNNING${NC} - $ENGINE_COUNT process(es)"
    echo "   ⚡ Processing game logic..."
else
    echo -e "   ${RED}❌ NOT RUNNING${NC}"
fi
echo

# Check Database
echo "💾 Database (PostgreSQL):"
if psql -U space4x_user -d space4x_game -c "SELECT version();" > /dev/null 2>&1; then
    echo -e "   ${GREEN}✅ CONNECTED${NC}"
    VERSION=$(psql -U space4x_user -d space4x_game -t -c "SELECT version();" 2>/dev/null | head -1 | xargs)
    echo "   🗄️  PostgreSQL connected"
else
    echo -e "   ${RED}❌ NOT CONNECTED${NC}"
fi
echo

# Summary
TOTAL_SERVICES=4
RUNNING_SERVICES=0

curl -s http://localhost:3000 > /dev/null 2>&1 && ((RUNNING_SERVICES++))
curl -s http://localhost:3001/health > /dev/null 2>&1 && ((RUNNING_SERVICES++))
[ "$ENGINE_COUNT" -gt 0 ] && ((RUNNING_SERVICES++))
psql -U space4x_user -d space4x_game -c "SELECT 1;" > /dev/null 2>&1 && ((RUNNING_SERVICES++))

echo "📊 Summary: $RUNNING_SERVICES/$TOTAL_SERVICES services running"

if [ "$RUNNING_SERVICES" -eq "$TOTAL_SERVICES" ]; then
    echo -e "${GREEN}🎉 Full stack is operational!${NC}"
elif [ "$RUNNING_SERVICES" -gt 0 ]; then
    echo -e "${YELLOW}⚠️  Partial stack running${NC}"
else
    echo -e "${RED}❌ No services running${NC}"
fi
