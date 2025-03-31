#include "sym_table.h"
#include <iostream>

int SymTable::hashFunction(const std::string& key) {
    int hash = 0;
    for (char ch : key) {
        hash += ch;
    }
    return hash % tableSize;
}

void SymTable::Insert(const std::string& name, unsigned int scope, bool isActive,
                        enum SymbolType type, std::list<Variable> arguments) {
    
    if (Lookup(name).name == NULL) {
        int index = hashFunction(name);
        Symbol* newSymbol = new Symbol();
        newSymbol->name = name;
        newSymbol
    }
}

Symbol SymTable::Lookup(const std::string& name) {
    int index = hashFunction(name);
    for (const auto& node : table[index]) {
        if (node.name == name) {
            return node;
        }
    }
    Symbol* temp = new Symbol();
    temp->name = NULL;
    return *temp;
}