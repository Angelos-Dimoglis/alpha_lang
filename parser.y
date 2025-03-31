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

//  ### list of terminals ###

%token KEYWORD
%token IF ELSE WHILE FOR FUNCTION RETURN BREAK CONTINUE AND NOT OR LOCAL TRUE FALSE NIL

%token OPERATOR
%token EQUAL_EQAL BANG_EQUAL PLUS_PLUS MINUS_MINUS GREATER_EQUAL LESS_EQUAL

%token <> INTCONST
%token <> REALCONST

// string

%token PUNCTUATION
%token IDENTIFIER
%token LINE_COMMENT
// block comment

%token NOT
%token COLON_COLON

%token 

// priorities
/*
%right '='
%left ','
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS
%left '(' ')'
*/

%%

program: stmt_series;

stmt_series: stmt_series stmt 
    | /* empty */;

stmt: expr';'
    | ifstmt
    | whilestmt
    | forstmt
    | returnstmt
    | break';'
    | continue';'
    | block
    | funcdef
    | /* empty */;

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

term: '(' expr ')' {
        $$ = ($2);
    }
    | '-' expr {
        $$ = -$2;
    }
    | NOT expr {
        $$ = !$2;
    }
    | PLUS_PLUS lvalue {
        $$ = ++$2;
    }
    | lvalue PLUS_PLUS {
        $$ = $1++;
    }
    | MINUS_MINUS lvalue {
        $$ = --$1;
    }
    | lvalue MINUS_MINUS {
        $$ = $1--;
    }
    | primary {
        $$ = $1;
    }
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
    | COLON_COLON id {
        $$ = $1 $2;
    }
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

elist: expr
    | expr ',' elist
    | /* empty */;

objectdef: '[' elist ']'
    | '[' indexed ']'
    | '[' ']'
    ;

indexed: indexedelem indexed_alt
    | /* empty */;

indexed_alt: ',' indexelem indexed_alt
    | /* empty */;

indexedelem: '{' expr ':' expr '}';

block: '{' stmt_series '}'

funcdef: function [id] '('idlist')' block;

const: number | string | nil | true | false;

idlist: id idlist_alt
    | /* empty */;

idlist_alt: ',' id
    | /* empty */;

ifstmt: if '(' expr ')' stmt ifstmt_alt;

ifstmt_alt: else stmt
    | /* empty */;

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
