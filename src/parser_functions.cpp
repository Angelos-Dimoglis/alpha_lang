#include "../lib/parser_functions.h"
#include <cassert>

extern SymTable sym_table;
extern int yylineno;
extern unsigned int scope;
extern list<Variable*> args;

Variable* add_local_id(const string name) {
    Symbol* sym = sym_table.Lookup(name, scope, THIS_SCOPE);

    if (sym != nullptr)
        return (Variable *) sym;

    try {
        SymbolType st;
        int line;
        if (name[0] == '_') {
            st = hidden;
            line = 0;
        }
        else {
            st = (scope == 0) ? global : local;
            line = yylineno;
        }
        sym_table.Insert(name, st, line, scope, list<Variable*>());
    } catch(const runtime_error &e) {
        cout << "ERROR: " << e.what() << endl;
        return nullptr;
    }
    return (Variable *) sym_table.Lookup(name, scope, THIS_SCOPE);
}

Symbol* lookup_global_id(const string name) {
    Symbol* sym = sym_table.Lookup(name, 0, THIS_SCOPE);

    if (sym == nullptr) {
        cout << "ERROR: Unable to locate global symbol: \"" <<
            name << "\"" << endl;
    }
    return sym;
}


Variable* add_id(const string name) {
    Symbol* temp = sym_table.Lookup(name, scope, ALL_SCOPES);

    if (temp == nullptr) {
        try {
            sym_table.Insert(name, (scope == 0) ? global : local, yylineno, scope, list<Variable*>());
        } catch(runtime_error &e) {
            cout << e.what() << endl;
            // assert(0);
            return nullptr;
        }

        return (Variable*) sym_table.Lookup(name, scope, THIS_SCOPE);
    }

    if (!(temp->type == local || temp->type == formal))
        return (Variable*) temp;

    for (int i = scope-1; i >= temp->scope; i--) {
        for (node* p = sym_table.scopeNode(i); p != nullptr; p = p->nextScope) {
            if (p->sym->type == userfunc && p->sym->isActive) {
                cout << "ERROR: Symbol \"" << name <<
                    "\" isn't accesible in function \"" << p->sym->name <<
                    "\"." <<endl;
            }
        }
    }
    return (Variable*) temp;
}

Function* add_func(string name) {
    static int anon_func_counter = 0;
    Symbol* temp = sym_table.Lookup(name, scope, THIS_SCOPE);

    if (name == "_f")
        name += to_string(anon_func_counter++);

    if (temp == nullptr) {
        try {
            sym_table.Insert(name, userfunc, yylineno, scope, args);
        } catch(const runtime_error &e) {
            cout << "ERROR: " << e.what() << endl;
        }
        args.clear();
        return (Function*) sym_table.Lookup(name, scope, THIS_SCOPE);
    } else {
        cout << "ERROR: Symbol name \"" << name <<
            "\" already exists in current scope (defined in line: " <<
            temp->line << ")" << endl;
    }
    return (Function*) temp;
}


Variable* add_formal_argument(const string name) {
    Symbol* sym = sym_table.Lookup(name, scope, THIS_SCOPE);
    if (sym != nullptr) {
        cout << "ERROR: formal argument \"" << name <<
            "\" already exists in current scope." << endl;
        return (Variable*) sym;
    }

    try {
        sym_table.Insert(name, formal, yylineno, scope, list<Variable*>());
    } catch(const runtime_error &e) {
        cout << "ERROR: " << e.what() << endl;
        return nullptr;
    }

    Variable* temp = (Variable*)sym_table.Lookup(name, scope, THIS_SCOPE);
    args.push_back(temp);
    return temp;
}

Variable* check_lvalue(const string name) {
    Symbol* temp = sym_table.Lookup(name, scope, ALL_SCOPES);

    if (temp != nullptr && (temp->type == userfunc || temp->type == libfunc))
        cout << "ERROR: Cannot use function \"" << name << 
            "\" as an lvalue." << endl;
    return (Variable*) temp;
}
