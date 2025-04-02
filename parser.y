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
%union {
    int intValue;
    double doubleValue;
    char *stringValue;
}

%start program

//  ### list of terminals ###

%token KEYWORD
%token IF ELSE WHILE FOR FUNCTION RETURN BREAK CONTINUE AND NOT OR LOCAL TRUE FALSE NIL

%token OPERATOR
%token EQUAL_EQUAL BANG_EQUAL PLUS_PLUS MINUS_MINUS GREATER_EQUAL LESS_EQUAL MINUS_UNARY

%token <intValue> INTCONST
%token <doubleValue> REALCONST

%token <stringValue> MY_STRING

%token PUNCTUATION
%token COLON_COLON DOT_DOT

%token <stringValue> IDENTIFIER
%token LINE_COMMENT
// block comment


// priorities
/*

%right ASSIGN
%left OR
%left AND
%nonassoc EQUAL NEQUAL
%nonassoc GRTR GRTREQ LESS LESSEQ
%left PLUS MIN
%left MUL DIV MOD
%right NOT PLUSPLUS MINMIN MINUNARY
%left DOT DOTDOT
%left BRACK_L BRACK_R
%left PAR_L PAR_R

*/

%right '='
%left OR
%left AND
%nonassoc EQUAL_EQUAL BANG_EQUAL
%nonassoc '>' GREATER_EQUAL '<' LESS_EQUAL
%left '+' '-'
%left '*' '/' '%'
%right NOT PLUS_PLUS MINUS_MINUS MINUS_UNARY
%left '.' DOT_DOT
%left '[' ']'
%left '(' ')'
%nonassoc IF
%nonassoc ELSE
%precedence OBJECTDEF_EMPTY

%type <intValue> expr term lvalue primary


%%

program: stmt_series;

stmt_series: stmt_series stmt 
    | /* empty */;

stmt: expr ';'
    | ifstmt
    | whilestmt
    | forstmt
    | returnstmt
    | BREAK ';'
    | CONTINUE ';'
    | block
    | funcdef
    | ';'
    ;

expr: assignexpr
    | expr '+' expr
    | expr '-' expr
    | expr '*' expr
    | expr '/' expr
    | expr '%' expr
    | expr '>' expr
    | expr GREATER_EQUAL expr
    | expr LESS_EQUAL expr
    | expr EQUAL_EQUAL expr
    | expr BANG_EQUAL expr
    | expr AND expr
    | expr OR expr
    | term
    ;

term: '(' expr ')'       { $$ = ($2); }
    | '-' expr           { $$ = -$2; } %prec MINUS_UNARY
    | NOT expr           { $$ = !$2; }
    | PLUS_PLUS lvalue   { $$ = ++$2; }
    | lvalue PLUS_PLUS   { $$ = $1++; }
    | MINUS_MINUS lvalue { $$ = --$2; }
    | lvalue MINUS_MINUS { $$ = $1--; }
    | primary            { $$ = $1; }
    ;

assignexpr: lvalue '=' expr;

primary: lvalue
    | call
    | objectdef
    | '('funcdef')'
    | const
    ;

lvalue: IDENTIFIER 
    | LOCAL IDENTIFIER 
    | COLON_COLON IDENTIFIER 
    | member
    ;

member: lvalue '.' IDENTIFIER 
    | lvalue '[' expr ']'
    | call '.' IDENTIFIER 
    | call '[' expr ']'
    ;

call: call '(' elist ')'
    | lvalue callsuffix
    | '('funcdef')' '(' elist ')'
    ;

callsuffix: normcall | methodcall;

normcall: '(' elist ')';

// equivalent to lvalue.id(lvalue, elist)
methodcall: DOT_DOT IDENTIFIER '(' elist ')'; 

elist: expr elist_alt
    | /* empty */
    ;

elist_alt: ',' expr elist_alt
    | /* empty */;

objectdef: '[' elist ']'
    | '[' indexed ']'
    ;

indexed: indexedelem indexed_alt;

indexed_alt: ',' indexedelem indexed_alt
    | /* empty */
    ;

indexedelem: '{' expr ':' expr '}';

block: '{' stmt_series '}'

funcdef: FUNCTION [id] '('idlist')' block;

const: INTCONST | REALCONST | MY_STRING | NIL | TRUE | FALSE;

idlist: IDENTIFIER idlist_alt
    | /* empty */
    ;

idlist_alt: ',' IDENTIFIER 
    | /* empty */
    ;

ifstmt: IF '(' expr ')' stmt %prec IF
    | IF '(' expr ')' stmt ELSE stmt
    ;

whilestmt: WHILE '(' expr ')' stmt;
forstmt: FOR '(' elist';' expr';' elist')' stmt;
returnstmt: RETURN [expr];

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
