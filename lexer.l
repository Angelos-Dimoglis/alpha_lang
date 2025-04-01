%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include "stack.h"
#include "parser.h"

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

alpha_token_t* create_token(int lineno, char* content, char* category_1, char* category_2, char* type) {
    assert(
        content &&
        category_1 &&
        category_2 &&
        type
    );

    static int yynumToken = 0;
    static alpha_token_t* current_token = NULL;
    alpha_token_t* new_token = malloc(sizeof(alpha_token_t));
    assert(new_token);

    new_token->numline = lineno;
    new_token->numToken = ++yynumToken;
    new_token->content = malloc(sizeof(char) * (strlen(content) + 1));
    new_token->category_1 = malloc(sizeof(char) * (strlen(category_1) + 1));
    new_token->category_2 = malloc(sizeof(char) * (strlen(category_2) + 1));
    new_token->type = malloc(sizeof(char) * (strlen(type) + 1));

    assert(
        new_token->content &&
        new_token->category_1 &&
        new_token->category_2 &&
        new_token->type
    );

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

void free_token_node (alpha_token_t *node) {
    assert(node);
        
    free(node->content);
    free(node->category_1);
    free(node->category_2);
    free(node->type);

    free(node);
}

void free_token_list () {
    
    if (list_head == NULL) {
        printf("List is empty\n");
        return;
    }

    alpha_token_t *prev = list_head;
    for (alpha_token_t *curr = list_head->alpha_token_next;
        curr != NULL; curr = curr->alpha_token_next) {
        free_token_node(prev);
        prev = curr;
    }
    
}

void print_token(alpha_token_t* token) {
    char tab[3];
    strcpy(tab, (strlen(token->content) >= 5 ? "\t" : "\t\t"));
    fprintf(yyout, "%d: #%d\t \"%s\"%s %s\t %s\t <-%s\n", 
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

    for (int i = 0; str[i] != '\0'; i++)
        capital_str[i] = toupper((unsigned char)str[i]);

    return capital_str;
}

void yyerror(const char *msg, int line_number) {

    if (yyout != stdout)
        fprintf(yyout, "Error at #%d: %s\n", line_number, msg);
        
    fprintf(stderr, "\033[0;31mError at #%d: \033[0m%s\n", line_number, msg);

    free_token_list();
    exit(1);
}

int translate (char *str) {

    if (strcmp(str, "if")) { return IF;
    } else if( strcmp(str, "else")) { return ELSE;
    } else if( strcmp(str, "while")) { return WHILE;
    } else if( strcmp(str, "for")) { return FOR;
    } else if( strcmp(str, "for")) { return FOR;
    } else if( strcmp(str, "function")) { return FUNCTION;
    } else if( strcmp(str, "return")) { return RETURN;
    } else if( strcmp(str, "break")) { return BREAK;
    } else if( strcmp(str, "continue")) { return CONTINUE;
    } else if( strcmp(str, "and")) { return AND;
    } else if( strcmp(str, "not")) { return NOT;
    } else if( strcmp(str, "or")) { return OR;
    } else if( strcmp(str, "local")) { return LOCAL;
    } else if( strcmp(str, "true")) { return TRUE;
    } else if( strcmp(str, "false")) { return FALSE;
    } else if( strcmp(str, "nil")) { return NIL;
    }

}

%}

%option noyywrap
%option yylineno
%option stack

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

%%

\n|" "|\t {}

{KEYWORD}  {
    char* UpperCase = toUpperCase(yytext);
    alpha_token_t* token = 
        create_token(yylineno, yytext, "KEYWORD", UpperCase, "enumerated");
    print_token(token);
    free(UpperCase);

    return translate(yytext);
}

{OPERATOR}  {
    char name_of_character[100];
    int return_type = yytext[0]; // pre-assign and change later if needed

    // TODO: change ternaries to single ifs

    switch (yytext[0]) {
        case '=':
            return_type = (yytext[1] == '=' ? EQUAL_EQUAL : '=');
            strcpy(name_of_character, (yytext[1] == '=' ? "EQUAL_EQUAL" : "EQUAL"));
            break;
        case '+':
            return_type = (yytext[1] == '+' ? PLUS_PLUS : '+');
            strcpy(name_of_character, (yytext[1] == '+' ? "PLUS_PLUS" : "PLUS"));
            break;
        case '-':
            return_type = (yytext[1] == '-' ? MINUS_MINUS : '-');
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
            if (yytext[1] == '=') {
                return_type = BANG_EQUAL;
                strcpy(name_of_character, "BANG_EQUAL");
            } else
                assert(NULL);

            break;
        case '>':
            return_type = (yytext[1] == '=' ? GREATER_EQUAL : '>');
            strcpy(name_of_character, (yytext[1] == '=' ? "GREATER_EQUAL" : "GREATER"));
            break;
        case '<':
            return_type = (yytext[1] == '=' ? LESS_EQUAL : '<');
            strcpy(name_of_character, (yytext[1] == '=' ? "LESS_EQUAL" : "LESS"));
            break;
        default:
            assert(NULL);
    }

    alpha_token_t* token = create_token(
        yylineno, yytext, "OPERATOR", name_of_character, "enumerated");
    print_token(token);

    return return_type;
}

{INTCONST}  {
    alpha_token_t* token = create_token(
        yylineno, yytext, "INTCONST", yytext, "int");
    print_token(token);

    yylval.intValue = atoi(yytext);
    return INTCONST;
}

{REALCONST}  {
    alpha_token_t* token = create_token(
        yylineno, yytext, "REALCONST", yytext, "double");
    print_token(token);

    yylval.doubleValue = atof(yytext);
    return REALCONST;
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
    alpha_token_t* token = create_token(
        yylineno, temp, "STRING", yytext, "char*");
    print_token(token);
    free(temp);

    BEGIN(INITIAL);
    
    yylval.stringValue = strdup(yytext);    
    return MY_STRING;
}
<STRING><<EOF>> { yyerror("Unclosed string", start_of_string); }
<STRING>.       { yyerror("Unclosed string", start_of_string); }

{PUNCTUATION} {
    char name_of_character[100];
    int return_type = yytext[0];

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
            if (yytext[1] == ':') return_type = COLON_COLON ;
            strcpy(name_of_character,
                (yytext[1] == ':' ? "COLON_COLON " : "COLON"));
            break;
        case '.':
            if (yytext[1] == '.') return_type = DOT_DOT;
            strcpy(name_of_character,
                (yytext[1] == '.' ? "DOT_DOT" : "DOT"));
            break;
        default:
            assert(NULL);            
    }

    alpha_token_t *token = create_token(
        yylineno, yytext, "PUNCTUATION", name_of_character, "enumerated");
    print_token(token);

    return PUNCTUATION;
}


{IDENTIFIER} {
    char *str = malloc(strlen(yytext) + 2);
    sprintf(str, "\"%s\"", yytext);
    alpha_token_t* token = create_token(yylineno, yytext, "ID", str, "char*");
    print_token(token);
    free(str);

    yylval.stringValue = strdup(yytext);    
    return IDENTIFIER;
}

{LINE_COMMENT} {
    char *content = yytext + 2;
    alpha_token_t* token = create_token(
        yylineno, content, "COMMENT", "LINE_COMMENT", "enumerated");
    print_token(token);

    return LINE_COMMENT;
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

        alpha_token_t* token = create_token(
            start, range_str, "COMMENT", "BLOCK_COMMENT", "enumerated");
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
    char str[100];
    snprintf(str, 100, "Undefined token: %s", yytext);
    yyerror(str, yylineno);
}

%%

/*
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
        } else
            yyout = stdout;
    } else
        yyin = stdin;

    stack = initStack();
    yylex();

    fclose(yyin);
    fclose(yyout);
    free_token_list();

    return 0;
}
*/
