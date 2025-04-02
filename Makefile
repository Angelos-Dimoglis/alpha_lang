# Author: Angelos T. Dimoglis
# AM: csd5078

# compiler related variables
CC = g++
CFLAGS = -g

TARGET = alpha_compiler.out

all: $(TARGET)

lexer.c: lexer.l 
	flex -o $@ $^

parser.cpp: parser.ypp
	bison --yacc --defines $^ -o $@

sym_table.out: sym_table.cpp
	g++ $^ -o $@

$(TARGET): lexer.c parser.cpp stack.c sym_table.cpp
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm lexer.c parser.{cpp,hpp} $(TARGET) *.out
