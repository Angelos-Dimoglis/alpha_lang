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

void print_quad (struct quad *q) {
    printf("opcode: %d, arg1: %p, arg2: %p, res: %p\n", 
           q->op, q->arg1, q->arg2, q->result);
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
    item->index = new expr(const_string_e, name);
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
    Symbol *sym = sym_table.Lookup(name, scope, THIS_SCOPE);
    if (sym == nullptr) {

        try {
            sym_table.Insert(name, hidden, yylineno, scope, std::list<Variable *>());
        } catch () {
        
        }

        return sym_table.Lookup(name, scope, THIS_SCOPE);
    }

    return sym;
}
