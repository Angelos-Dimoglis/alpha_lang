%{
    #include <stdio.h>
    #include <string.h>
    #include <stack>
    #include <assert.h>

    #include "parser_functions.h"
    #include "icode_gen.h"

    extern int yylex (void);
    extern void yyerror(const char *msg, int line_number);

    extern int yylineno;
    extern char *yytext;
    extern FILE *yyin;
    extern int print_lexer_tokens;

    unsigned int scope = 0;
    SymTable sym_table;
    list<Variable*> args;

    quad *quads = (quad*) 0;
    unsigned total = 0;
    unsigned int curr_quad = 0;

    void yyerror(const char *msg) {
        if (strcmp(msg, "syntax error, unexpected end of file"))
            fprintf(stderr, "%s at line %d before token: %s\n",
                msg, yylineno, yytext);
    }

    std::stack<int> loopcounter;

    void push_loopcounter() {
        loopcounter.push(0);
    }

    void pop_loopcounter() {
        int temp = loopcounter.top();
        loopcounter.pop();
        assert(temp == 0);
    }

    void increase_loopcounter() {
        int temp = loopcounter.top() + 1;
        loopcounter.pop();
        loopcounter.push(temp);
    }

    void decrease_loopcounter() {
        int temp = loopcounter.top() - 1;
        loopcounter.pop();
        loopcounter.push(temp);
    }

    void break_continue_valid(string break_or_continue) {
        if (loopcounter.empty() || loopcounter.top() == 0) {
            yyerror(("Use of \'" + break_or_continue + "\' while not in a loop").c_str());
        }
    }

    void return_valid() {
        if (loopcounter.empty()) {
            yyerror("Use of \'return\' while not in a function");
        }
    }

%}

%define parse.error detailed

%union {
    int intValue;
    double doubleValue;
    char *stringValue;
    struct expr *exprValue;
    struct Function *funcSymValue;
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

%type <exprValue> expr term lvalue primary member
%type <intValue> block
%type <funcSymValue> funcname

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
    | BREAK ';' {break_continue_valid("break");}
    | CONTINUE ';' {break_continue_valid("continue");}
    | block
    | funcdef
    | ';'
    ;

expr: assignexpr {}
    | expr '+' expr {
        //f(expr1, expr2, operator);
    }
    | expr '-' expr {

    }
    | expr '*' expr {

    }
    | expr '/' expr {

    }
    | expr '%' expr {

    }
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
    | PLUS_PLUS lvalue {check_lvalue($2->sym->name);}
    | lvalue PLUS_PLUS {check_lvalue($1->sym->name);}
    | MINUS_MINUS lvalue {check_lvalue($2->sym->name);}
    | lvalue MINUS_MINUS {check_lvalue($1->sym->name);}
    | primary
    ;

assignexpr: lvalue '=' expr {check_lvalue($1->sym->name);}; 

primary: lvalue
    | call {}
    | objectdef {}
    | '('funcdef')' {}
    | const {}
    ;

lvalue: IDENTIFIER {
        // $$ = $1;
        Variable* sym = add_id($1);
        $$ -> sym = sym;
    }
    | LOCAL IDENTIFIER {
        // $$ = $2;
        Variable* sym = add_local_id($2);
    }
    | COLON_COLON IDENTIFIER {
        // $$ = $2;
        Symbol* sym = lookup_global_id($2);
    }
    | member {}
    ;

member: lvalue '.' IDENTIFIER {$$ = member_item($1, $3);}
    | lvalue '[' expr ']'
    | call '.' IDENTIFIER {/* $$ = $3; */}
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

block: '{' {scope++;} stmt_series '}' {sym_table.Hide(scope--); $$ = getoffset();};

funcblockstart: { scope--; enterscopespace(); push_loopcounter(); };
funcblockend: { exitscopespace(); exitscopespace(); pop_loopcounter(); };
formal_arguments: '(' {scope++; enterscopespace();} idlist ')';
funcname: IDENTIFIER { $$ = add_func($1);}
    | { $$ = add_func("_f"); };

funcdef: FUNCTION funcname
        formal_arguments
        funcblockstart block { $2 ->num_of_locals = $5; } funcblockend;
                                            //^ Possibility for errors
                                            // if function is not
                                            // initialized due to errors

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

loopstart: { increase_loopcounter; }
loopend: { decrease_loopcounter; }
whilestmt: WHILE '(' expr ')' loopstart stmt loopend;
forstmt: FOR '(' elist';' expr';' elist')' loopstart stmt loopend;
returnstmt: RETURN ';' {return_valid();}
    | RETURN  expr ';' {return_valid();}
    ;

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

    // TODO: write all quads in a file
    for (int i = 0; i < total; i++)
        print_quad(&(quads[i]));

    sym_table.PrintTable();
    sym_table.freeTable();  

    return 0;
}
