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

    unsigned int scope = 0;
    SymTable sym_table;
    list<Variable*> args;

    quad *quads = (quad*) 0;
    unsigned total = 1;
    unsigned int curr_quad = 1;

    extern bool has_errors;

    void yyerror(const char *msg) {
        if (strcmp(msg, "syntax error, unexpected end of file")) {
            fprintf(stderr, "%s at line %d before token: %s\n",
                msg, yylineno, yytext);
                has_errors = true;
        }
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
%right NOT
%nonassoc EQUAL_EQUAL BANG_EQUAL
%nonassoc '>' GREATER_EQUAL '<' LESS_EQUAL
%left '+' '-'
%left '*' '/' '%'
%right PLUS_PLUS MINUS_MINUS MINUS_UNARY
%left '.' DOT_DOT
%left '[' ']'
%left '(' ')'
%nonassoc IF
%nonassoc ELSE


%type <callValue> callsuffix normcall methodcall
%type <indexedList> indexed indexed_alt
%type <indexedPair> indexedelem
%type <exprValue> expr lvalue term primary member assignexpr const elist elist_alt call objectdef
%type <stmtValue> stmt block stmt_series ifstmt whilestmt forstmt returnstmt
%type <forValue> forprefix
%type <intValue> ifprefix elseprefix whilestart whilecond N M
%type <funcSymValue> funcname funcdef // NOTE: THIS MIGHT HAVE TO BECOME symValue LATER ON lec 10, sl 7

%%

program: stmt_series;

stmt_series: stmt stmt_series  {

        if ($1 != nullptr && $2 != nullptr) {
            $1->returnlist.splice($1->returnlist.end(), $2->returnlist);
        }

        if ($stmt != nullptr && $2 != nullptr) {
            $1->breaklist.splice($1->breaklist.end(), $2->breaklist);
        }

        if ($stmt != nullptr && $2 != nullptr) {
            $1->contlist.splice($1->contlist.end(), $2->contlist);
        }

        $$ = $1;
    }
    | {$stmt_series = new stmt();} /* empty */
    ;

stmt: expr ';' {
        emit_ifboolexpr($expr);
        $$ = new stmt();
        resettemp();
    }
    | ifstmt {
        $$ = $ifstmt;
        resettemp();
    }
    | whilestmt {
        $$ = new stmt();
        $$->returnlist = $whilestmt->returnlist;
        resettemp();
    }
    | forstmt {
        $$ = new stmt();
        $$->returnlist = $forstmt->returnlist;
        resettemp();
    }
    | returnstmt {
        $stmt = $returnstmt;
        $stmt->returnlist.push_back(curr_quad); 
        emit(jump, unsigned(0));
        resettemp();
    }
    | BREAK ';' {
        $$ = new stmt();
        if (break_continue_valid("break")) {
            $stmt->breaklist.push_back(curr_quad);
            emit(jump, unsigned(0));
        }
        resettemp();
    }
    | CONTINUE ';' {
        $$ = new stmt();
        if (break_continue_valid("continue")) {
            $stmt = new stmt();
            $stmt->contlist.push_back(curr_quad);
            emit(jump, unsigned(0));
        }
        resettemp();
    }
    | block {
        $stmt = $block;
        resettemp();
    }
    | funcdef {
        $$ = new stmt();
        resettemp();
    }
    | ';' {
        $$ = new stmt();
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
        $$ = new expr(bool_expr_e, newtemptemp());
        $$->truelist = new list<unsigned>{curr_quad};
        $$->falselist = new list<unsigned>{curr_quad + 1};
        emit(if_greater, $1 , $3, EMPTY_LABEL);
        emit(jump, EMPTY_LABEL);
    }
    | expr '<' expr {
        $$ = new expr(bool_expr_e, newtemptemp());
        $$->truelist = new list<unsigned>{curr_quad};
        $$->falselist = new list<unsigned>{curr_quad + 1};
        emit(if_less, $1 , $3, EMPTY_LABEL);
        emit(jump, EMPTY_LABEL);
    }
    | expr GREATER_EQUAL expr {
        $$ = new expr(bool_expr_e, newtemptemp());
        $$->truelist = new list<unsigned>{curr_quad};
        $$->falselist = new list<unsigned>{curr_quad + 1};
        emit(if_greatereq, $1 , $3, EMPTY_LABEL);
        emit(jump, EMPTY_LABEL);
    }
    | expr LESS_EQUAL expr {
        $$ = new expr(bool_expr_e, newtemptemp());
        $$->truelist = new list<unsigned>{curr_quad};
        $$->falselist = new list<unsigned>{curr_quad + 1};
        emit(if_lesseq, $1 , $3, EMPTY_LABEL);
        emit(jump, EMPTY_LABEL);
    }
    | expr EQUAL_EQUAL {$1 = emit_ifboolexpr($1);} expr {
        $$ = new expr(bool_expr_e, newtemptemp());
        $$->truelist = new list<unsigned>{curr_quad};
        $$->falselist = new list<unsigned>{curr_quad + 1};
        emit(if_eq, $1 , $4, EMPTY_LABEL);
        emit(jump, EMPTY_LABEL);
    }
    | expr BANG_EQUAL {$1 = emit_ifboolexpr($1);} expr {
        $$ = new expr(bool_expr_e, newtemptemp());
        $$->truelist = new list<unsigned>{curr_quad};
        $$->falselist = new list<unsigned>{curr_quad + 1};
        emit(if_noteq, $1 , $4, EMPTY_LABEL);
        emit(jump, EMPTY_LABEL);
    }
    | expr AND {emit_ifnotrelop($1);} M expr {

        $$ = new expr(bool_expr_e, newtemptemp());

        emit_ifnotrelop($5);

        patchlist(*($1->truelist), $M);
        $$->truelist = $5->truelist;
        $$->falselist = merge($1->falselist, $5->falselist);
    }
    | expr OR {emit_ifnotrelop($1);} M expr {

        $$ = new expr(bool_expr_e, newtemptemp());
        
        emit_ifnotrelop($5);

        patchlist(*($1->falselist), $M);
        $$->truelist = merge($1->truelist, $5->truelist);
        $$->falselist = $5->falselist;
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
        $$ = emit_ifboolexpr($expr);
    }
    | '-' expr %prec MINUS_UNARY{
        check_arith($expr, "unary minus");
        $term = new expr(arith_expr_e, newtemp());
        emit(uminus, $expr, NULL, $term, 0);
    }
    | NOT expr {
        $term = new expr(bool_expr_e, newtemptemp());
        emit_ifnotrelop($expr);
        $term->truelist = $expr->falselist;
        $term->falselist = $expr->truelist;
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
        expr *temp = emit_ifboolexpr($expr);
        if ($1->type == table_item_e) {
            emit(table_set_elem, $1->index, temp, $1, 0);
            $$ = emit_iftableitem($1); // Will always emit
            $$->type = assign_expr_e;
        } else {
            // that is: lvalue = expr
            emit(assign, temp, NULL, $1, 0);
            $$ = new expr(assign_expr_e);
            $$->sym = newtemp();
            emit(assign, $1, NULL, $$, 0);
        }
    }; 

primary: lvalue { $$ = emit_iftableitem($1); }
    | call {
        $primary = $call;
    }
    | objectdef {
        // may not be right
        $primary = $objectdef;
    }
    | '('funcdef')' {
        $primary = new expr(program_func_e, $funcdef);
    }
    | const {
        $$ == $1;
    }
    ;

lvalue: IDENTIFIER {
        Variable* sym = add_id($1);
        $$ = new expr(sym);
    }
    | LOCAL IDENTIFIER {
        Variable* sym = add_local_id($2);
        $$ = new expr(sym);
    }
    | COLON_COLON IDENTIFIER {
        Symbol* sym = lookup_global_id($2);
        $$ = new expr(sym);
    }
    | member {}
    ;

member: lvalue '.' IDENTIFIER {$$ = member_item($1, $3);}
    | lvalue '[' expr ']' {
        $expr = emit_ifboolexpr($expr);
        $1 = emit_iftableitem($1);
        $$ = new expr(table_item_e);
        $$->sym = $1->sym;
        $$->index = $expr;
    }
    | call '.' IDENTIFIER {}
    | call '[' expr ']' {
        $expr = emit_ifboolexpr($expr);
        $1 = emit_iftableitem($1);
        $$ = new expr(table_item_e);
        $$->sym = $1->sym;
        $$->index = $expr;
    }
    ;

call: call '(' elist ')' {
        $$ = make_call($1, $elist);
    }
    | lvalue callsuffix {
        $lvalue = emit_iftableitem($lvalue);    // in case it was a table item too
        if ($callsuffix.method){
            if ($callsuffix.elist)
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
        $expr = emit_ifboolexpr($expr);
        $expr->next = $elist_alt;
        $elist = $expr;
    }
    | {$elist = NULL;}
    ;

elist_alt: ',' expr elist_alt {
        $expr = emit_ifboolexpr($expr);
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
        $2 = emit_ifboolexpr($2);
        $4 = emit_ifboolexpr($4);
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
        patchlist($block->returnlist, nextquadlabel());
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

ifstmt: ifprefix stmt {
        patchlabel($1, nextquadlabel());
        $$ = $stmt;
    }
    %prec IF
    | ifprefix stmt elseprefix stmt {
        patchlabel($1, $3 + 1);
        patchlabel($3, nextquadlabel());
        
        assert($2 && $4);

        $2->returnlist.splice($2->returnlist.end(), $4->returnlist);
        $2->breaklist.splice($2->breaklist.end(), $4->breaklist);
        $2->contlist.splice($2->contlist.end(), $4->contlist);
        
        $$ = $2;
    }
    ;

ifprefix: IF '(' expr ')' {

        $expr = emit_ifboolexpr($expr);

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
        assert($stmt);
        if ($stmt != nullptr) {
            patchlist($stmt->breaklist, nextquadlabel());
        }

        if ($stmt != nullptr)
            patchlist($stmt->contlist, $1);

    } loopend {
        $$ = $stmt;
    };

whilestart: WHILE {
    $whilestart = nextquadlabel();
}

whilecond: '(' expr ')' {
    $expr = emit_ifboolexpr($expr);
    emit(if_eq, $expr, new expr(true), nextquadlabel() + 2);
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

    $forstmt = $stmt;
}

forprefix: FOR '(' elist ';' M expr ';' {
    $expr = emit_ifboolexpr($expr);
    $forprefix.test = $M;
    $forprefix.enter = nextquadlabel();
    emit(if_eq, $expr, new expr(true), unsigned(0));
}

returnstmt: RETURN ';' {
        return_valid();
        emit(ret, NULL, NULL, NULL, 0);
        $returnstmt = new stmt();
    }
    | RETURN  expr ';' {
        $expr = emit_ifboolexpr($expr);
        return_valid();
        emit(ret, NULL, NULL, $expr, 0);
        $returnstmt = new stmt();
    }
    ;
