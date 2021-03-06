# Names of the programs we want to compile
TARGETS =\
	compare\
	generate\
	bin-to-graph\
	graph-to-bin\
	mtx-to-bin\
	mpitest\
	shared\
	rem

# C compiler
CXX = mpicc

# Compiler flags
CFLAGS = --std=c11 -fopenmp

# Debuger flags
DFLAGS =

# Library linkage
LFLAGS = -fopenmp

# Warning flags
WFLAGS = -Wall -Wextra -Winline

# Source files
SRC_DIR = src
SRC = $(shell find $(SRC_DIR) -type f -name '*.c' -not -path "$(SRC_DIR)/main/*")

# Object files
BUILD_DIR = build
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Main targets
.PHONY: all release debug timers clean clean-all

all: debug

debug: DFLAGS += -ggdb
debug: CFLAGS += -DTIMERS
debug: $(TARGETS)

release: CFLAGS += -O3 -DNDEBUG
release: $(TARGETS)

timers: CFLAGS += -DTIMERS
timers: release

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

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CXX) -o $@ -c $< $(CFLAGS) $(DFLAGS) $(WFLAGS)
