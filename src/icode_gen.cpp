#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <string>

#include "../lib/sym_table.h"
#include "../lib/icode_gen.h"

extern quad *quads;
extern unsigned total;
extern unsigned int curr_quad;

extern void yyerror(const char *msg, int line_number);

unsigned int nextquadlabel() {
    return curr_quad;
}

void patchlabel(unsigned quadNo, unsigned label) {
    assert(quadNo < curr_quad && !quads[quadNo].label);
    quads[quadNo].label = label;
}

list<unsigned> *merge (list<unsigned> *L1, list<unsigned> *L2) {
    if (L1 == NULL && L2 == NULL)
        return new list<unsigned>();

    if (L1 == NULL) 
        return L2;

    if (L2 == NULL)
        return L1;

    L1->splice(L1->end(), *L2);
    return L1;
}

void patchlist(list<unsigned> quadBBC, unsigned label) {
    if (quadBBC.empty()) return;

    for (unsigned quadNo : quadBBC)
        patchlabel(quadNo, label);
}

void expand (void) {
    assert(total == curr_quad);
    quad* p = (quad*) malloc(NEW_SIZE);

    if (quads) {
        memcpy(p, quads, CURR_SIZE);
        free(quads);
    }

    quads = p;
    total += EXPAND_SIZE;
}

void emit (iopcode op, expr* arg1, expr* arg2, expr* result, unsigned label) {
    if (curr_quad == total) 
        expand();

    quad* p = quads + curr_quad++;
    p->op = op;
    p->arg1 = arg1;
    p->arg2 = arg2;
    p->result = result;
    p->label = label;
    p->line = yylineno;
}

void emit (iopcode op, expr* arg1, expr* arg2, unsigned label) {
    if (curr_quad == total) 
        expand();

    quad* p = quads + curr_quad++;
    p->op = op;
    p->arg1 = arg1;
    p->arg2 = arg2;
    p->result = NULL;
    p->label = label;
    p->line = yylineno;
}

void emit (iopcode op, expr* arg1, expr* arg2, expr* result) {
    if (curr_quad == total) 
        expand();

    quad* p = quads + curr_quad++;
    p->op = op;
    p->arg1 = arg1;
    p->arg2 = arg2;
    p->result = result;
    p->label = 0;
    p->line = yylineno;
}

void emit (iopcode op, expr* arg1, expr* result) {
    if (curr_quad == total) 
        expand();

    quad* p = quads + curr_quad++;
    p->op = op;
    p->arg1 = arg1;
    p->arg2 = NULL;
    p->result = result;
    p->label = 0;
    p->line = yylineno;
}

void emit (iopcode op, unsigned label) {
    if (curr_quad == total) 
        expand();

    quad* p = quads + curr_quad++;
    p->op = op;
    p->arg1 = NULL;
    p->arg2 = NULL;
    p->result = NULL;
    p->label = label;
    p->line = yylineno;
}

void emit (iopcode op, expr* arg1) {
    if (curr_quad == total) 
        expand();

    quad* p = quads + curr_quad++;
    p->op = op;
    p->arg1 = arg1;
    p->arg2 = NULL;
    p->result = NULL;
    p->label = 0;
    p->line = yylineno;
}

int tmp_var_counter = 0;

expr* emit_iftableitem(expr* e) {
    if (e->type != table_item_e)  {
        return e;
    }

    expr* result = new expr(var_e);
    result->sym = newtemp();
    emit(table_get_elem, e, e->index, result, 0);
    return result;
}

expr *member_item(expr *lvalue, string name) {
    lvalue = emit_iftableitem(lvalue);
    expr *item = new expr(table_item_e);
    item->sym = lvalue->sym;
    item->index = new expr(name);
    return item;
}

expr *get_last(expr* exp) {
    if (exp == nullptr) {
        return nullptr;
    }
    expr* e;
    for (e = exp; e->next != nullptr; e = e->next){}
    return e;
}

expr* make_call (expr* lv, expr* reversed_elist) {
    expr* func = emit_iftableitem(lv);
    while (reversed_elist) {
        emit(param, reversed_elist, (expr*) NULL, (expr*) NULL);
        reversed_elist = reversed_elist->next;
    }
    emit(call, func);
    expr* result = new expr(var_e, newtemp());
    emit(get_ret_val, NULL, NULL, result);
    return result;
}

void check_arith (expr* e, string context) {
    if (e->type == const_bool_e ||
        e->type == const_string_e ||
        e->type == const_nil_e ||
        e->type == new_table_e ||
        e->type == program_func_e ||
        e->type == library_func_e ||
        e->type == bool_expr_e ) {
            string msg = "Illegal expr used in \'" + context;
            yyerror(msg.c_str(), yylineno);
        }
}

string newtempname() {
    return  "_t" + to_string(tmp_var_counter++);
}

void resettemp() {
    tmp_var_counter = 0; 
}

Symbol *newtemp() {
    string name = newtempname();
    return add_local_id(name);
}

expr *emit_ifboolexpr(expr *e) {
    if (e->type != bool_expr_e)
        return e;

    expr *temp = new expr(const_bool_e, newtemp());
    emit(assign, new expr(true), temp);
    emit(jump, curr_quad + 2);
    emit(assign, new expr(false), temp);
    return temp;
}

expr *emit_ifnotrelop(expr *e) {
    if (
        e->type == const_bool_e ||
        e->type == const_num_e ||
        e->type == const_string_e ||
        e->type == var_e
    ) {
        emit(if_eq, e, new expr(true), (unsigned) 0);
        emit(jump, (unsigned) 0);

        switch (e->type) {
            case const_bool_e:
                e->truelist->push_front(e->bool_const == true ? 1 : 0);
                break;
            case const_num_e:
                e->truelist->push_front(e->num_const == 0.0 ? 0 : 1);
                break;
            case const_string_e:
                e->truelist->push_front(e->str_const.empty() ? 0 : 1);
                break;
            case var_e:
                e->truelist->push_front(1);
                e->truelist->push_front(true ? 0 : 1);
                break;
            default: assert(0);
        }
    }

    return nullptr;
}
