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

%token KEYWORD
%token OPERATOR
%token INTCONST
%token REALCONST
// string
%token PUNCTUATION
%token IDENTIFIER
%token LINE_COMMENT
// block comment

%token GREATER_EQALS 
%token LESS_EQUAL
%token EQUAL
%token BANG_EQUAL

// priorities
/*
%right '='
%left ','
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS
%left '(' ')'
*/

// grammar
%%

program: stmt;

stmt: expr';'
    | ifstmt
    | whilestmt
    | forstmt
    | returnstmt
    | break';'
    | continue';'
    | block
    | funcdef
    |;

expr: assignexpr
    | expr op expr
    | term
    ;

op: '+'
    | '-'
    | '*'
    | '/'
    | '%'
    | '>'
    | GREATER_EQUAL
    | LESS_EQUAL
    | EQUAL
    | BANG_EQUAL
    | and
    | or
    ;

term: '(' expr ')'
    | '-' expr
    | not expr
    | "++"lvalue
    | lvalue"++"
    | --lvalue
    | lvalue--
    | primary
    ;

assginexpr: lvalue '=' expr;

primary: lvalue
    | call
    | objectdef
    | '('funcdef')'
    | const
    ;

lvalue: id
    | local id
    | :: id
    | member
    ;

member: lvalue.id
    | lvalue '[' expr ']'
    | call . id
    | call '[' expr ']'
    ;

call: call '(' elist ')'
    | lvalue callsuffix
    | '('funcdef')' '(' elist ')'
    ;

callsuffix: normcall | methodcall;

normcall: '(' elist ')';

// equivalent to lvalue.id(lvalue, elist)
methodcall: .. id '(' elist ')'; 

elist: [ expr [, expr] * ];

objectdef: '[' [elist | indexed] ']';
indexed: [indexedelem [, indexedelem] * ];
indexedelem: '{' expr ':' expr '}';

block: '{'[stmt*]'}'

funcdef: function [id] '('idlist')' block;

const: number | string | nil | true | false;
idlist: [id [, id] * ];

ifstmt: if '(' expr ')' stmt [ else stmt ];
whilestmt: while '(' expr ')' stmt;
forstmt: for '(' elist';' expr';' elist')' stmt;
returnstmt: return [expr];

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
