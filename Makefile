CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2
LDFLAGS =

# Directories
SRC_DIR = src
BUILD_DIR = build
DATA_DIR = data

# Source files
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/AS.cpp
OBJECTS = $(BUILD_DIR)/main.o $(BUILD_DIR)/AS.o
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
	$(CXX) $(CXXFLAGS) -c $< -o $@

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