%{
    #include <stdio.h>
    #include <string.h>
    #include <string>
    #include <stack>
    #include <assert.h>
    #include <list>
    #include <utility>

    using namespace std;

    #include "../lib/parser_functions.h"
    #include "../lib/icode_gen.h"

    #define EMPTY_LABEL ((unsigned) 0)

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
    unsigned total = 1;
    unsigned int curr_quad = 1;

    void yyerror(const char *msg) {
        if (strcmp(msg, "syntax error, unexpected end of file"))
            fprintf(stderr, "%s at line %d before token: %s\n",
                msg, yylineno, yytext);
    }

    stack<int> loopcounter {std::deque<int> {0}};

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

    int break_continue_valid(string break_or_continue) {
        if (loopcounter.empty() || loopcounter.top() == 0) {
            yyerror(("Use of \'" + break_or_continue + "\' while not in a loop").c_str());
            return 0;
        }
        return 1;
    }

    int return_valid() {
        if (loopcounter.empty()) {
            yyerror("Use of \'return\' while not in a function");
            return 0;
        }
        return 1;
    }

%}

%code requires {

    using namespace std;

    #include "sym_table.h"
    #include "icode_gen.h"
    #include "parser_functions.h"

    struct call {
        expr* elist;
        unsigned char method;
        char* name;
    };
}

%define parse.error detailed

%union {
    int intValue;
    double doubleValue;
    bool boolValue;
    char *stringValue;
    void *nilValue;
    struct expr *exprValue;
    struct stmt *stmtValue;
    struct forp forValue;
    struct Function *funcSymValue;
    struct quad * quadValue;
    struct call callValue;
    list<pair<expr*, expr*>>* indexedList;
    pair<expr*, expr*>* indexedPair;
    enum iopcode opcodeValues;
}

%start program

//  ### list of terminals ###

%token <doubleValue> REALCONST INTCONST
%token <stringValue> MY_STRING
%token <nilValue> NIL
%token <boolValue> TRUE FALSE
%token <stringValue> IDENTIFIER

%token KEYWORD
%token IF ELSE WHILE FOR FUNCTION RETURN BREAK CONTINUE AND NOT OR LOCAL

%token OPERATOR
%token EQUAL_EQUAL BANG_EQUAL PLUS_PLUS MINUS_MINUS GREATER_EQUAL LESS_EQUAL MINUS_UNARY

%token PUNCTUATION
%token COLON_COLON DOT_DOT

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


%type <callValue> callsuffix normcall methodcall
%type <indexedList> indexed indexed_alt
%type <indexedPair> indexedelem
%type <exprValue> expr lvalue term primary member assignexpr const elist elist_alt call objectdef
%type <stmtValue> stmt block stmt_series
%type <forValue> forprefix
%type <intValue> ifprefix elseprefix whilestart whilecond N M
%type <funcSymValue> funcname funcdef // NOTE: THIS MIGHT HAVE TO BECOME symValue LATER ON lec 10, sl 7

%%

program: stmt_series;

stmt_series: stmt_series stmt  {

        if ($stmt != nullptr && !$stmt->breaklist.empty())
            $1->breaklist.push_back($stmt->breaklist.front());

        if ($stmt != nullptr && !$stmt->contlist.empty())
            $1->contlist.push_back($stmt->contlist.front());

        $$ = $1;
    }
    | {$stmt_series = new stmt();} /* empty */
    ;

stmt: expr ';' {
        $$ = nullptr;
        resettemp();
    }
    | ifstmt {
        $$ = nullptr;
        resettemp();
    }
    | whilestmt {
        $$ = nullptr;
        resettemp();
    }
    | forstmt {
        $$ = nullptr;
        resettemp();
    }
    | returnstmt {
        $$ = nullptr;
        $stmt = new stmt();
        $stmt->breaklist.push_back(curr_quad); 
        emit(jump, unsigned(0));
        resettemp();
    }
    | BREAK ';' {
        $$ = nullptr;
        if (break_continue_valid("break")) {
            $stmt = new stmt();
            $stmt->breaklist.push_back(curr_quad);
            emit(jump, unsigned(0));
        }
        resettemp();
    }
    | CONTINUE ';' {
        $$ = nullptr;
        if (break_continue_valid("continue")) {
            $stmt = new stmt();
            $stmt->contlist.push_back(curr_quad);
            emit(jump, unsigned(0));
        }
        resettemp();
    }
    | block {$stmt = $block; resettemp();}
    | funcdef {
        $$ = nullptr;
        resettemp();
    }
    | ';' {
        $$ = nullptr;
        resettemp();
    }
    ;

