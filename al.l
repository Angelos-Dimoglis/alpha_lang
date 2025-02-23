%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

FILE* yyout;

typedef struct alpha_token {
  unsigned int     numline;
  unsigned int     numToken;
  char          *content;
  char          *category_1;
  char          *category_2;
  char          *type;
  struct alpha_token *alpha_token_next;
} alpha_token_t;

alpha_toke_t *list_head = NULL;

alpha_token_t* create_token(char* content, char* category_1, char* category_2, char* type) {
    assert(content);
    assert(category_1);
    assert(category_2);
    assert(type);


    static int yynumToken = 0;
    static alpha_token_t* current_token = NULL;
    alpha_toke_t* new_token = malloc(sizeof(alpha_token_t*));
    assert(new_token);

    new_token->numline = yylineno;
    new_token->numToken = ++yynumToken;
    strcpy(new_token->content, content);
    strcpy(new_token->category_1, category_1);
    strcpy(new_token->category_2, category_2);
    strcpy(new_token->type, type);
    new_token->alpha_token_next = NULL;
    
    if (!list_head)
        list_head = new_token;
    else
        current_token->alpha_token_next = new_token;

    current_token = new_token;

    return current_token;
}

void print_token(alpha_token_t* token) {
    printf("%d: #%d\t \"%s\"\t %s\t %s <-%s", 
        token->numline, token->numToken, token->content, token->category_1, token->category_2, token->type);    
}
%}

KEYWORD if|else|while|for|function|return|break|continue|and|not|or|local|true|false|nil
OPERATOR "="|"+"|"-"|"*"|"/"|"%"|"=="|"!="|"++"|"--"|">"|"<"|">="|"<="
INTCONST [0-9]+

%option noyywrap
%option yylineno

%%

{KEYWORD}  {}
{OPERATOR}  {printf("OPERATOR", );}
{INTCONST}  {printf("INTCONST");}

%%

int main(int argc, char **argv) {
    if (argc > 1){
        if (!(yyin = fopen(argv[1], "r"))){
            fprintf(stderr, "Cannot read file: %s\n", argv[1]);
            return 1;
        }
        if (argc == 3){
            if (!(yyout = fopen(argv[2], "w"))){
                fprintf(stderr, "Cannot write file: %s\n", argv[2]);
                fclose(yyin);
                return 1;
            }
        }else
            yyout = stdout;
    }else
        yyin = stdin;
    yylex();  // Call the lexer

    fclose(yyin);
    fclose(yyout);

    return 0;
}
