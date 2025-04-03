#include "parser_functions.h"
#include <cassert>
using namespace std;

extern SymTable sym_table;
extern int yylineno;
extern unsigned int scope;

void add_local_id(const string name){
    if (sym_table.Lookup(name, scope, false)->name.empty()){
        try{
            sym_table.Insert(name, (scope == 0) ? GLOBAL : _LOCAL, yylineno, scope, list<Variable*>());
        }catch(const std::runtime_error &e){cout << "debugging: " << e.what() << endl;}
    }
    sym_table.PrintTable();
}

void lookup_global_id(const string name){
    if (sym_table.Lookup(name, 0, false)->name.empty()){
        cout << "Unable to locate global symbol: \"" << name << "\"" << endl;
    }
}


void add_id(const string name){
    if (sym_table.Lookup(name, scope, true)->name.empty()) {
        try{
            sym_table.Insert(name, (scope == 0) ? GLOBAL : _LOCAL, yylineno, scope, list<Variable*>());
        }catch(std::runtime_error &e){
            cout << e.what() << endl;
            assert(0);
        }
    }
    sym_table.PrintTable();
}

void add_func(const string name, list<Variable*> args) {
    if (sym_table.Lookup(name, scope, false)->name.empty()) {
        try {
            sym_table.Insert(name, USERFUNC, yylineno, scope, args);
        }catch(const std::runtime_error &e){cout << "debugging: " << e.what() << endl;}
    }
    else {
        cout << "Name \"" << name << "\" already exists in current scope." << endl;
    }
    sym_table.PrintTable();
}