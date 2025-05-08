# Author: Angelos T. Dimoglis
# AM: csd5078

# compiler related variables
CC = g++
CFLAGS = -g

TARGET = alpha_compiler.out

all: $(TARGET)

lexer.cpp: lexer.l 
	flex -o $@ $^

parser.cpp: parser.y
	bison --yacc --defines -tdv $^ -o $@

$(TARGET): lexer.cpp parser.cpp sym_table.cpp parser_functions.cpp icode_gen.cpp
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm lexer.cpp parser.cpp parser.hpp parser.output $(TARGET)
