# Author: Angelos T. Dimoglis
# AM: csd5078

# compiler related variables
CC = gcc
CFLAGS = -g

TARGET = alpha_compiler.out

all: $(TARGET)

lexer.c: lexer.l 
	flex -o $@ $^

parser.c: parser.y
	bison --yacc --defines $^ -o $@

counter_parser.c: parser.y
	bison -Wcounterexamples --yacc --defines $^ -o $@

$(TARGET): lexer.c parser.c stack.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm lexer.c parser.{c,h} $(TARGET)
