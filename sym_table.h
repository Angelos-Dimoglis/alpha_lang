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
        Symbol* scopeNext;
};

class Variable : Symbol {};

class Function : Symbol {
    public:
        std::list<Variable> arguments;
};

class SymTable {
    private:
        static const int tableSize = 1987;
        std::list<Symbol> table[tableSize];
        int hashFunction(const std::string& key);

    public:
        void Insert(const std::string& name, unsigned int scope, bool isActive,
                    enum SymbolType type, std::list<Variable> arguments);

        Symbol Lookup(const std::string& name);

        void Hide(const std::string& name);

        void PrintTable();
};

#endif