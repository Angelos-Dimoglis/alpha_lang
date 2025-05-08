#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <string>

#include "sym_table.h"

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
    const_num_e, const_bool_e, const_string_e
};

struct expr {
    expr_t type;
    Symbol *sym;
    expr *index;
    double num_const;
    string str_const;
    unsigned char bool_const;
    expr *next;

    expr(expr_t type, string str_const = "") : type(type), sym(nullptr), index(nullptr),
        num_const(0), str_const(str_const),
        bool_const(false), next(nullptr) {};
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

void print_quad (struct quad *q);

void expand (void);

void emit (iopcode op, expr* arg1, expr* arg2, expr* result, unsigned label, unsigned line);

Symbol *newtemp();

expr* emit_iftableitem(expr* e);

expr *member_item(expr *lvalue, string name);
