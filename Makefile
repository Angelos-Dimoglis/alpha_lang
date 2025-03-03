# Author: Angelos T. Dimoglis
# AM: csd5078

# compiler related variables
CC = gcc
#CFLAGS = -ansi -pedantic -Wall -g

all: lex.out

lex.yy.c: al.l stack.c
	flex al.l stack.c

lex.out: lex.yy.c
	$(CC) $< -g -o $@

clean:
	rm *.out ; rm lex.yy.c