expr: assignexpr {}
    | expr '+' expr {
        $$ = new expr(arith_expr_e, newtemp());
        emit(add, $1, $3, $$);
    }
    | expr '-' expr {
        $$ = new expr(arith_expr_e, newtemp());
        emit(sub, $1, $3, $$);
    }
    | expr '*' expr {
        $$ = new expr(arith_expr_e, newtemp());
        emit(mul, $1, $3, $$);
    }
    | expr '/' expr {
        $$ = new expr(arith_expr_e, newtemp());
        emit(_div, $1, $3, $$);
    }
    | expr '%' expr {
        $$ = new expr(arith_expr_e, newtemp());
        emit(mod, $1, $3, $$);
    }
    | expr '>' expr {
        $$ = new expr(bool_expr_e, newtemp());
        $$->truelist = new list<unsigned>();
        $$->falselist = new list<unsigned>();
        emit(if_greater, $1 , $3, EMPTY_LABEL);
        emit(jump, EMPTY_LABEL);
    }
    | expr '<' expr {
        $$ = new expr(bool_expr_e, newtemp());
        $$->truelist = new list<unsigned>();
        $$->falselist = new list<unsigned>();
        emit(if_less, $1 , $3, EMPTY_LABEL);
        emit(jump, EMPTY_LABEL);
    }
    | expr GREATER_EQUAL expr {
        $$ = new expr(bool_expr_e, newtemp());
        $$->truelist = new list<unsigned>();
        $$->falselist = new list<unsigned>();
        emit(if_greatereq, $1 , $3, EMPTY_LABEL);
        emit(jump, EMPTY_LABEL);
    }
    | expr LESS_EQUAL expr {
        $$ = new expr(bool_expr_e, newtemp());
        $$->truelist = new list<unsigned>();
        $$->falselist = new list<unsigned>();
        emit(if_lesseq, $1 , $3, EMPTY_LABEL);
        emit(jump, EMPTY_LABEL);
    }
    | expr EQUAL_EQUAL expr {
        $$->truelist = new list<unsigned>();
        $$->falselist = new list<unsigned>();
        $$ = new expr(bool_expr_e, newtemp());
        emit(if_eq, $1 , $3, EMPTY_LABEL);
        emit(jump, EMPTY_LABEL);
    }
    | expr BANG_EQUAL expr {
        $$ = new expr(bool_expr_e, newtemp());
        $$->truelist = new list<unsigned>();
        $$->falselist = new list<unsigned>();
        emit(if_noteq, $1 , $3, EMPTY_LABEL);
        emit(jump, EMPTY_LABEL);
    }
    | expr AND M expr {
        emit_ifnotrelop($1);
        patchlist(*($1->truelist), $M);
        $$->truelist = $4->truelist;
        $$->falselist = merge($1->falselist, $4->falselist);
    }
    | expr OR M expr {
        patchlist(*($1->falselist), $M);
        $$->truelist = merge($1->truelist, $4->truelist);
        $$->falselist = $4->falselist;
    }
    | term {
        $$ = $1;
    }
    ;

M: /* empry rule */ {
    $M = nextquadlabel();
 }
 ;

N: {$N = nextquadlabel(); emit(jump, unsigned(0));}

