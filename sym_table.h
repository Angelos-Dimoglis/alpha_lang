#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <list>
#include <string>
#include <iostream>
#include <memory>
#include <map>

enum SymbolType {
    GLOBAL, LOCAL, FORMAL,
    USERFUNC, LIBFUNC
};

class Symbol {
    public:
        std::string name;
        enum SymbolType type;
        unsigned int line;
        unsigned int scope;
        bool isActive;     
};

class Variable : public Symbol {};

class Function : public Symbol {
    public:
        std::list<Variable> arguments;
};

struct node {
    Symbol sym;
    node* nextCollision;
    node* nextScope;

    node(Symbol s) : sym(s), nextCollision(nullptr), nextScope(nullptr) {}
};

class SymTable {
    private:
        static const int tableSize = 1987;
        node* table[tableSize] = {nullptr};
        std::map<int, node*> scopeHeads;
        int hashFunction(const std::string& key);
        node* scopeNode(unsigned int scope);
        void Initialize();
    public:
        SymTable();

        void Insert(const std::string& name, enum SymbolType type, unsigned int line,
                    unsigned int scope, std::list<Variable> arguments);

        Symbol* Lookup(const std::string& name);

        void Hide(unsigned int scope);
        
        void PrintTable();

        void freeTable();
};

#endif