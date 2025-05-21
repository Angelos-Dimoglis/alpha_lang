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
        case jump: return "jump";
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

string get_tabs(string str) {
    string tabs = "";
    for (int i = 0; i <= 2 - (str.size() / 8); i++) {
        tabs += "\t";
    }
    return tabs;
}

string doubleToString(double value) {
    string result = to_string(value);
    
    // Remove trailing zeros after decimal point
    size_t dot_pos = result.find('.');
    if (dot_pos != string::npos) {
        // Trim trailing zeros
        result.erase(result.find_last_not_of('0') + 1);
        // If the decimal point is now the last character, remove it too
        if (result.back() == '.') {
            result.pop_back();
        }
    }
    return result;
}

void print_expr_content(expr* exp) {
    if (!exp) {
        return;
    }
    string str;
    switch (exp->type) {
        case const_num_e:
            str = doubleToString(exp->num_const);
            break;
        case const_bool_e:
            str = (exp->bool_const ? "TRUE" : "FALSE");
            break;
        case const_string_e:
            str = exp->str_const;
            break;
        case const_nil_e:
            str = "NIL";
            break;
        default:
            if (exp->sym)
                str = exp->sym->name;
            else
                str = "";
    }
    string tabs = get_tabs(str);
    cout << str << tabs;
}

void print_quad (struct quad *q, int index) {

    cout << index << ": \t\t";

    string tabs = get_tabs(opcode_to_string(q->op));

    cout << opcode_to_string(q->op) << tabs;
    
    if (q->result) {
        print_expr_content(q->result);
    }

    if (q->arg1) {
        print_expr_content(q->arg1);
    }

    if (q->arg2) {
        print_expr_content(q->arg2);
    }

}

void print_quads () {
    const string highlight = "\033[48;5;240m"; // Gray background
    const string reset = "\033[0m"; // Reset formatting

    cout << "quad#\t\topcode\t\t\tresult\t\t\targ1\t\t\targ2\t\t\tlabel\n" <<
    "-------------------------------------------------------------------------" <<
    "--------------------------------------------" + highlight;
    // cout << highlight << "hello";
    for (int i = 0; i < curr_quad; i++) {
        if (i % 2 == 0)
            cout << highlight;
        cout << endl;
        print_quad(&(quads[i]), i);
        if (i % 2 == 0)
            cout << reset;
    }
    cout << reset;


    cout << "\n-------------------------------------------------------------------------" <<
    "--------------------------------------------\n";
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

expr* newexpr_constbool(unsigned int b) {
    expr* e = new expr(const_bool_e);
    e->bool_const = !!b;
    return e;
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
                e->truelist->push_front(e-> ? 0 : 1);
                break;
            default: assert(0);
        }
    }

}
