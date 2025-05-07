#include "parser_functions.h"
#include <cassert>

using namespace std;

extern SymTable sym_table;
extern int yylineno;
extern unsigned int scope;
extern list<Variable*> args;



void add_local_id(const string name) {
    if (sym_table.Lookup(name, scope, THIS_SCOPE) != nullptr)
        return;

    try {
        sym_table.Insert(name, (scope == 0) ? GLOBAL : _LOCAL, yylineno, scope, list<Variable*>());
    } catch(const std::runtime_error &e) {
        cout << "ERROR: " << e.what() << endl;
    }
}

void lookup_global_id(const string name) {
    if (sym_table.Lookup(name, 0, THIS_SCOPE) == nullptr)
        cout << "ERROR: Unable to locate global symbol: \"" <<
            name << "\"" << endl;
}


void add_id(const string name) {
    Symbol* temp = sym_table.Lookup(name, scope, ALL_SCOPES);

    if (temp == nullptr) {
        try {
            sym_table.Insert(name, (scope == 0) ? GLOBAL : _LOCAL, yylineno, scope, list<Variable*>());
        } catch(std::runtime_error &e) {
            cout << e.what() << endl;
            assert(0);
        }

        return;
    }

    if (!(temp->type == _LOCAL || temp->type == FORMAL))
        return;

    for (int i = scope-1; i >= temp->scope; i--) {
        for (node* p = sym_table.scopeNode(i); p != nullptr; p = p->nextScope) {
            if (p->sym.type == USERFUNC && p->sym.isActive) {
                cout << "ERROR: Symbol \"" << name <<
                    "\" isn't accesible in function \"" << p->sym.name <<
                    "\"." <<endl;
                return;
            }
        }
    }
}

void add_func(string name) {
    static int anon_func_counter = 1;
    Symbol* temp = sym_table.Lookup(name, scope, THIS_SCOPE);

    if (name == "_f")
        name += to_string(anon_func_counter++);

    if (temp == nullptr) {
        try {
            sym_table.Insert(name, USERFUNC, yylineno, scope, args);
        } catch(const std::runtime_error &e) {
            cout << "ERROR: " << e.what() << endl;
        }
        args.clear();
    } else {
        cout << "ERROR: Symbol name \"" << name <<
            "\" already exists in current scope (defined in line: " <<
            temp->line << ")" << endl;
    }
}


void add_formal_argument(const string name){
    if (sym_table.Lookup(name, scope, THIS_SCOPE) != nullptr)
        cout << "ERROR: Formal argument \"" << name <<
            "\" already exists in current scope." << endl;

    try {
        sym_table.Insert(name, FORMAL, yylineno, scope, list<Variable*>());
    } catch(const std::runtime_error &e) {
        cout << "ERROR: " << e.what() << endl;
        return;
    }

    args.push_back((Variable*)sym_table.Lookup(name, scope, THIS_SCOPE));
}

void check_lvalue(const string name) {
    Symbol* temp = sym_table.Lookup(name, scope, ALL_SCOPES);

    if (temp != nullptr && (temp->type == USERFUNC || temp->type == LIBFUNC))
        cout << "ERROR: Cannot use function \"" << name << 
            "\" as an lvalue." << endl;
}
