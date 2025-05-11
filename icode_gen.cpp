#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <string>

#include "sym_table.h"
#include "icode_gen.h"

extern quad *quads;
extern unsigned total;
extern unsigned int curr_quad;

unsigned int nextquadlabel() {
    return curr_quad + 1;
}

void patchlabel (unsigned quadNo, unsigned label) {
    assert(quadNo < curr_quad);
    quads[quadNo].label = label;
}

string opcode_to_string(iopcode opcode) {

    switch (opcode) {
        case assign: return "assign";
        case add: return "add";
        case sub: return "sub";
        case mul: return "mul";
        case _div: return "div";
        case mod: return "mod";
        case uminus: return "uminus";
        case _and: return "and";
        case _or: return "or";
        case _not: return "not";
        case if_eq: return "if_eq";
        case if_noteq: return "if_noteq";
        case if_lesseq: return "if_lesseq";
        case if_greatereq: return "if_greatereq";
        case if_less: return "if_less";
        case if_greater: return "if_greater";
        case call: return "call";
        case param: return "param";
        case ret: return "ret";
        case get_ret_val: return "get_ret_val";
        case func_start: return "func_start";
        case func_end: return "func_end";
        case table_create: return "table_create";
        case table_get_elem: return "table_get_elem";
        case table_set_elem: return "table_set_elem";
        default:
            assert(0);
    }

    return "";
}

void print_quad (struct quad *q) {

    cout << q->line << ": ";

    cout << "opcode: " << opcode_to_string(q->op) << " ";

    if (q->arg1)
        cout << "arg1: " << q->arg1->sym->name << " ";

    if (q->arg2)
        cout << "arg2: " << q->arg2->sym->name << " ";

    if (q->result)
        cout << "res: " << q->result->sym->name;

    cout << endl;
}

void print_quads () {
    for (int i = 0; i < curr_quad; i++)
        print_quad(&(quads[i]));
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

void emit (iopcode op, expr* arg1, expr* arg2, expr* result, unsigned label, unsigned line) {
    if (curr_quad == total) 
        expand();

    quad* p = quads + curr_quad++;
    p->op = op;
    p->arg1 = arg1;
    p->arg2 = arg2;
    p->result = result;
    p->label = label;
    p->line = line;
}

int tmp_var_counter = 0;

expr* emit_iftableitem(expr* e) {
    if (e->type != table_item_e) 
        return e;

    expr* result = new expr(var_e);
    result->sym = newtemp();
    emit(table_get_elem, e, e->index, result, 0, yylineno);
    return result;
}

expr *member_item(expr *lvalue, string name) {
    lvalue = emit_iftableitem(lvalue);
    expr *item = new expr(table_item_e);
    item->sym = lvalue->sym;
    item->index = new expr(name);
    return item;
}

int tempcounter = 0;

string newtempname() {
    return  "_t" + to_string(tmp_var_counter++);
}

void resettemp() {
    tempcounter = 0; 
}

Symbol *newtemp() {
    string name = newtempname();
    return add_id(name);
}

