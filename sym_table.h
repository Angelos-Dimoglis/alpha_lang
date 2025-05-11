#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <list>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <unordered_map>
#include <stdexcept>
#include <cassert>
#include <stack>
#include <initializer_list>

#define LIB_MAX 12

#ifndef ALL_SCOPES

#define ALL_SCOPES true
#define THIS_SCOPE false
#define IS_VARIABLE(x) (x == global || x == local || x == formal || x == hidden)

#endif

using namespace std;

void enterscopespace (void);
void exitscopespace (void);
int getoffset();

enum SymbolType {
    global, local, formal, hidden, userfunc, libfunc
};

enum scopespace_t {
    program_var,
    function_local,
    formal_arg
};

class Symbol {
    public:
        string name;
        enum SymbolType type;
        unsigned int line;
        unsigned int scope;
        bool isActive;
};

class Variable : public Symbol {
    public:
        scopespace_t space;
        unsigned int offset;
};

class Function : public Symbol {
    public:
        list<Variable*> arguments;
        unsigned int num_of_locals = 0;
        unsigned int index_address = 0;
};

struct node {
    Symbol* sym;
    node* nextCollision;
    node* nextScope;

    node(Symbol* s) : sym(s), nextCollision(nullptr), nextScope(nullptr) {}
};

class SymTable {
    private:
        unordered_map<string, node*> table;
        map<int, node*> scopeHeads;
        const std::string library_functions[12] = {
            "print",
            "input",
            "objectmemberkeys",
            "objecttotalmembers",
            "objectcopy",
            "totalarguments",
            "argument",
            "typeof",
            "strtonum",
            "sqrt",
            "cos",
            "sin"
        };

        bool libfunc_check(const string& name);
    public:
        SymTable();

        void Insert(
            const string& name,
            enum SymbolType type,
            unsigned int line,
            unsigned int scope,
            list<Variable*> arguments
        );

        Symbol* Lookup(const string& name, int scope, bool mode);

        void Hide(unsigned int scope);
        
        void PrintTable();

        void freeTable();

        node* collisionNode(const string& key);

        node* scopeNode(unsigned int scope);
};

#endif
