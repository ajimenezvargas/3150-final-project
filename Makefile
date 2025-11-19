CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2
INCLUDES = -I./include -I./src

# Try to find and link libcurl if available (optional)
LDFLAGS = $(shell pkg-config --libs libcurl 2>/dev/null || echo "")

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
DATA_DIR = data

# Source files
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/AS.cpp $(SRC_DIR)/ASGraph.cpp $(SRC_DIR)/Announcement.cpp $(SRC_DIR)/Policy.cpp $(SRC_DIR)/ROV.cpp $(SRC_DIR)/Community.cpp $(SRC_DIR)/Aggregation.cpp $(SRC_DIR)/Statistics.cpp $(SRC_DIR)/Csvoutput.cpp $(SRC_DIR)/CSVInput.cpp
OBJECTS = $(BUILD_DIR)/main.o $(BUILD_DIR)/AS.o $(BUILD_DIR)/ASGraph.o $(BUILD_DIR)/Announcement.o $(BUILD_DIR)/Policy.o $(BUILD_DIR)/ROV.o $(BUILD_DIR)/Community.o $(BUILD_DIR)/Aggregation.o $(BUILD_DIR)/Statistics.o $(BUILD_DIR)/Csvoutput.o $(BUILD_DIR)/CSVInput.o

# Production simulator sources (without test main)
SIM_SOURCES = $(SRC_DIR)/simulator_main.cpp $(SRC_DIR)/AS.cpp $(SRC_DIR)/ASGraph.cpp $(SRC_DIR)/Announcement.cpp $(SRC_DIR)/Policy.cpp $(SRC_DIR)/ROV.cpp $(SRC_DIR)/Community.cpp $(SRC_DIR)/Aggregation.cpp $(SRC_DIR)/Statistics.cpp $(SRC_DIR)/Csvoutput.cpp $(SRC_DIR)/CSVInput.cpp
SIM_OBJECTS = $(BUILD_DIR)/simulator_main.o $(BUILD_DIR)/AS.o $(BUILD_DIR)/ASGraph.o $(BUILD_DIR)/Announcement.o $(BUILD_DIR)/Policy.o $(BUILD_DIR)/ROV.o $(BUILD_DIR)/Community.o $(BUILD_DIR)/Aggregation.o $(BUILD_DIR)/Statistics.o $(BUILD_DIR)/Csvoutput.o $(BUILD_DIR)/CSVInput.o
TARGET = bgp_sim

# Default target
all: $(BUILD_DIR) $(DATA_DIR) $(TARGET) bgp_simulator

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Create data directory
$(DATA_DIR):
	mkdir -p $(DATA_DIR)

# Build test executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful! Run with: ./$(TARGET)"

# Build production simulator
bgp_simulator: $(SIM_OBJECTS)
	$(CXX) $(SIM_OBJECTS) -o bgp_simulator $(LDFLAGS)
	@echo "Simulator build successful! Run with: ./bgp_simulator --help"

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET) bgp_simulator

.PHONY: all clean cleanall run