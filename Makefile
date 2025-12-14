CXX ?= clang++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -O2 -Iinclude
LDFLAGS :=
BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)/bin
OBJ_DIR := $(BUILD_DIR)/obj

SRC_SRCS := $(shell find src -name '*.cpp')
SRC_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC_SRCS))
CLI_SRCS := $(shell find cli -name '*.cpp')
CLI_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(CLI_SRCS))
TEST_SRCS := $(shell find tests -name '*.cpp')
TEST_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(TEST_SRCS))
SIM_ARGS ?= --pattern ACGT --input datasets/dna/sample.txt

TARGET := $(BIN_DIR)/automata_sim
TEST_TARGET := $(BIN_DIR)/automata_tests

.PHONY: all clean run test

all: $(TARGET)

run: $(TARGET)
	$(TARGET) $(SIM_ARGS)

$(TARGET): $(SRC_OBJS) $(CLI_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_TARGET): $(SRC_OBJS) $(TEST_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR)
