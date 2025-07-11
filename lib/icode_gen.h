#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <string>

#include "sym_table.h"
#include "parser_functions.h"

extern int yylineno;
extern unsigned int scope;
extern SymTable sym_table;

enum iopcode {
    assign = 0, add, sub, mul, _div, mod, uminus,
    if_eq, if_noteq, if_lesseq, if_greatereq, if_less, if_greater, jump,
    call, param, ret, get_ret_val, func_start, func_end,
    table_create, table_get_elem, table_set_elem
};

enum expr_t {
    var_e = 0, table_item_e,
    program_func_e, library_func_e,
    arith_expr_e, bool_expr_e, assign_expr_e, new_table_e,
    const_num_e, const_bool_e, const_string_e, const_nil_e
};

// TODO: add a union for the data
struct expr {
    expr_t type;
    Symbol *sym;
    expr *index;
    double num_const;
    string str_const;
    bool bool_const;
    expr *next;
    list<unsigned int>* falselist;
    list<unsigned int>* truelist;

    expr () : 
        type(const_nil_e),
        sym(nullptr),
        index(nullptr),
        num_const(0),
        str_const(""),
        bool_const(false),
        next(nullptr) 
    {
    };

    expr(expr_t type, Symbol* sym = nullptr) : type(type), sym(sym), index(nullptr),
        num_const(0), str_const(""),
        bool_const(false), next(nullptr) {
        //assert(type == )
    };

    expr(Symbol* sym) : sym(sym), index(nullptr),
        num_const(0), str_const(""),
        bool_const(false), next(nullptr) {
        
            if (sym->type == userfunc) {
                type = program_func_e;
            }else if (sym->type == libfunc) {
                type = library_func_e;
            }else {
                type = var_e;
            }
    };

    expr(double num_const) :
        type(const_num_e),
        sym(nullptr),
        index(nullptr),
        num_const(num_const),
        str_const(""),
        bool_const(false),
        next(nullptr) 
    {
    };

    expr(string str_const) :
        type(const_string_e),
        sym(nullptr),
        index(nullptr),
        num_const(0),
        str_const(str_const),
        bool_const(false),
        next(nullptr)
    {
    };

    expr(bool bool_const) :
        type(const_bool_e),
        sym(nullptr),
        index(nullptr),
        num_const(0),
        str_const(""),
        bool_const(bool_const),
        next(nullptr)
    {
    };

};

struct stmt {
    list<unsigned> returnlist;
    list<unsigned> breaklist;
    list<unsigned> contlist;

    stmt() {
        returnlist = list<unsigned>();
        breaklist = list<unsigned>();
        contlist = list<unsigned>();
    }
};

struct forp {
    unsigned enter;
    unsigned test;
};

struct quad {
    iopcode op;
    expr *arg1;
    expr *arg2;
    expr *result;
    unsigned label;
    unsigned line;
    unsigned taddress; // NOTE: maybe here
};

#define EXPAND_SIZE 1024
#define CURR_SIZE (total*sizeof(quad))
#define NEW_SIZE (EXPAND_SIZE*sizeof(quad)+CURR_SIZE)

list<unsigned> *merge (list<unsigned> *L1, list<unsigned> *L2);

void check_arith (expr* e, string context);

expr *get_last (expr* exp);

expr* make_call (expr* lv, expr* reversed_elist);

unsigned int nextquadlabel ();

void patchlabel (unsigned quadNo, unsigned label);

void patchlist (list<unsigned> quadBBC, unsigned label);

void expand (void);

void emit (iopcode op, expr* arg1, expr* arg2, expr* result, unsigned label);

void emit (iopcode op, expr* arg1, expr* arg2, expr* result);

void emit (iopcode op, expr* arg1, expr* arg2, unsigned label);

void emit (iopcode op, expr* arg1, expr* result);

void emit (iopcode op, expr* arg1);

void emit (iopcode op, unsigned label);

Symbol *newtemp ();

Symbol *newtemptemp();

void resettemp();

expr *emit_iftableitem(expr* e);

expr *member_item (expr *lvalue, string name);

expr *newexpr_constbool (unsigned int b);

expr *emit_ifboolexpr (expr* e);

void emit_ifnotrelop (expr *e);

void print_lists(expr* e);
