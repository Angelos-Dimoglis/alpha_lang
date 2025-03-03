%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include "stack.h"

FILE* yyout;
Stack *stack;
int start_of_string;

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

void yyerror(const char *msg, int line_number) {
    fprintf(stderr, "Error at #%d: %s\n", line_number, msg);
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

BLOCK_COMMENT_START ("/*")
BLOCK_COMMENT_END   ("*/")
%x IN_COMMENT

%x STRING

%option noyywrap
%option yylineno
%option stack

%%


\n {}

" " {}

{KEYWORD}  {
    char* UpperCase = toUpperCase(yytext);
    alpha_token_t* token = create_token(yytext, "KEYWORD", UpperCase, "enumerated");
    print_token(token);
    free(UpperCase);
}

{OPERATOR}  {
    char name_of_character[100];

    switch (yytext[0]) {
        case '=':
            strcpy(name_of_character, (yytext[1] == '=' ? "EQUALS_EQUALS" : "EQUALS"));
            break;
        case '+':
            strcpy(name_of_character, (yytext[1] == '+' ? "PLUS_PLUS" : "PLUS"));
            break;
        case '-':
            strcpy(name_of_character, (yytext[1] == '-' ? "MINUS_MINUS" : "MINUS"));
            break;
        case '*':
            strcpy(name_of_character, "ASTERISK");
            break;
        case '/':
            strcpy(name_of_character, "SLASH");
            break;
        case '%':
            strcpy(name_of_character, "PERCENT");
            break;
        case '!':
            if (yytext[1] == '=')
                strcpy(name_of_character, "BANG_EQUALS");
            else
                assert(NULL);

            break;
        case '>':
            strcpy(name_of_character, (yytext[1] == '=' ? "GREATER_EQALS" : "GREATER"));
            break;
        case '<':
            strcpy(name_of_character, (yytext[1] == '=' ? "LESS_EQUALS" : "LESS"));
            break;
        default:
            assert(NULL);
    }

    alpha_token_t* token = create_token(yytext, "OPERATOR", name_of_character, "enumerated");
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

\"  {
    unput('\"');
    start_of_string = yylineno;
    BEGIN(STRING);
}
<STRING>\n      {yylineno++;}
<STRING>\"(\\.|[^\\"])*\"      {
    char* temp = malloc(sizeof(char) * yyleng);
    int j = 0;
    for (int i = 1; i < yyleng; i++){
        if (yytext[i] == '\\'){
            //TODO: Possibility for seg
            switch (yytext[i+1]){
                case '\"': 
                    temp[j++] = '\"';
                    i++;
                    break;
                case 't':
                    temp[j++] = '\t';
                    i++;
                    break;
                case 'n':
                    temp[j++] = '\n';
                    i++;
                    break;
                case '\\':
                    temp[j++] = '\\';
                    i++;
                    break;
                default:
                    temp[j++] = yytext[i];
                    break;
            }
        }else
            temp[j++] = yytext[i];
    }
    temp[j - 1] = '\0';

    strcpy(&yytext[1], temp);
    yytext[j] = '\"';
    yytext[j+1] = '\0';
    alpha_token_t* token = create_token(temp, "STRING", yytext, "char*");\
                           print_token(token);
    BEGIN(INITIAL);
}
<STRING><<EOF>> { yyerror("Unclosed string", start_of_string); }
<STRING>.       { yyerror("Unclosed string", start_of_string); }

{PUNCTUATION} {
    char name_of_character[100];

    switch (yytext[0]) {
        case '{':
            strcpy(name_of_character, "OPENING_BRACE");
            break;
        case '}':
            strcpy(name_of_character, "CLOSING_BRACE");
            break;
        case '[':
            strcpy(name_of_character, "OPENING_BRACKET");
            break;
        case ']':
            strcpy(name_of_character, "CLOSING_BRACKET");
            break;
        case '(':
            strcpy(name_of_character, "OPENING_PARENTHESIS");
            break;
        case ')':
            strcpy(name_of_character, "CLOSING_PARENTHESIS");
            break;
        case ';':
            strcpy(name_of_character, "SEMICOLON");
            break;
        case ',':
            strcpy(name_of_character, "COMMA");
            break;
        case ':':
            strcpy(name_of_character, (yytext[1] == ':' ? "DOUBLE_COLON" : "COLON"));
            break;
        case '.':
            strcpy(name_of_character, (yytext[1] == '.' ? "DOUBLE_DOT" : "DOT"));
            break;
        default:
            assert(NULL);            
    }

    alpha_token_t *token = create_token(yytext, "PUNCTUATION", name_of_character, "enumerated");
    print_token(token);
}


{IDENTIFIER} {
    char *str = malloc(strlen(yytext) + 2);
    sprintf(str, "\"%s\"", yytext);
    alpha_token_t* token = create_token(yytext, "ID", str, "char*");
    print_token(token);
}

{LINE_COMMENT} {
    char *content = yytext + 2;
    alpha_token_t* token = create_token(content, "COMMENT", "LINE_COMMENT", "enumerated");
    print_token(token);
}

{BLOCK_COMMENT_START} {
    push(stack, yylineno);
    BEGIN(IN_COMMENT); 
}
<IN_COMMENT>{
    {BLOCK_COMMENT_START} {
        push(stack, yylineno);
        BEGIN(IN_COMMENT); 
    }
    {BLOCK_COMMENT_END} {
        int start = pop(stack), end = yylineno;
        char range_str [100];

        snprintf(range_str, sizeof(range_str), "%d - %d", start, end);

        alpha_token_t* token = create_token(range_str, "COMMENT", "BLOCK_COMMENT", "enumerated");
        print_token(token);

        if (isEmpty(stack))
            BEGIN(INITIAL);
    }
    [^\/*\n]+
    "/"
    "*"
    \n
    <<EOF>> {
        yyerror("Unclosed comment", peek(stack));
    }
}

. {
    yyerror("Undefined token", yylineno);
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

    stack = initStack();
    yylex();  // Call the lexer

    fclose(yyin);
    fclose(yyout);

    return 0;
}
