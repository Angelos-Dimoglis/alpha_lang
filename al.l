%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

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

alpha_token_t *list_head = NULL;

alpha_token_t* create_token(char* content, char* category_1, char* category_2, char* type) {
    assert(content);
    assert(category_1);
    assert(category_2);
    assert(type);


    static int yynumToken = 0;
    static alpha_token_t* current_token = NULL;
    alpha_token_t* new_token = malloc(sizeof(alpha_token_t));
    assert(new_token);

    new_token->numline = yylineno;
    new_token->numToken = ++yynumToken;
    new_token->content = malloc(sizeof(char) * (strlen(content) + 1));
    new_token->category_1 = malloc(sizeof(char) * (strlen(category_1) + 1));
    new_token->category_2 = malloc(sizeof(char) * (strlen(category_2) + 1));
    new_token->type = malloc(sizeof(char) * (strlen(type) + 1));
    assert(new_token->content && new_token->category_1 && new_token->category_2 && new_token->type);
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
    char* tab = malloc(sizeof(char) * 2);
    tab = strlen(token->content) >= 5 ? "\t" : "\t\t";
    printf("%d: #%d\t \"%s\"%s %s\t %s\t <-%s\n", 
        token->numline,
        token->numToken,
        token->content,
        tab,
        token->category_1,
        token->category_2,
        token->type
    );    
}

char* toUpperCase(const char *str) {
    char* capital_str = malloc(sizeof(char) * strlen(str));
    assert(capital_str);
    for (int i = 0; str[i] != '\0'; i++) {
        capital_str[i] = toupper((unsigned char)str[i]);
    }
    return capital_str;
}

void yyerror(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}

%}

LETTER [a-zA-Z]
DIGIT [0-9]
ANY_CHARACTER (.|\n)

KEYWORD             if|else|while|for|function|return|break|continue|and|not|or|local|true|false|nil
OPERATOR            "="|"+"|"-"|"*"|"/"|"%"|"=="|"!="|"++"|"--"|">"|"<"|">="|"<="
INTCONST            {DIGIT}+
REALCONST           {INTCONST}\.{INTCONST}
PUNCTUATION         "{"|"}"|"["|"]"|"("|")"|";"|","|":"|"::"|"."|".."
IDENTIFIER          {LETTER}({LETTER}|{DIGIT}|_)*
LINE_COMMENT        (\/\/).*$

BLOCK_COMMENT_START ("\/\*")
BLOCK_COMMENT_END   ("\*\/")
BLOCK_COMMENT       {BLOCK_COMMENT_START}{ANY_CHARACTER}*?{BLOCK_COMMENT_END}

%option noyywrap
%option yylineno
%x STRING


%%


\n {}

" " {}

{KEYWORD}  {
    alpha_token_t* token = create_token(yytext, "KEYWORD", toUpperCase(yytext), "enumerated");
    print_token(token);
}

{OPERATOR}  {
    alpha_token_t* token = create_token(yytext, "OPERATOR", yytext, "enumerated");
    print_token(token);
}

{INTCONST}  {
    alpha_token_t* token = create_token(yytext, "INTCONST", yytext, "int");
    print_token(token);
}

{REALCONST}  {
    alpha_token_t* token = create_token(yytext, "REALCONST", yytext, "double");
    print_token(token);
}

\"  {BEGIN(STRING);}
<STRING>\n      {yylineno++;}
<STRING>\"      {
    char* temp = malloc(sizeof(char) * yyleng);
    for (int i = 0; i < yyleng; i++){
        if (yytext[i] == '\\'){
            //TODO: Possibility for seg
            switch (yytext[i+1]){
                case '\"': 
                    temp[i++] = '\"';
                case 't':
                    temp[i++] = '\t';
                case 'n':
                    temp[i++] = '\n';
                case '\\':
                    temp[i++] = '\\';
                default:
                    temp[i] = yytext[i];
            }
        }else
            temp[i] = yytext[i];
    }
    //temp[yyleng - 2] = '\0';

    alpha_token_t* token = create_token(temp, "STRING", temp, "char*");\
                           print_token(token);
    BEGIN(INITIAL);
}
<STRING><<EOF>> { yyerror("Unclosed string"); }
<STRING>.       {}

{LINE_COMMENT} {
    alpha_token_t* token = create_token(yytext, "COMMENT", "LINE_COMMENT", "enumerated");
    print_token(token);
}

{BLOCK_COMMENT} {
    char *start_to_end = 0;
    printf("block comment\n");
    alpha_token_t* token = create_token("test_range", "COMMENT", "BLOCK_COMMENT", "enumerated");
    print_token(token);
}

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
