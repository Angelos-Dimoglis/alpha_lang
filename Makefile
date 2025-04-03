# Author: Angelos T. Dimoglis
# AM: csd5078

# compiler related variables
CC = g++
CFLAGS = -g

TARGET = alpha_compiler.out

all: $(TARGET)

lexer.cpp: lexer.l 
	flex -o $@ $^

parser.cpp: parser.ypp
	bison --yacc --defines -tdv $^ -o $@

sym_table.out: sym_table.cpp
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET): lexer.cpp parser.cpp stack.c sym_table.cpp parser_functions.cpp
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm lexer.cpp parser.{cpp,hpp,output} $(TARGET)
