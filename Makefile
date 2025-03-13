# Author: Angelos T. Dimoglis
# AM: csd5078

# compiler related variables
CC = gcc
#CFLAGS = -ansi -pedantic -Wall -g

all: lex.out

lex.yy.c: lexer.l stack.c
	flex lexer.l stack.c

lex.out: lex.yy.c
	$(CC) $< -g -o $@

run_tests: lex.out
	./lex.out tests/general.alpha

clean:
	rm *.out ; rm lex.yy.c
