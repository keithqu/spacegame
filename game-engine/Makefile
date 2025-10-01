CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -I/opt/homebrew/include -I/opt/homebrew/Cellar/libpq/17.5/include
LDFLAGS = -L/opt/homebrew/Cellar/libpq/17.5/lib -lpq

SRC_DIR = src
BUILD_DIR = build
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/galaxy.cpp $(SRC_DIR)/http_server.cpp $(SRC_DIR)/celestial_bodies.cpp $(SRC_DIR)/backend_server.cpp
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TARGET = $(BUILD_DIR)/space4x-backend

.PHONY: all debug release dev clean

all: release

debug: CXXFLAGS += -g -DDEBUG -O0
debug: $(TARGET)

release: CXXFLAGS += -O3 -DNDEBUG
release: $(TARGET)

dev: CXXFLAGS += -g -O2
dev: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Built backend server: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	@echo "Cleaned build directory"

run: dev
	./$(TARGET) --mode service
