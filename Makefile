# Author: Angelos T. Dimoglis
# AM: csd5078

# compiler related variables
CC = g++
CFLAGS = -g

TARGET = alpha_compiler.out

SRCS = sym_table.cpp parser_functions.cpp icode_gen.cpp parser.cpp lexer.cpp
OBJS = $(SRCS:.cpp=.o)

GEN_SRCS = lexer.cpp parser.cpp
GEN_HDRS = parser.hpp

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

parser.cpp: parser.y
	bison --yacc --defines -tdv $^ -o parser.cpp

lexer.cpp: lexer.l
	flex -o $@ $^

parser.o: parser.cpp
lexer.o: lexer.cpp
sym_table.o: sym_table.cpp
parser_functions.o: parser_functions.cpp parser_functions.h
icode_gen.o: icode_gen.cpp

clean:
	rm -f $(TARGET) $(OBJS) $(GEN_SRCS) $(GEN_HDRS) parser.output

.PHONY: all clean