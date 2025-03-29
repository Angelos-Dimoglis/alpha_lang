# Author: Angelos T. Dimoglis
# AM: csd5078

# compiler related variables
CC = gcc
CFLAGS = -g

all: alpha_compiler.out

lexer.c: lexer.l 
	flex -o lexer.c $^

parser.c: parser.y
	bison --yacc --defines $^ -o parser.c

alpha_compiler.out: lexer.c parser.c stack.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm lexer.c parser.c parser.h my_parser.out
