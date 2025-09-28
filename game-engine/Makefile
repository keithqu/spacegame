CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -I/opt/homebrew/include
LDFLAGS = 

SRC_DIR = src
BUILD_DIR = build
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/galaxy.cpp $(SRC_DIR)/http_server.cpp
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TARGET = $(BUILD_DIR)/space4x-engine

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
	@echo "Built game engine: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	@echo "Cleaned build directory"

run: dev
	./$(TARGET) --mode service