term: '(' expr ')' {
        emit_ifboolexpr($expr);
        $term = $expr;
    }
    | '-' expr %prec MINUS_UNARY{
        check_arith($expr, "unary minus");
        $term = new expr(arith_expr_e, newtemp());
        emit(uminus, $expr, NULL, $term, 0);
    }
    | NOT expr {
        $term->truelist = $expr->falselist;
        $term->falselist = $expr->truelist;

        $term = new expr(bool_expr_e, newtemp());
        emit(_not , $expr, NULL, $term, 0);
    }
    | PLUS_PLUS lvalue {
        check_arith($lvalue, "++lvalue");
        if ($lvalue->type == table_item_e) {
            $term = emit_iftableitem($lvalue);
            emit(add, $term, new expr((double) 1), $term);
            emit(table_set_elem, $lvalue->index, $lvalue, $term);
        } else {
            emit(add, $lvalue, new expr((double) 1), $lvalue);
            $term = new expr(arith_expr_e, newtemp());
            emit(assign, $lvalue, NULL, $term);
        }
    }
    | lvalue PLUS_PLUS {
        check_arith($lvalue, "lvalue++");
        $term = new expr(var_e, newtemp());
        if ($lvalue->type == table_item_e) {
            expr* val = emit_iftableitem($lvalue);
            emit(assign, val, NULL, $term, 0);
            emit(add, val, new expr((double) 1), val, 0);
            emit(table_set_elem, $lvalue->index, val, $lvalue, 0);
        } else {
            emit(assign, $lvalue, NULL, $term, 0);
            emit(add, $lvalue, new expr((double) 1), $lvalue, 0);
        }
    }
    | MINUS_MINUS lvalue {
        check_arith($lvalue, "--lvalue");
        if ($lvalue->type == table_item_e) {
            $term = emit_iftableitem($lvalue);
            emit(sub, $term, new expr((double) 1), $term, 0);
            emit(table_set_elem, $lvalue->index, $term, $lvalue, 0);
        } else {
            emit(sub, $lvalue, new expr((double) 1), $lvalue, 0);
            $term = new expr(arith_expr_e, newtemp());
            emit(assign, $lvalue, NULL, $term, 0);
        }
    }
    | lvalue MINUS_MINUS {
        check_arith($lvalue, "lvalue--");
        $term = new expr(var_e, newtemp());
        if ($lvalue->type == table_item_e) {
            expr* val = emit_iftableitem($lvalue);
            emit(assign, val, NULL, $term, 0);
            emit(sub, val, new expr((double) 1), val, 0);
            emit(table_set_elem, $lvalue->index, val, $lvalue, 0);
        } else {
            emit(assign, $lvalue, NULL, $term, 0);
            emit(sub, $lvalue, new expr((double) 1), $lvalue, 0);
        }
    }
    | primary {
        $term = $primary;
    }
    ;

assignexpr: lvalue '=' expr {
        check_lvalue($1->sym->name);
        if ($1->type == table_item_e) {
            // lvalue[index] = expr
            emit(table_set_elem, $1->index, $3, $1, 0);
            $$ = emit_iftableitem($1); // Will always emit
            $$->type = assign_expr_e;
        } else {
            // that is: lvalue = expr
            emit(assign, $3, NULL, $1, 0);
            $$ = new expr(assign_expr_e);
            $$->sym = newtemp();
            emit(assign, $1, NULL, $$, 0);
        }
    }; 

primary: lvalue { $$ = emit_iftableitem($1); }
    | call {}
    | objectdef {}
    | '('funcdef')' {
        $primary = new expr(program_func_e, $funcdef);
    }
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
    | call '[' expr ']' {
        emit_ifboolexpr($expr);
    }
    ;

call: call '(' elist ')' {
        $$ = make_call($1, $elist);
    }
    | lvalue callsuffix {
        $lvalue = emit_iftableitem($lvalue);    // in case it was a table item too
        if ($callsuffix.method){
            get_last($callsuffix.elist)->next = $lvalue;    // insert first (reversed, so from last)
            $lvalue = emit_iftableitem(member_item($lvalue, $callsuffix.name));
        }
        $call = make_call($lvalue, $callsuffix.elist);
    }
    | '('funcdef')' '(' elist ')' {
        expr* func = new expr(program_func_e, $funcdef);
        $call = make_call(func, $elist);
    }
    ;

callsuffix: normcall {
        $callsuffix = $normcall;
    }
    | methodcall {
        $callsuffix = $methodcall;
    }
    ;

normcall: '(' elist ')' {
        $normcall.elist = $elist;
        $normcall.method = 0;
        $normcall.name = NULL;
    }
    ;

// equivalent to lvalue.id(lvalue, elist)
methodcall: DOT_DOT IDENTIFIER '(' elist ')' {
        $methodcall.elist = $elist;
        $methodcall.method = 1;
        $methodcall.name = strdup($IDENTIFIER);
    }
    ;

elist: expr elist_alt {
        emit_ifboolexpr($expr);
        $expr->next = $elist_alt;
        $elist = $expr;
    }
    | {$elist = NULL;}
    ;

elist_alt: ',' expr elist_alt {
        emit_ifboolexpr($expr);
        $expr->next = $3;
        $$ = $expr;
    }
    | {$elist_alt = NULL;}
    ;

objectdef: '[' elist ']' {
        expr* t = new expr(new_table_e, newtemp());
        emit(table_create, NULL, NULL, t, 0);
        for (int i = 0; $elist != NULL; $elist = $elist->next)
            emit(table_set_elem, new expr((double) i++), $elist, t);
        $objectdef = t;
    }
    | '[' indexed ']' {
        expr* t = new expr(new_table_e, newtemp());
        emit(table_create, NULL, NULL, t, 0);
        for (const auto& pair : *($indexed)) {
            emit(table_set_elem, pair.first, pair.second, t);
        }
        $objectdef = t;
    }
    ;

