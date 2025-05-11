%{
    #include <stdio.h>
    #include <string.h>
    #include <string>
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
    bool boolValue;
    char *stringValue;
    void *nilValue;
    struct expr *exprValue;
    struct Function *funcSymValue;
    struct quad * quadValue;
}

%start program

//  ### list of terminals ###

%token KEYWORD
%token IF ELSE WHILE FOR FUNCTION RETURN BREAK CONTINUE AND NOT OR LOCAL

%token OPERATOR
%token EQUAL_EQUAL BANG_EQUAL PLUS_PLUS MINUS_MINUS GREATER_EQUAL LESS_EQUAL MINUS_UNARY

%token <doubleValue> REALCONST INTCONST
%token <stringValue> MY_STRING
%token <nilValue> NIL
%token <boolValue> TRUE FALSE

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

%type <exprValue> expr term lvalue primary member assignexpr const
%type <intValue> block
%type <funcSymValue> funcname funcdef // NOTE: THIS MIGHT HAVE TO BECOME symValue LATER ON lec 10, sl 7

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
        $$ = new expr(arith_expr_e);
        $$->sym = newtemp();
        emit(add, $1, $3, $$, 0, yylineno);
    }
    | expr '-' expr {
        $$ = new expr(arith_expr_e);
        $$->sym = newtemp();
        emit(sub, $1, $3, $$, 0, yylineno);
    }
    | expr '*' expr {
        $$ = new expr(arith_expr_e);
        $$->sym = newtemp();
        emit(mul, $1, $3, $$, 0, yylineno);
    }
    | expr '/' expr {
        $$ = new expr(arith_expr_e);
        $$->sym = newtemp();
        emit(_div, $1, $3, $$, 0, yylineno);
    }
    | expr '%' expr {
        $$ = new expr(arith_expr_e);
        $$->sym = newtemp();
        emit(mod, $1, $3, $$, 0, yylineno);
    }
    | expr '>' expr {
        $$ = new expr(bool_expr_e);
        $$->sym = newtemp();
        emit(if_greater, $1, $3, $$, 0, yylineno);
    }
    | expr '<' expr {
        $$ = new expr(bool_expr_e);
        $$->sym = newtemp();
        emit(if_less, $1, $3, $$, 0, yylineno);
    }
    | expr GREATER_EQUAL expr {
        $$ = new expr(bool_expr_e);
        $$->sym = newtemp();
        emit(if_greatereq, $1, $3, $$, 0, yylineno);
    }
    | expr LESS_EQUAL expr {
        $$ = new expr(bool_expr_e);
        $$->sym = newtemp();
        emit(if_lesseq, $1, $3, $$, 0, yylineno);
    }
    | expr EQUAL_EQUAL expr {
        $$ = new expr(bool_expr_e);
        $$->sym = newtemp();
        emit(if_eq, $1, $3, $$, 0, yylineno);
    }
    | expr BANG_EQUAL expr {
        $$ = new expr(bool_expr_e);
        $$->sym = newtemp();
        emit(if_noteq, $1, $3, $$, 0, yylineno);
    }
    | expr AND expr
    | expr OR expr
    | term {
        $$ = $1;
    }
    ;

term: '(' expr ')' {$$ = $2;}
    | '-' expr %prec MINUS_UNARY{
        $$ = new expr(arith_expr_e);
        emit(uminus, $2, NULL, $$, 0, yylineno);
    }
    | NOT expr {}
    | PLUS_PLUS lvalue { // TODO: No clue how to differentiate between ++x and x++
        check_lvalue($2->sym->name);
        $$ = new expr(arith_expr_e);
        expr* temp = new expr((double) 1);
        emit(add, $2, temp, $2, 0, yylineno);
        $$ = $2;
    }
    | lvalue PLUS_PLUS {
        check_lvalue($1->sym->name);
        $$ = new expr(arith_expr_e);
        expr* temp = new expr((double) 1);
        emit(add, $1, temp, $1, 0, yylineno);
        $$ = $1;
    }
    | MINUS_MINUS lvalue {
        check_lvalue($2->sym->name);
        $$ = new expr(arith_expr_e);
        expr* temp = new expr((double) 1);
        emit(sub, $2, temp, $2, 0, yylineno);
        $$ = $2;
    }
    | lvalue MINUS_MINUS {
        check_lvalue($1->sym->name);
        $$ = new expr(arith_expr_e);
        expr* temp = new expr((double) 1);
        emit(sub, $1, temp, $1, 0, yylineno);
        $$ = $1;
    }
    | primary {
        $$ = $1;
    }
    ;

assignexpr: lvalue '=' expr 
    {
        check_lvalue($1->sym->name);
        if ($1->type == table_item_e) {
            // lvalue[index] = expr
            emit(table_set_elem, $1, $1->index, $3, 0, yylineno);
            $$ = emit_iftableitem($1); // Will always emit
            $$->type = assign_expr_e;
        } else {
            // that is: lvalue = expr
            emit(assign, $3, NULL, $1, 0, yylineno);
            $$ = new expr(assign_expr_e);
            $$->sym = newtemp();
            emit(assign, $1, NULL, $$, 0, yylineno);
        }
    }; 

primary: lvalue { $$ = emit_iftableitem($1); }
    | call {}
    | objectdef {}
    | '('funcdef')' {}
    | const {
        $$ == $1;
    }
    ;

lvalue: IDENTIFIER {
        // $$ = $1;
        Variable* sym = add_id($1);
        $$ = new expr(var_e);
        $$ -> sym = sym;
    }
    | LOCAL IDENTIFIER {
        Variable* sym = add_local_id($2);
        $$ = new expr(var_e);
        $$ -> sym = sym;
    }
    | COLON_COLON IDENTIFIER {
        Symbol* sym = lookup_global_id($2);
        $$ = new expr(var_e);
        $$ -> sym = sym;
    }
    | member {}
    ;

member: lvalue '.' IDENTIFIER {$$ = member_item($1, $3);}
    | lvalue '[' expr ']' {
        $1 = emit_iftableitem($1);
        $$ = new expr(table_item_e);
        $$->sym = $1 ->sym;
        $$->index = $3;
    }
    | call '.' IDENTIFIER {}
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
funcname: IDENTIFIER { $$ = add_func($1); }
    | { $$ = add_func("_f"); };

/* IMPORTANT NOTE: It would seem that blocks of code count as new tokens . here, the token block is $6 */
funcdef: FUNCTION
    funcname {
        $2 -> index_address = nextquadlabel();
        expr* temp = new expr(program_func_e, $2);
        emit(func_start, temp, NULL, NULL, 0, yylineno);
    }
    formal_arguments
    funcblockstart block {
        $2 ->num_of_locals = $6;
    }
    funcblockend {
        $$ = $2;
        expr* temp = new expr(program_func_e, $2);
        emit(func_end, temp, NULL, NULL, 0, yylineno);
    };

const: INTCONST {
        $$ = new expr( (double) $1);
    }
    | REALCONST {
        $$ = new expr((double) $1);
    }
    | MY_STRING {
        $$ = new expr((std::string) $1);
    }
    | NIL {
        $$ = new expr();
    }
    | TRUE {
        $$ = new expr((bool) $1);
    }
    | FALSE {
        $$ = new expr((bool) $1);
    };

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

    sym_table.PrintTable();
    sym_table.freeTable();  

    printf("\n\nprinting quads\n\n");

    // TODO: write all quads in a file
    print_quads();

    return 0;
}
