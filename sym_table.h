#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <list>
#include <string>

enum SymbolType {
    GLOBAL, LOCAL, FORMAL,
    USERFUNC, LIBFUNC
};

class Symbol {
    public:
        std::string name;
        unsigned int scope;
        bool isActive;     
        enum SymbolType type;
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
        int hashFunction(const std::string& key);

    public:
        void Insert(const std::string& name, unsigned int scope,
                    enum SymbolType type, std::list<Variable> arguments);

        Symbol Lookup(const std::string& name);

        void Hide(const std::string& name);

        node* scopeNode(unsigned int scope);
        
        void PrintTable();
};

#endif