indexed: indexedelem indexed_alt {
        $indexed_alt->push_front(*($indexedelem));
        $indexed = $indexed_alt;
    }
    ;

indexed_alt: ',' indexedelem indexed_alt {
        ($3)->push_front(*($indexedelem));
        $$ = $3;
    }
    | {$indexed_alt = new list<pair<expr*, expr*>>();}
    ;

indexedelem: '{' expr ':' expr '}' {
        emit_ifboolexpr($2);
        emit_ifboolexpr($4);
        $indexedelem = new pair<expr*, expr*>($2, $4);
    }
    ;

block: '{' {scope++;} stmt_series '}' {sym_table.Hide(scope--); $block = $stmt_series;};

funcblockstart: { scope--; enterscopespace(); push_loopcounter(); };
funcblockend: { exitscopespace(); pop_loopcounter(); };
formal_arguments: '(' {scope++; enterscopespace();} idlist ')';
funcname: IDENTIFIER { $$ = add_func($1); }
    | { $$ = add_func("_f"); };

/* IMPORTANT NOTE: It would seem that blocks of code count as new tokens . here, the token block is $6 */
funcdef: FUNCTION N
    funcname {
        $3 -> index_address = nextquadlabel();
        expr* temp = new expr(program_func_e, $3);
        emit(func_start, NULL, NULL, temp, 0);
    }
    formal_arguments
    funcblockstart block {
        $3 ->num_of_locals = getoffset();
    }
    funcblockend {
        $$ = $3;
        expr* temp = new expr(program_func_e, $3);
        patchlist($block->breaklist, nextquadlabel());
        emit(func_end, NULL, NULL, temp, 0);
        patchlabel($N, nextquadlabel());
    };

const: INTCONST {
        $$ = new expr( (double) $1);
    }
    | REALCONST {
        $$ = new expr((double) $1);
    }
    | MY_STRING {
        $$ = new expr((string) $1);
    }
    | NIL {
        $$ = new expr();
    }
    | TRUE {
        $$ = new expr(true);
    }
    | FALSE {
        $$ = new expr(false);
    };

idlist: IDENTIFIER {add_formal_argument($1);} idlist_alt
    | /* empty */
    ;

idlist_alt: ',' IDENTIFIER {add_formal_argument($2);} idlist_alt
    | /* empty */
    ;

ifstmt: ifprefix stmt {patchlabel($1, nextquadlabel());} %prec IF
    | ifprefix stmt elseprefix stmt {patchlabel($1, $3 + 1); patchlabel($3, nextquadlabel());}
    ;

ifprefix: IF '(' expr ')' {
        emit(if_eq, $expr, new expr(true), nextquadlabel() + 2);

        $ifprefix = nextquadlabel();
        emit(jump, unsigned(0));
}

elseprefix: ELSE {
    $elseprefix = nextquadlabel();
    emit(jump, unsigned(0));
}

loopstart: { increase_loopcounter(); }
loopend: { decrease_loopcounter(); }

whilestmt: whilestart whilecond loopstart stmt {
    emit(jump, unsigned($1));
    patchlabel($2, nextquadlabel());
    if ($stmt != nullptr)
        patchlist($stmt->breaklist, nextquadlabel());

    if ($stmt != nullptr)
        patchlist($stmt->contlist, $1);
} loopend;

whilestart: WHILE {
    $whilestart = nextquadlabel();
}

whilecond: '(' expr ')' {
    emit(if_eq, $2, new expr(true), nextquadlabel() + 2);
    $whilecond = nextquadlabel();
    emit(jump, unsigned(0));
}

forstmt: forprefix N elist ')' N loopstart stmt N loopend {
    patchlabel($forprefix.enter, $5 + 1);
    patchlabel($2, nextquadlabel());
    patchlabel($5, $forprefix.test);
    patchlabel($8, $2 + 1);

    patchlist($stmt->breaklist, nextquadlabel());
    patchlist($stmt->contlist, $2 + 1);
}

forprefix: FOR '(' elist ';' M expr ';' {
    $forprefix.test = $M;
    $forprefix.enter = nextquadlabel();
    emit(if_eq, $expr, new expr(true), unsigned(0));
}

returnstmt: RETURN ';' {return_valid(); emit(ret, NULL, NULL, NULL, 0);}
    | RETURN  expr ';' {return_valid(); emit(ret, NULL, NULL, $expr, 0);}
    ;

%%


int main (int argc, char **argv) {

    // uncomment for debug mode
    //yydebug = 1;

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
