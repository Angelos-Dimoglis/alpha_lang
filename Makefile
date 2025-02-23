# Author: Angelos T. Dimoglis
# AM: csd5078

# compiler related variables
CC = gcc
#CFLAGS = -ansi -pedantic -Wall -g

all: lex.out

lex.yy.c: al.l
	flex al.l

lex.out: lex.yy.c
	$(CC) $< -g -o $@

#run_tests:

clean:
	rm *.out && rm lex.yy.c
