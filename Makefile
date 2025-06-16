CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2 -pthread
CPPFLAGS = -Iinclude/MCP3008/include -Iinclude/lg/include
LDFLAGS = -lpigpio -llgpio -lrt -Linclude/lg/lib -llg

SRC_DIR = src
BUILD_DIR = build

SRC_FILES = $(SRC_DIR)/main.cpp $(SRC_DIR)/game.cpp $(SRC_DIR)/servo.cpp
EXT_SRC = include/MCP3008/src/MCP3008.cpp

OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o) $(EXT_SRC:%.cpp=$(BUILD_DIR)/%.o)

EXEC = main

all: $(EXEC)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/include/MCP3008/src/%.o: include/MCP3008/src/%.cpp | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(EXEC): $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $(EXEC) $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR) $(EXEC)

.PHONY: all clean
