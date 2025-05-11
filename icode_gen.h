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
    _and, _or, _not,
    if_eq, if_noteq, if_lesseq, if_greatereq, if_less, if_greater,
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
    unsigned char bool_const;
    expr *next;

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

struct quad {
    iopcode op;
    expr *arg1;
    expr *arg2;
    expr *result;
    unsigned label;
    unsigned line;
};

#define EXPAND_SIZE 1024
#define CURR_SIZE (total*sizeof(quad))
#define NEW_SIZE (EXPAND_SIZE*sizeof(quad)+CURR_SIZE)

unsigned int nextquadlabel();

void patchlabel (unsigned quadNo, unsigned label);

void print_quad (struct quad *q);

void print_quads ();

void expand (void);

void emit (iopcode op, expr* arg1, expr* arg2, expr* result, unsigned label, unsigned line);

Symbol *newtemp();

expr* emit_iftableitem(expr* e);

expr *member_item(expr *lvalue, string name);
