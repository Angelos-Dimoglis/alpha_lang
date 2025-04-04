%{
    #include <stdio.h>
    #include <string.h>

    #include "parser_functions.h"

    extern int yylex (void);
    extern void yyerror(const char *msg, int line_number);

    extern int yylineno;
    extern char *yytext;
    extern FILE *yyin;
    extern int print_lexer_tokens;

    unsigned int scope = 0;
    SymTable sym_table;
    list<Variable*> args;

    void yyerror(const char *msg) {
        if (strcmp(msg, "syntax error, unexpected end of file"))
            fprintf(stderr, "%s at line %d before token: %s\n",
                msg, yylineno, yytext);
    }

%}

%define parse.error detailed

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

// priorities
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

%type <stringValue> expr term lvalue primary member

%%

program: stmt_series;

stmt_series: stmt_series stmt 
    | /* empty */
    ;

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

expr: assignexpr {}
    | expr '+' expr
    | expr '-' expr
    | expr '*' expr
    | expr '/' expr
    | expr '%' expr
    | expr '>' expr
    | expr '<' expr
    | expr GREATER_EQUAL expr
    | expr LESS_EQUAL expr
    | expr EQUAL_EQUAL expr
    | expr BANG_EQUAL expr
    | expr AND expr
    | expr OR expr
    | term
    ;

term: '(' expr ')' {}
    | '-' expr {} %prec MINUS_UNARY
    | NOT expr {}
    | PLUS_PLUS lvalue {check_lvalue($2);}
    | lvalue PLUS_PLUS {check_lvalue($1);}
    | MINUS_MINUS lvalue {check_lvalue($2);}
    | lvalue MINUS_MINUS {check_lvalue($1);}
    | primary
    ;

assignexpr: lvalue '=' expr {check_lvalue($1);}; 

primary: lvalue
    | call {}
    | objectdef {}
    | '('funcdef')' {}
    | const {}
    ;

lvalue: IDENTIFIER {
        $$ = $1;
        add_id($1);
    }
    | LOCAL IDENTIFIER {
        $$ = $2;
        add_local_id($2);
    }
    | COLON_COLON IDENTIFIER {
        $$ = $2;
        lookup_global_id($2);
    }
    | member {}
    ;

member: lvalue '.' IDENTIFIER {$$ = $3;}
    | lvalue '[' expr ']'
    | call '.' IDENTIFIER {$$ = $3;}
    | call '[' expr ']' {}
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
    | /* empty */
    ;

objectdef: '[' elist ']'
    | '[' indexed ']'
    ;

indexed: indexedelem indexed_alt;

indexed_alt: ',' indexedelem indexed_alt
    | /* empty */
    ;

indexedelem: '{' expr ':' expr '}';

block: '{' {scope++;} stmt_series '}' {sym_table.Hide(scope--);};

funcdef: FUNCTION IDENTIFIER '(' {scope++;} idlist ')' {scope--; add_func($2);} block
    | FUNCTION '(' {scope++;} idlist ')' {scope--; add_func("_f");} block;

const: INTCONST | REALCONST | MY_STRING | NIL | TRUE | FALSE;

idlist: IDENTIFIER {add_formal_argument($1);} idlist_alt
    | /* empty */
    ;

idlist_alt: ',' IDENTIFIER {add_formal_argument($2);} idlist_alt
    | /* empty */
    ;

ifstmt: IF '(' expr ')' stmt %prec IF
    | IF '(' expr ')' stmt ELSE stmt
    ;

whilestmt: WHILE '(' expr ')' stmt;
forstmt: FOR '(' elist';' expr';' elist')' stmt;
returnstmt: RETURN [expr];

%%


int main (int argc, char **argv) {

    // uncomment for debug mode
    // yydebug = 1;

    // Uncomment if you want to see the prints from the lexer for the tokens
    //print_lexer_tokens = 1;

    if (argc > 1) {
        if (!(yyin = fopen(argv[1], "r"))) {
            fprintf(stderr, "Cannot read file: %s\n", argv[1]);
            return 1;
        }
    } else
        yyin = stdin;


    yyparse();

    /* TODO: needed?
    fclose(yyin);
    fclose(yyout);
    free_token_list();
    */

    sym_table.PrintTable();
    sym_table.freeTable();  

    return 0;
}
