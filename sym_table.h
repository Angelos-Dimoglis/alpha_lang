#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <list>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <unordered_map>
#include <stdexcept>

<<<<<<< HEAD
#ifndef ALL_SCOPES

#define ALL_SCOPES true
#define THIS_SCOPE false

#endif
=======
using namespace std;
>>>>>>> 614beb4 (added new sym table)

enum SymbolType {
    GLOBAL, _LOCAL, FORMAL,
    USERFUNC, LIBFUNC
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
<<<<<<< HEAD
        static const int tableSize = 1987;
        node* table[tableSize] = {nullptr};
        std::map<int, node*> scopeHeads;
        int hashFunction(const std::string& key);
        bool libfunc_check(const std::string& name);
        const std::string library_functions[12] = {"print", "input", "objectmemberkeys", "objecttotalmembers",
            "objectcopy", "totalarguments", "argument", "typeof", 
            "strtonum", "sqrt", "cos", "sin"};
        void Initialize();
=======
        unordered_map<string, node*> table;
        map<int, node*> scopeHeads;
        node* collisionNode(const string& key);
        node* scopeNode(unsigned int scope);
>>>>>>> 614beb4 (added new sym table)
    public:
        SymTable();

        void Insert(const string& name, enum SymbolType type, unsigned int line,
                    unsigned int scope, list<Variable*> arguments);

        Symbol* Lookup(const string& name, int scope, bool mode);

        void Hide(unsigned int scope);
        
        void PrintTable();

        void freeTable();

        node* scopeNode(unsigned int scope);
};

#endif