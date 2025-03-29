%{
    #include <stdio.h>
    #include "stack.h"
    int yylex (void);
    void yyerror(const char *msg);

    extern Stack *stack;
    extern int yylineno;
    extern char *yytext;
    extern FILE *yyin;

%}

// TODO: find replacement of %error-verbose

%start program

// token list
%token NUM


// priorities
%right '='
%left ','
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS
%left '(' ')'

// grammar
%%

program: stmt;

stmt:;

%%

/* NOTE: maybe not needed as it is defined in lexer.l
void yyerror(const char *msg) {
    fprintf(stderr, "%s at line %s before token: %s\n", msg, yylineno, yytext);
    fprintf(stderr, "INPUT NOT VALID\n");
}
*/

int main (int argc, char **argv) {
    if (argc > 1) {
        if (!(yyin = fopen(argv[1], "r"))) {
            fprintf(stderr, "Cannot read file: %s\n", argv[1]);
            return 1;
        }
    } else
        yyin = stdin;


    stack = initStack();
    yyparse();

    /* TODO: needed?
    fclose(yyin);
    fclose(yyout);
    free_token_list();
    */

    return 0;
}
