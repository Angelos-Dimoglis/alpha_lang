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
    assert(quadNo < curr_quad);
    assert(!quads[quadNo].label);
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

void emit (iopcode op, expr* result) {
    if (curr_quad == total) 
        expand();

    quad* p = quads + curr_quad++;
    p->op = op;
    p->arg1 = NULL;
    p->arg2 = NULL;
    p->result = result;
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
        emit(param, reversed_elist);
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

Symbol *newtemptemp() {
    Symbol *temp = newtemp();
    tmp_var_counter--;
    return temp;
}

expr *emit_ifboolexpr(expr *e) {
    if (e->type != bool_expr_e || e->truelist == nullptr) {
        return e;
    }

    expr *temp = new expr(bool_expr_e, newtemp());

    unsigned label_true = curr_quad, label_false = curr_quad + 2;

    assert(e->truelist && e->falselist);

    emit(assign, new expr(true), temp);
    emit(jump, curr_quad + 2);
    emit(assign, new expr(false), temp);

    patchlist(*(e->truelist), label_true);
    patchlist(*(e->falselist), label_false);

    return temp;
}

expr *evaluate_to_bool (expr *e) {

    if (e->type == const_num_e)
        return new expr((bool) e->num_const);

    if (e->type == const_string_e)
        return new expr(e->str_const.empty());

    if (e->type == const_nil_e)
        return new expr(false);

    return e;
}

void emit_ifnotrelop(expr *e) {
    if (e->truelist == nullptr || e->falselist == nullptr) {
        e->truelist = new list<unsigned>{curr_quad};
        e->falselist = new list<unsigned>{curr_quad + 1};

        emit(if_eq, evaluate_to_bool(e), new expr(true), (unsigned) 0);
        emit(jump, (unsigned) 0);
    }
}

void print_lists(expr* e) {
    cout << "\n\n\ntruelist:";
    for (auto &i : *e->truelist) {
        printf(" %d", i);
    }
    cout << "\n\n\nfalselist:";
    for (auto &i : *e->falselist) {
        printf(" %d", i);
    }
    cout << "\n\n\n";
}