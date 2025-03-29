# Author: Angelos T. Dimoglis
# AM: csd5078

# compiler related variables
CC = gcc
#CFLAGS = -ansi -pedantic -Wall -g

all: calc

lexer.c: lexer.l 
	flex --outfile=lexer.c lexer.l

parser.c: parser.y
	bison --yacc --defines --output=parser.c $<

calc: lexer.c parser.c stack.c
	$(CC) lexer.c parser.c stack.c -g -o $@

clean:
	rm *.out ; rm lexer.c parser.c parser.h
