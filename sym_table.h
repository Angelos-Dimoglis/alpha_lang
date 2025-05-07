#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <list>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <unordered_map>
#include <stdexcept>

#define LIB_MAX 12

#ifndef ALL_SCOPES

#define ALL_SCOPES true
#define THIS_SCOPE false

#endif

using namespace std;

enum SymbolType {
    global, local, formal,
    userfunc, libfunc,
    hidden
};

class Symbol {
    public:
        string name;
        enum SymbolType type;
        unsigned int line;
        unsigned int scope;
        bool isActive;
};

class Variable : public Symbol {};

class Function : public Symbol {
    public:
        list<Variable*> arguments;
};

struct node {
    Symbol sym;
    node* nextCollision;
    node* nextScope;

    node(Symbol s) : sym(s), nextCollision(nullptr), nextScope(nullptr) {}
};

class SymTable {
    private:
        unordered_map<string, node*> table;
        map<int, node*> scopeHeads;
        const std::string library_functions[12] = {"print", "input", "objectmemberkeys", "objecttotalmembers",
            "objectcopy", "totalarguments", "argument", "typeof", 
            "strtonum", "sqrt", "cos", "sin"};
        bool libfunc_check(const string& name);
    public:
        SymTable();

        void Insert(const string& name, enum SymbolType type, unsigned int line,
                    unsigned int scope, list<Variable*> arguments);

        Symbol* Lookup(const string& name, int scope, bool mode);

        void Hide(unsigned int scope);
        
        void PrintTable();

        void freeTable();

        node* collisionNode(const string& key);

        node* scopeNode(unsigned int scope);
};

#endif