CXX      := g++                 
CXXFLAGS := -O3 -march=native -flto -funroll-loops -fopenmp -mavx2 -Wall -Wextra -Werror
LDFLAGS  := -lm                 
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)/
OUTCOME_DIR  := ./outcome
TARGET   := app
INCLUDE  := -Iinclude/
SRC      := $(wildcard src/*.cpp) 

OBJECTS := $(SRC:%.cpp=$(OBJ_DIR)/%.o)

all: clean build $(APP_DIR)/$(TARGET) run

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
	-@rm -rvf $(OUTCOME_DIR)/*

run:
	./$(BUILD)/$(TARGET)