CXX      := g++                 
CXXFLAGS := -Wall -Wextra -Werror -O3
LDFLAGS  := -lm                 
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)/
OUTCOME_DIR  := ./outcome
TARGET   := app
INCLUDE  := -Iinclude/
SRC      := $(wildcard src/*.cpp) 

OBJECTS := $(SRC:%.cpp=$(OBJ_DIR)/%.o)

all: clean build $(APP_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: %.cpp         
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<

$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LDFLAGS) -o $(APP_DIR)/$(TARGET) $(OBJECTS)

.PHONY: all build clean debug release run

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OUTCOME_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O3
release: all

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*

run:
	./$(BUILD)/$(TARGET)