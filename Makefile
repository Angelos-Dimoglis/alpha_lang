# Author: Angelos T. Dimoglis
# AM: csd5078

# compiler related variables
CC = g++
CFLAGS = -g -Ilib

# target executable
TARGET = alpha_compiler.out

# directory variables
BIN_DIR = bin
GEN_SRCS = src/lexer.cpp src/parser.cpp
SRCS = $(filter-out $(GEN_SRCS), $(wildcard src/*.cpp)) $(GEN_SRCS)
OBJS = $(patsubst src/%.cpp, $(BIN_DIR)/%.o, $(SRCS))

# default target
all: $(TARGET)

# linking step
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# compile all .cpp files
$(BIN_DIR)/%.o: src/%.cpp | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

src/lexer.cpp: lexer.l lib/parser.hpp
	flex -o $@ $<

src/parser.cpp lib/parser.hpp: parser.y
	bison --yacc --defines=lib/parser.hpp -Wcounterexamples -tdv $^ -o src/parser.cpp

# create the bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(TARGET) $(BIN_DIR) src/lexer.cpp src/parser.cpp lib/parser.hpp src/parser.output

remake:
	make clean && clear && make

.PHONY: all clean
