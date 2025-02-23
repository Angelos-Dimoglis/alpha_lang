%{
#include <stdio.h>

FILE* yyout;
%}

number [0-9]

%option noyywrap

%%

{number} { 
    fprintf(stderr, "thats a number: %s\n", yytext);
}

%%

int main(int argc, char **argv) {
    if (argc > 1) {
        if (!(yyin = fopen(argv[1], "r"))) {
            fprintf(stderr, "Cannot read file: %s\n", argv[1]);
            return 1;
        }
        if (argc == 3) {
            if (!(yyout = fopen(argv[2], "w"))) {
                fprintf(stderr, "Cannot write file: %s\n", argv[2]);
                fclose(yyin);
                return 1;
            }
        } else
            yyout = stdout;
    } else
        yyin = stdin;
    yylex();  // Call the lexer

    fclose(yyin);
    fclose(yyout);

    return 0;
}
