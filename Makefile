DIRECTORIES := src
TARGET := ROMDemangler
BUILD_DIR := build
CXX := g++
ASAN := 0
CXXFLAGS := -Wall -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare -Wextra -O3 -Iinclude -std=c++23 -D_LANGUAGE_C_PLUS_PLUS
LDFLAGS :=

ifeq ($(ASAN),1)
	CXXFLAGS += -g -fsanitize=address -fsanitize=undefined
	LDFLAGS += -fsanitize=address -fsanitize=undefined
endif

ifeq ($(OS),Windows_NT)
    LDFLAGS += -static
endif

SOURCES := $(wildcard $(addsuffix /*.cpp,$(DIRECTORIES)))

OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET) copyFiles

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

copyFiles:
	cp sm64.us.map $(BUILD_DIR)/sm64.us.map
	cp ROMDemanglerGUI.py $(BUILD_DIR)/ROMDemanglerGUI.py
	cp -r gui/ $(BUILD_DIR)/

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean copyFiles