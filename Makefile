# Author: Angelos T. Dimoglis
# AM: csd5078

# compiler related variables
CC = gcc
#CFLAGS = -ansi -pedantic -Wall -g

all: my_parser.out

lexer.c: lexer.l 
	flex -o lexer.c lexer.l

parser.c: parser.y
	bison --yacc --defines -o parser.c $<

my_parser.out: lexer.c parser.c stack.c
	$(CC) lexer.c parser.c stack.c -g -o $@

clean:
	rm *.out ; rm lexer.c parser.c parser.h
