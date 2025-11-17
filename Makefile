CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2
INCLUDES = -I./include -I./src
LDFLAGS = -lcurl

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
DATA_DIR = data

# Source files
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/AS.cpp $(SRC_DIR)/ASGraph.cpp $(SRC_DIR)/Announcement.cpp $(SRC_DIR)/Policy.cpp
OBJECTS = $(BUILD_DIR)/main.o $(BUILD_DIR)/AS.o $(BUILD_DIR)/ASGraph.o $(BUILD_DIR)/Announcement.o $(BUILD_DIR)/Policy.o
TARGET = bgp_sim

# Default target
all: $(BUILD_DIR) $(DATA_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Create data directory
$(DATA_DIR):
	mkdir -p $(DATA_DIR)

# Build executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful! Run with: ./$(TARGET)"

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Clean everything including downloaded data
cleanall: clean
	rm -rf $(DATA_DIR)

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean cleanall run