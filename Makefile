# Names of the programs we want to compile
TARGETS =\
	compare\
	spanning_forest\
	generate\
	infos\
	mpitest\
	graph-to-bin\
	bin-to-graph\

# C++ compiler
CXX = mpic++

# Compiler flags
CFLAGS = -std=c++1y

# Debuger flags
DFLAGS =

# Warning flags
WFLAGS = -Wall -Wextra

# Source files
SRC_DIR = src
SRC = $(shell find $(SRC_DIR) -type f -name '*.cpp' -not -path "$(SRC_DIR)/main/*")

# Object files
BUILD_DIR = build
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Main targets
.PHONY: all release debug clean clean-all

all: release

debug: DFLAGS += -ggdb
debug: $(TARGETS)

release: CFLAGS += -O3 -DNDEBUG
release: $(TARGETS)

clean:
	rm -r $(BUILD_DIR)

clean-all:
	rm $(TARGETS)
	rm -r $(BUILD_DIR)
	rm inputs/random_*
	rm inputs/*.bin
	rm inputs/*.out

# Compile different programs
$(TARGETS): %: $(OBJ) $(BUILD_DIR)/main/%.o
	$(CXX) -o $@ $^ $(DFLAGS) $(LFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -o $@ -c $< $(CFLAGS) $(DFLAGS) $(WFLAGS)
